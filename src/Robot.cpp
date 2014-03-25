#include "Robot.h"

Robot :: Robot ():
	WheelConfig (),
	MasterBeltConfig (),
	SlaveBeltConfig (),
	ArmConfig ()
{

	Log = Logger :: GetInstance ();
	Log -> SetPrintLevel ( Logger :: LOG_DEBUG2 );

	Log -> Log ( Logger :: LOG_EVENT, "** Robot Starting **\n" );

	Mode = RobotStartMode;

	DsLcd = DriverStationLCD :: GetInstance ();

	InitVision ();
	InitSensors ();
	InitMotors ();
	InitControls ();
	InitBehaviors ();
	InitDecorations ();

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

	DistanceSensorAnalog = new AnalogChannel ( 1, 2 );
	BallSensor = new IRDistanceSensor ( DistanceSensorAnalog );

	BallPositionSwitch = new DigitalInput ( 4 );

};

void Robot :: InitControls ()
{

	Log -> Log ( Logger :: LOG_EVENT, "Initializing Controls\n" );

	StrafeStick = new Joystick ( 1 );
	RotateStick = new Joystick ( 2 );
	CancelStick = new Joystick ( 3 );

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

	// CANJaguar Server

	JagServer = new CANJaguarServer ();
	JagServer -> Start ();
	//JagServer -> SetJagCheckInterval ( 0.01 );

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
	WheelRL = new AsynchCANJaguar ( JagServer, 6, WheelConfig );
	WheelRR = new AsynchCANJaguar ( JagServer, 2, WheelConfig );

	Drive = new MecanumDrive ( WheelFL, WheelFR, WheelRL, WheelRR );

	Drive -> SetMotorScale ( SPEED_SCALE_GEAR1 );
	Drive -> SetPreScale ( 1, 1 );
	Drive -> SetInverted ( false, true, false, true );
	Drive -> SetSineInversion ( false );
	Drive -> Disable ();

	// BELTS

	MasterBeltConfig.Mode = CANJaguar :: kSpeed;
	MasterBeltConfig.P = BELT_P;
	MasterBeltConfig.I = BELT_I;
	MasterBeltConfig.D = BELT_D;
	MasterBeltConfig.EncoderLinesPerRev = BELT_ENCODER_CODES_PER_REVOLUTION;
	MasterBeltConfig.SpeedRef = CANJaguar :: kSpeedRef_QuadEncoder;
	MasterBeltConfig.NeutralAction = CANJaguar :: kNeutralMode_Coast;
	MasterBeltConfig.MaxVoltage = 14;
	MasterBeltConfig.FaultTime = 0.51;

	SlaveBeltConfig.Mode = CANJaguar :: kVoltage;
	SlaveBeltConfig.NeutralAction = CANJaguar :: kNeutralMode_Coast;
	SlaveBeltConfig.MaxVoltage = 14;
	SlaveBeltConfig.FaultTime = 0.51;

	BeltL = new AsynchCANJaguar ( JagServer, 8, MasterBeltConfig );
	BeltR = new AsynchCANJaguar ( JagServer, 5, MasterBeltConfig );
	BeltLSlave = new AsynchCANJaguar ( JagServer, 10, SlaveBeltConfig );
	BeltRSlave = new AsynchCANJaguar ( JagServer, 11, SlaveBeltConfig );

	Belts = new ShooterBelts ( BeltL, BeltR );

	Belts -> SetMotorScale ( BELT_SPEED_SCALE );
	Belts -> SetInverted ( false, true );

	ArmConfig.Mode = CANJaguar :: kPosition;
	ArmConfig.P = 500.0;
	ArmConfig.I = 0;
	ArmConfig.D = 0;
	ArmConfig.PotentiometerTurnsPerRev = 10;
	ArmConfig.PosRef = CANJaguar :: kPosRef_Potentiometer;
	ArmConfig.NeutralAction = CANJaguar :: kNeutralMode_Coast;
	ArmConfig.MaxVoltage = 14;
	ArmConfig.FaultTime = 0.51;
	ArmConfig.LowPosLimit = 0.5;
	ArmConfig.HighPosLimit = 9.5;
	ArmConfig.Limiting = CANJaguar :: kLimitMode_SoftPositionLimits;

	ArmL = new AsynchCANJaguar ( JagServer, 3, ArmConfig );
	ArmR = new AsynchCANJaguar ( JagServer, 4, ArmConfig );

	Arms = new CollectorArms ( ArmL, ArmR );
	Arms -> SetPreScale ( 1.0 );
	Arms -> SetInverted ( false, false );
	Arms -> SetZeros ();

};

void Robot :: InitBehaviors ()
{

	Log -> Log ( Logger :: LOG_EVENT, "Initializing Behaviors\n" );

	Behaviors = new BehaviorController ();

	TELEOP_DRIVE_BEHAVIOR = "TeleopDrive";
	TeleopDrive = new TeleopDriveBehavior ( Drive, Belts, Arms, StrafeStick, RotateStick, CancelStick, GearStepper, OnShiftDelegate );

	Behaviors -> AddBehavior ( TELEOP_DRIVE_BEHAVIOR, TeleopDrive );

};

void Robot :: InitDecorations ()
{

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
//==========================[COMMON]==========================//
//============================================================//

void Robot :: PeriodicCommon ()
{

	if ( IsEnabled () )
	{

		BeltLSlave -> Set ( BeltL -> GetOutputVoltage () * ( ( BeltL -> Get () > 0 ) ? 1 : - 1 ) );
		BeltRSlave -> Set ( BeltR -> GetOutputVoltage () * ( ( BeltR -> Get () > 0 ) ? 1 : - 1 ) );

	}

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

	Log -> Log ( Logger :: LOG_DEBUG, "AUTO!\n" );

	GearStepper -> Set ( 2 );
	OnShift ();

};

void Robot :: AutonomousPeriodic ()
{

	AutoCount ++;

	//Behaviors -> Update ();

	PeriodicCommon ();

	Log -> Log ( Logger :: LOG_DEBUG, "Left: %f, Right: %f\n", Arms -> GetPositionLeft (), Arms -> GetPositionRight () );

};

void Robot :: AutonomousEnd ()
{

	Arms -> Disable ();

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

	LowestVoltage = 14.0;

	Arms -> SetZeros ();
	Arms -> Enable ();

};

void Robot :: TeleopPeriodic ()
{

	TeleCount ++;

	Behaviors -> Update ();

	if ( m_ds -> GetBatteryVoltage () < LowestVoltage )
		LowestVoltage = m_ds -> GetBatteryVoltage ();

	PeriodicCommon ();

	//Arms -> DrivePositions ( StrafeStick -> GetZ () / 4, RotateStick -> GetZ () / 4 );

};

void Robot :: TeleopEnd ()
{

	Log -> Log ( Logger :: LOG_EVENT, "================\n=   Teleop X   =\n================\n" );

	if ( Behaviors -> GetBehaviorActive ( TELEOP_DRIVE_BEHAVIOR ) )
		Behaviors -> StopBehavior ( TELEOP_DRIVE_BEHAVIOR );

	DsLcd -> UpdateLCD ();

	TeleopTask -> Stop ();

	Drive -> Disable ();
	Belts -> Disable ();

	Arms -> Disable ();

	Log -> Log ( Logger :: LOG_EVENT, "Lowest voltage for Teleop Run: %f\n", LowestVoltage );

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

	PeriodicCommon ();

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

	Belts -> Disable ();
	Drive -> Disable ();

	BeltLSlave -> Disable ();
	BeltRSlave -> Disable ();

};

void Robot :: DisabledPeriodic ()
{

	DisbCount ++;

	PeriodicCommon ();

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

START_ROBOT_CLASS ( Robot );

