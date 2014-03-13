#include "Robot.h"

Robot :: Robot ():
	WheelConfig (),
	BeltConfig ()
{

	Log = Logger :: GetInstance ();
	Log -> SetPrintLevel ( Logger :: LOG_DEBUG );

	Log -> Log ( Logger :: LOG_EVENT, "** Robot Starting **\n" );

	Mode = RobotStartMode;

	DsLcd = DriverStationLCD :: GetInstance ();

	InitVision ();
	InitSensors ();
	InitMotors ();
	InitControls ();
	InitBehaviors ();
	InitDecorations ();

	Winch -> SetZero ();
	Arms -> SetZeros ();

	TestCount = 0;
	AutoCount = 0;
	TeleCount = 0;
	DisbCount = 0;

	Log -> Log ( Logger :: LOG_EVENT, "** Robot Started **\n" );

};

void Robot :: InitVision ()
{

	Log -> Log ( Logger :: LOG_DEBUG, "Initializing Vision\n" );

	AutonomousTask = new Task ( "2605_Autonomous", (FUNCPTR) & Robot :: AutonomousTaskStub );
	TeleopTask = new Task ( "2605_Teleop", (FUNCPTR) & Robot :: TeleopTaskStub );
	VisionTask = new Task ( "2605_Vision", (FUNCPTR) & Robot :: VisionTaskStub, VISION_PRIORITY );

	TargetingCamera = & AxisCamera :: GetInstance ( "10.26.5.11" );

};

void Robot :: InitSensors ()
{

	Log -> Log ( Logger :: LOG_EVENT, "Initializing Sensors\n" );

	DistanceSensorAnalog = new AnalogChannel ( 1, 6 );
	BallSensor = new IRDistanceSensor ( DistanceSensorAnalog );

	BallPositionSwitch = new DigitalInput ( 4 );

};

void Robot :: InitControls ()
{

	Log -> Log ( Logger :: LOG_EVENT, "Initializing Controls\n" );

	printf ( "INIT CONTROLS\n" );

	StrafeStick = new Joystick ( 1 );
	RotateStick = new Joystick ( 2 );
	ShootStick = new Joystick ( 3 );

	OnShiftDelegate = new ClassDelegate <Robot, void> ( this, & Robot :: OnShift );

	GearStepper = new NumericStepper ( StrafeStick, 5, 4 );
	GearStepper -> SetRange ( 1, 3 );
	GearStepper -> SetChangeListener ( OnShiftDelegate );
	GearStepper -> Set ( 1 );

	OnShift ();

};

void Robot :: InitMotors ()
{

	Log -> Log ( Logger :: LOG_EVENT, "Initializing Motors\n" );

	// PICServo Controller

	PICServoControl = new PICServoController ();

	// CANJaguar Server

	JagServer = new CANJaguarServer ();
	JagServer -> Start ();

	// DRIVE

	//WheelConfig.Mode = CANJaguar :: kVoltage;
	WheelConfig.Mode = CANJaguar :: kSpeed;
	WheelConfig.P = P_GEAR1;
	WheelConfig.I = I_GEAR1;
	WheelConfig.D = D_GEAR1;
	WheelConfig.EncoderLinesPerRev = ENCODER_CODES_PER_REVOLUTION;
	WheelConfig.SpeedRef = CANJaguar :: kSpeedRef_QuadEncoder;
	WheelConfig.NeutralAction = CANJaguar :: kNeutralMode_Brake;
	WheelConfig.FaultTime = 0.51;

	WheelFL = new AsynchCANJaguar ( JagServer, 7, WheelConfig );
	WheelFR = new AsynchCANJaguar ( JagServer, 1, WheelConfig );
	WheelRL = new AsynchCANJaguar ( JagServer, 5, WheelConfig );
	WheelRR = new AsynchCANJaguar ( JagServer, 2, WheelConfig );

	Drive = new MecanumDrive ( WheelFL, WheelFR, WheelRL, WheelRR );

	Drive -> SetMotorScale ( SPEED_SCALE_GEAR1 );
	Drive -> SetPreScale ( 1, 1 );
	Drive -> SetInverted ( false, true, false, true );
	Drive -> SetSineInversion ( false );
	Drive -> Disable ();

	// BELTS

	BeltConfig.Mode = CANJaguar :: kSpeed;
	BeltConfig.P = BELT_P;
	BeltConfig.I = BELT_I;
	BeltConfig.D = BELT_D;
	BeltConfig.EncoderLinesPerRev = BELT_ENCODER_CODES_PER_REVOLUTION;
	BeltConfig.SpeedRef = CANJaguar :: kSpeedRef_QuadEncoder;
	BeltConfig.NeutralAction = CANJaguar :: kNeutralMode_Brake;
	BeltConfig.MaxVoltage = 14;
	BeltConfig.FaultTime = 0.51;

	BeltL = new AsynchCANJaguar ( JagServer, 8, BeltConfig );
	BeltR = new AsynchCANJaguar ( JagServer, 6, BeltConfig );

	Shooter = new ShooterBelts ( BeltL, BeltR );

	Shooter -> SetMotorScale ( BELT_SPEED_SCALE );
	Shooter -> SetInverted ( false, true );

	// ARMS/WINCH

	Wait ( 3.0 );
	Log -> Log ( Logger :: LOG_EVENT, "Initializing PIC-Servos\n" );

	PICServoControl -> AddPICServo ( 1, false, 3, 4 );
	PICServoControl -> AddPICServo ( 3, false, 4, 3 );
	PICServoControl -> AddPICServo ( 4, false, 9, 5 );

	ArmL = PICServoControl -> GetModule ( 1 );

	ArmL -> SetEncoderResolution ( 360 * 4 );
	ArmL -> SetPID ( 0.5, 0.001, 0.05, 0.65 );
	ArmL -> ConfigVelocity ( 1.0 );
	ArmL -> ConfigAcceleration ( 0.1 );

	ArmR = PICServoControl -> GetModule ( 3 );

	ArmR -> SetEncoderResolution ( 360 * 4 );
	ArmR -> SetPID ( 0.5, 0.001, 0.05, 0.65 );
	ArmR -> ConfigVelocity ( 1.0 );
	ArmR -> ConfigAcceleration ( 0.1 );

	Arms = new CollectorArms ( ArmL, ArmR );

	Arms -> SetInverted ( false, false );

	// WINCH

	WinchM = PICServoControl -> GetModule ( 4 );

	WinchM -> SetEncoderResolution ( 1000 );
	WinchM -> SetPID ( 0.27, 0, 0.01, 1.0, 0.4 );
	WinchM -> ConfigAcceleration ( 0.1258789 );
	WinchM -> ConfigVelocity ( 2.0 );

	Winch = new ShooterWinch ( WinchM );

};

void Robot :: InitBehaviors ()
{

	Log -> Log ( Logger :: LOG_EVENT, "Initializing Behaviors\n" );

	Behaviors = new BehaviorController ();

	TELEOP_DRIVE_BEHAVIOR = "TeleopDrive";
	TeleopDrive = new TeleopDriveBehavior ( Drive, StrafeStick, RotateStick, GearStepper, OnShiftDelegate, Arms, Shooter );

	BALL_PICKUP_BEHAVIOR = "BallPickup";
	BallPickup = new BallPickupBehavior ( Shooter, Arms, Drive, GearStepper, OnShiftDelegate, BallSensor, BallPositionSwitch );

	AUTONOMOUS_START_BEHAVIOR = "AutonomousStart";
	AutonomousStart = new AutonomousStartBehavior ( Arms, Winch );

	Behaviors -> AddBehavior ( TELEOP_DRIVE_BEHAVIOR, TeleopDrive );
	Behaviors -> AddBehavior ( BALL_PICKUP_BEHAVIOR, BallPickup );
	Behaviors -> AddBehavior ( AUTONOMOUS_START_BEHAVIOR, AutonomousStart );

};

void Robot :: InitDecorations ()
{

	//LEDs = new LEDStrip ( 1, 2, 7 );

	//TestAnimation = BuildTestAnimation ( LEDs );

	//LEDs -> Clear ();

};

Robot :: ~Robot ()
{



};

//============================================================//
/*========================[DRIVE STUFF]=======================*/
//============================================================//

void Robot :: OnShift ()
{

	ShiftVGear ( GearStepper -> Get () );

	DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line2, "GEAR %i", GearStepper -> Get () );
	DsLcd -> UpdateLCD ();

};

void Robot :: ShiftVGear ( uint8_t Gear )
{

	switch ( Gear )
	{

		case 1:

			Drive -> SetMotorScale ( SPEED_SCALE_GEAR1 );

			GearRPM = SPEED_SCALE_GEAR1;

			WheelConfig.P = P_GEAR1;
			WheelConfig.I = I_GEAR1;
			WheelConfig.D = D_GEAR1;

			WheelFL -> Configure ( WheelConfig );
			WheelFR -> Configure ( WheelConfig );
			WheelRL -> Configure ( WheelConfig );
			WheelRR -> Configure ( WheelConfig );

			break;

		case 2:

			Drive -> SetMotorScale ( SPEED_SCALE_GEAR2 );

			GearRPM = SPEED_SCALE_GEAR2;

			WheelConfig.P = P_GEAR2;
			WheelConfig.I = I_GEAR2;
			WheelConfig.D = D_GEAR2;

			WheelFL -> Configure ( WheelConfig );
			WheelFR -> Configure ( WheelConfig );
			WheelRL -> Configure ( WheelConfig );
			WheelRR -> Configure ( WheelConfig );

			break;

		case 3:

			Drive -> SetMotorScale ( SPEED_SCALE_GEAR3 );

			GearRPM = SPEED_SCALE_GEAR3;
	
			WheelConfig.P = P_GEAR3;
			WheelConfig.I = I_GEAR3;
			WheelConfig.D = D_GEAR3;

			WheelFL -> Configure ( WheelConfig );
			WheelFR -> Configure ( WheelConfig );
			WheelRL -> Configure ( WheelConfig );
			WheelRR -> Configure ( WheelConfig );

			break;

		default:

			break;

	}

};

//============================================================//
/*==========================[VISION]==========================*/
//============================================================//

void Robot :: VisionTaskRoutine ()
{

	Threshold TargetColorThreshold (105, 137, 230, 255, 133, 183);

	ColorImage * TargetImage = new ColorImage ( IMAQ_IMAGE_HSL );

	TargetingCamera -> WriteResolution ( AxisCameraParams :: kResolution_320x240 );
	TargetingCamera -> GetImage ( TargetImage );

};

//============================================================//
/*========================[AUTONOMOUS]========================*/
//============================================================//

void Robot :: AutonomousInit ()
{

	DisabledEnd ();
	Mode = AutonomousMode;

	Log -> Log ( Logger :: LOG_EVENT, "================\n= Autonomous ! =\n================\n" );

	DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line1, "Autonomous" );
	DsLcd -> UpdateLCD ();

	Behaviors -> StartBehavior ( AUTONOMOUS_START_BEHAVIOR );

	Log -> Log ( Logger :: LOG_DEBUG, "AUTO!\n" );

};

void Robot :: AutonomousPeriodic ()
{

	AutoCount ++;

	Log -> Log ( Logger :: LOG_DEBUG3, "BEHAVIOR_UPDATE!\n" );
	Behaviors -> Update ();

};

void Robot :: AutonomousEnd ()
{

	Winch -> Disable ();

	Behaviors -> StopBehavior ( AUTONOMOUS_START_BEHAVIOR );

	Log -> Log ( Logger :: LOG_EVENT, "================\n= Autonomous X =\n================\n" );

};

void Robot :: AutonomousTaskRoutine ()
{



};

//============================================================//
/*==========================[TELEOP]==========================*/
//============================================================//

void Robot :: TeleopInit ()
{

	DisabledEnd ();
	Mode = TeleopMode;

	Log -> Log ( Logger :: LOG_EVENT, "================\n=   Teleop !    =\n================\n" );

	DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line1, "Teleop" );

	Behaviors -> StartBehavior ( TELEOP_DRIVE_BEHAVIOR );

	TeleopTask -> Start ( reinterpret_cast <uint32_t> ( this ) );

};

void Robot :: TeleopPeriodic ()
{

	TeleCount ++;

	Behaviors -> Update ();

	if ( Behaviors -> GetBehaviorActive ( TELEOP_DRIVE_BEHAVIOR ) )
	{

		if ( TeleopDrive -> DoPickup () )
		{

			Log -> Log ( Logger :: LOG_DEBUG, "Switching to BallPickup\n" );

			Behaviors -> StopBehavior ( TELEOP_DRIVE_BEHAVIOR );
			Behaviors -> StartBehavior ( BALL_PICKUP_BEHAVIOR );

		}

	}
	else if ( Behaviors -> GetBehaviorActive ( BALL_PICKUP_BEHAVIOR ) )
	{

		if ( BallPickup -> IsDone () )
		{

			Log -> Log ( Logger :: LOG_DEBUG, "Switching to TeleopDrive\n" );

			Behaviors -> StopBehavior ( BALL_PICKUP_BEHAVIOR );
			Behaviors -> StartBehavior ( TELEOP_DRIVE_BEHAVIOR );

		}

	}

};

void Robot :: TeleopEnd ()
{

	Log -> Log ( Logger :: LOG_EVENT, "================\n=   Teleop X   =\n================\n" );

	if ( Behaviors -> GetBehaviorActive ( TELEOP_DRIVE_BEHAVIOR ) )
		Behaviors -> StopBehavior ( TELEOP_DRIVE_BEHAVIOR );

	if ( Behaviors -> GetBehaviorActive ( BALL_PICKUP_BEHAVIOR ) )
		Behaviors -> StopBehavior ( BALL_PICKUP_BEHAVIOR );

	DsLcd -> UpdateLCD ();

	TeleopTask -> Stop ();

	Drive -> Disable ();
	Shooter -> Disable ();

};

void Robot :: TeleopTaskRoutine ()
{

	while ( true )
	{

		Wait ( 0.01 );

		

	}

};

//============================================================//
/*===========================[TEST]===========================*/
//============================================================//

void Robot :: TestInit ()
{

	if ( Mode != TestMode )
		DisabledEnd ();

	Mode = TestMode;

	Log -> Log ( Logger :: LOG_EVENT, "================\n=    Test !     =\n================\n" );

	DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line1, "Test" );
	DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line3, "0.2 Meters," );
	DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line4, "Press 3" );
	DsLcd -> UpdateLCD ();

	TestPeriodMode = 0;

};

void Robot :: TestPeriodic ()
{

	TestCount ++;

	switch ( TestPeriodMode )
	{

	case 0:

		if ( StrafeStick -> GetRawButton ( 3 ) )
		{

			BallSensor -> CalibHighPoint ( 0.2 );

			TestPeriodMode = 1;

			DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line3, "0.8 Meters," );
			DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line4, "Press 2." );

		}

		break;

	case 1:

		if ( StrafeStick -> GetRawButton ( 2 ) )
		{

			BallSensor -> CalibLowPoint ( 0.8 );

			TestPeriodMode = 0;

			DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line3, "0.2 Meters," );
			DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line4, "Press 3" );

		}

		break;

	default:
		break;

	}

	DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line6, "X: %f", BallSensor -> Get () );
	DsLcd -> UpdateLCD ();

};

void Robot :: TestEnd ()
{

	Log -> Log ( Logger :: LOG_EVENT, "================\n=    Test X    =\n================\n" );

};

//============================================================//
/*=========================[DISABLED]=========================*/
//============================================================//

void Robot :: DisabledInit ()
{

	switch ( Mode )
	{

	default:
	case DisabledMode:

		break;

	case AutonomousMode:

		AutonomousEnd ();

		break;

	case TeleopMode:

		TeleopEnd ();

		break;

	case TestMode:

		TestEnd ();

		break;

	}

	Mode = DisabledMode;

	Log -> Log ( Logger :: LOG_EVENT, "================\n=  Disabled !  =\n================\n" );

	DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line1, "Disabled\n" );
	DsLcd -> UpdateLCD ();

};

void Robot :: DisabledPeriodic ()
{

	DisbCount ++;

	/*LEDs -> SetPixelColor ( ( DisbCount + 0 ) % 7, 0xFF0000 );
	LEDs -> SetPixelColor ( ( DisbCount + 1 ) % 7, 0x00FF00 );
	LEDs -> SetPixelColor ( ( DisbCount + 2 ) % 7, 0x0000FF );
	LEDs -> SetPixelColor ( ( DisbCount + 3 ) % 7, 0xFFFF00 );
	LEDs -> SetPixelColor ( ( DisbCount + 4 ) % 7, 0xFF00FF );
	LEDs -> SetPixelColor ( ( DisbCount + 5 ) % 7, 0x00FFFF );
	LEDs -> SetPixelColor ( ( DisbCount + 6 ) % 7, 0xFFFFFF );*/
	//LEDs -> PushColors ();

	//Arms -> CalibratePICServoAnalogs ();

};

void Robot :: DisabledEnd ()
{

	Log -> Log ( Logger :: LOG_EVENT, "================\n=  Disabled X  =\n================\n" );

};

//============================================================//
/*========================[TASK STUBS]========================*/
//============================================================//

int Robot :: AutonomousTaskStub ( Robot * This )
{

	This -> AutonomousTaskRoutine ();

	return 0;

};

int Robot :: TeleopTaskStub ( Robot * This )
{

	This -> TeleopTaskRoutine ();

	return 0;

};

int Robot :: VisionTaskStub ( Robot * This )
{

	This -> VisionTaskRoutine ();

	return 0;

};
