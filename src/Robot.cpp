#include "Robot.h"

Robot :: Robot ():
	WheelConfig (),
	BeltConfig ()
{

	Log = Logger :: GetInstance ();
	Log -> SetPrintLevel ( Logger :: LOG_DEBUG3 );

	Log -> Log ( Logger :: LOG_EVENT, "** Robot Starting **\n" );

	Mode = RobotStartMode;

	DsLcd = DriverStationLCD :: GetInstance ();

	InitVision ();
	InitSensors ();
	InitMotors ();
	InitControls ();
	InitBehaviors ();

	TestCount = 0;
	AutoCount = 0;
	TeleCount = 0;

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

	BallPositionSwitch = new DigitalInput ( 1 );

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

	Log -> Log ( Logger :: LOG_EVENT, "Initializing PIC-Servos\n" );

	PICServoControl -> AddPICServo ( 1, false, 3, 4 );
	PICServoControl -> AddPICServo ( 2, false, 4, 3 );
	PICServoControl -> AddPICServo ( 4, false, 9, 5 );

	ArmL = PICServoControl -> GetModule ( 1 );

	ArmL -> SetEncoderResolution ( 360 * 4 );
	ArmL -> SetPID ( 0.35, 0.0, 0.0, 0.4 );
	ArmL -> ConfigVelocity ( 0.04 );
	ArmL -> ConfigAcceleration ( 0.002 );

	ArmR = PICServoControl -> GetModule ( 2 );

	ArmR -> SetEncoderResolution ( 360 * 4 );
	ArmR -> SetPID ( 0.35, 0.0, 0.0, 0.4 );
	ArmR -> ConfigVelocity ( 0.04 );
	ArmR -> ConfigAcceleration ( 0.002 );

	Arms = new CollectorArms ( ArmL, ArmR );

	Arms -> SetInverted ( false, false );

	// WINCH

	WinchM = PICServoControl -> GetModule ( 4 );

	WinchM -> SetEncoderResolution ( 1000 );
	WinchM -> SetPID ( 1.0, 0, 3.0, 1.0, 0.2, 0.23529 );
	WinchM -> ConfigAcceleration ( 0.0015258789 );
	WinchM -> ConfigVelocity ( 1.525878 );

	Winch = new ShooterWinch ( WinchM );

};

void Robot :: InitBehaviors ()
{

	Log -> Log ( Logger :: LOG_EVENT, "Initializing Behaviors\n" );

	Behaviors = new BehaviorController ();

	TELEOP_DRIVE_BEHAVIOR = "TeleopDrive";
	TeleopDrive = new TeleopDriveBehavior ( Drive, StrafeStick, RotateStick, GearStepper, OnShiftDelegate, Arms );

	BALL_PICKUP_BEHAVIOR = "BallPickup";
	BallPickup = new BallPickupBehavior ( Shooter, Arms, Drive, GearStepper, OnShiftDelegate, BallSensor, BallPositionSwitch );

	AUTONOMOUS_START_BEHAVIOR = "AutonomousStart";
	AutonomousStart = new AutonomousStartBehavior ( Arms );

	Behaviors -> AddBehavior ( TELEOP_DRIVE_BEHAVIOR, TeleopDrive );
	Behaviors -> AddBehavior ( BALL_PICKUP_BEHAVIOR, BallPickup );
	Behaviors -> AddBehavior ( AUTONOMOUS_START_BEHAVIOR, AutonomousStart );

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

	TargetFound = false;

	/*Log -> Log ( Logger :: LOG_DEBUG, "Starting auto tasks...\n" );

	VisionTask -> Start ( reinterpret_cast <uint32_t> ( this ) );
	AutonomousTask -> Start ( reinterpret_cast <uint32_t> ( this ) );

	Log -> Log ( Logger :: LOG_DEBUG, "Starting behaviors...\n" );

	Behaviors -> StartBehavior ( AUTONOMOUS_START_BEHAVIOR );

	Log -> Log ( Logger :: LOG_DEBUG, "AUTO!\n" );*/

	Arms -> Enable ();
	Arms -> SetZeros ();

};

void Robot :: AutonomousPeriodic ()
{

	AutoCount ++;

	/*Behaviors -> Update ();*/
	Log -> Log ( Logger :: LOG_DEBUG, "Left: %f\n", ArmL -> GetPosition () );

	ArmL -> SetControlMode ( PICServo :: kPosition );
	ArmL -> Set ( 0.67014 );

};

void Robot :: AutonomousEnd ()
{

	Arms -> Disable ();

	/*Behaviors -> StopBehavior ( AUTONOMOUS_START_BEHAVIOR );

	AutonomousTask -> Stop ();
	VisionTask -> Stop ();*/

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
	
	Drive -> Enable ();
	Shooter -> Enable ();

	Behaviors -> StartBehavior ( TELEOP_DRIVE_BEHAVIOR );

	TeleopTask -> Start ( reinterpret_cast <uint32_t> ( this ) );

};

void Robot :: TeleopPeriodic ()
{

	TeleCount ++;

	Behaviors -> Update ();

	if ( RotateStick -> GetRawButton ( 1 ) )
	{

		Shooter -> SetSpeed ( 1.0 );

	}
	else if ( RotateStick -> GetRawButton ( 3 ) )
		Shooter -> SetSpeed ( 0.17 );
	else if ( RotateStick -> GetRawButton ( 2 ) )
		Shooter -> SetSpeed ( - 0.17 );
	else
		Shooter -> SetSpeed ( 0 );

	Shooter -> PushSpeeds ();

};

void Robot :: TeleopEnd ()
{

	Log -> Log ( Logger :: LOG_EVENT, "================\n=   Teleop X   =\n================\n" );

	Behaviors -> StopBehavior ( TELEOP_DRIVE_BEHAVIOR );

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

	Log -> Log ( Logger :: LOG_EVENT, "================\n=    Test !     =\n================\n" );

	Mode = TestMode;

	DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line1, "Test" );
	DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line2, "Please select a test:" );
	DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line3, "8: Cal. Wench" );
	DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line4, "11: Cal. Ball Sensor" );
	DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line5, "10: Pickup Ball" );
	DsLcd -> UpdateLCD ();

	WinchM -> CalibrateAnalog ();
	ArmL -> CalibrateAnalog ();
	ArmR -> CalibrateAnalog ();

	ArmL -> ResetPosition ();
	ArmR -> ResetPosition ();

	TestPeriodMode = 0;

	while ( TestPeriodMode == 0 && IsEnabled () )
	{

		if ( StrafeStick -> GetRawButton ( 8 ) )
		{

			Log -> Log ( Logger :: LOG_EVENT, "Winch Calibration\n" );

			TestPeriodMode = 1;

			DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line2, "** CALIBRATE WENCH **" );
			DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line3, "Use 11/10 to move wench" );
			DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line4, "to 0, with slack. Then," );
			DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line5, "press 3." );
			DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line6, "BALL: %f m", BallSensor -> Get () );
			DsLcd -> UpdateLCD ();

			Winch -> Enable ();

		}

		if ( StrafeStick -> GetRawButton ( 11 ) )
		{

			Log -> Log ( Logger :: LOG_EVENT, "Ball Sensor Calibration\n" );

			TestPeriodMode = 2;

			DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line2, "** CALIBRATE BALL SENSE **" );
			DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line3, "Place ball 0.2m from," );
			DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line4, "bot, press 3." );
			DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line5, "" );
			DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line6, "BALL: %f m", BallSensor -> Get () );
			DsLcd -> UpdateLCD ();

		}

		if ( StrafeStick -> GetRawButton ( 10 ) )
		{

			Log -> Log ( Logger :: LOG_EVENT, "Ball Pickup\n" );

			TestPeriodMode = 3;
			
			DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line2, "** PICKUP BALL **" );
			DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line3, "" );
			DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line4, "" );
			DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line5, "" );
			DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line6, "BALL: %f m", BallSensor -> Get () );
			DsLcd -> UpdateLCD ();

		}

		m_ds -> WaitForData ();
		Wait ( 0.01 );

	}

};

void Robot :: TestPeriodic ()
{

	TestCount ++;

	switch ( TestPeriodMode )
	{

		case 1:

			if ( StrafeStick -> GetRawButton ( 10 ) )
				Winch -> DrivePWM ( 0.5 );
			else if ( StrafeStick -> GetRawButton ( 11 ) )
				Winch -> DrivePWM ( - 0.5 );
			else
				Winch -> Stop ();

			if ( StrafeStick -> GetRawButton ( 3 ) )
				TestInit ();

			break;
		case 2:

			if ( StrafeStick -> GetRawButton ( 3 ) )
			{

				BallSensor -> CalibHighPoint ( 0.8 );

				TestPeriodMode = 4;

				DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line2, "** CALIBRATE BALL SENSE **" );
				DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line3, "Place ball 0.8m from," );
				DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line4, "bot, press 2." );
				DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line5, "" );
				DsLcd -> UpdateLCD ();

			}

			break;

		case 3:

			TestInit ();

			break;

		case 4:

			if ( StrafeStick -> GetRawButton ( 2 ) )
			{

				BallSensor -> CalibLowPoint ( 0.2 );

				TestInit ();

			}

			break;

		default:

			TestInit ();

			break;
	}

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
