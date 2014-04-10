#include "Robot.h"

Robot :: Robot ():
	WheelConfig360 (),
	WheelConfig480 (),
	MasterBeltConfig (),
	SlaveBeltConfig (),
	ArmServoConfig ()
{

	Log = Logger :: GetInstance ();
	Log -> SetPrintLevel ( Logger :: LOG_DEBUG );


	Log -> Log ( Logger :: LOG_EVENT, "** Robot Starting **\n" );

	Mode = RobotStartMode;

	DsLcd = DriverStationLCD :: GetInstance ();
	DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line1, "Disabled\n" );
	DsLcd -> UpdateLCD ();

	Log -> Log ( Logger :: LOG_EVENT, "Opening config file...\n" );

	RobotConfig = new ConfigFile ();

	if ( ! RobotConfig -> Init () )
		Log -> Log ( Logger :: LOG_WARNING, "Unable to open config file!\n" );
	else
		Log -> Log ( Logger :: LOG_EVENT, "Config opened!\n" );

	InitVision ();
	InitSensors ();
	InitMotors ();
	InitControls ();
	InitBehaviors ();
	InitDecorations ();
	LoadConfiguration ();

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

};

void Robot :: InitSensors ()
{

	Log -> Log ( Logger :: LOG_EVENT, "Initializing Sensors\n" );

	// BALL DISTANCE IR

	DistanceSensorAnalog = new AnalogChannel ( 1, 2 );
	BallSensor = new IRDistanceSensor ( DistanceSensorAnalog );

	BallSensorConfig  = new IRDistanceConfig ( BallSensor );
	RobotConfig -> AddConfigSection ( BallSensorConfig );

	// BALL POSITION LIMIT SWITCH

	BallLimit = new DigitalInput ( 10 );

};

void Robot :: InitMotors ()
{

	Log -> Log ( Logger :: LOG_DEBUG, "Initializing Motors\n" );

	// CANJaguar Server

	JagServer = new CANJaguarServer ();

	JagServer -> Start ();
	JagServer -> WaitForServerActive ();

	// DRIVE

	WheelConfig360.Mode = CANJaguar :: kSpeed;
	WheelConfig360.P = P_GEAR1;
	WheelConfig360.I = I_GEAR1;
	WheelConfig360.D = D_GEAR1;
	WheelConfig360.EncoderLinesPerRev = ENCODER_CODES_PER_REVOLUTION_360;
	WheelConfig360.SpeedRef = CANJaguar :: kSpeedRef_QuadEncoder;
	WheelConfig360.NeutralAction = CANJaguar :: kNeutralMode_Brake;
	WheelConfig360.FaultTime = 0.51;

	WheelConfig480 = WheelConfig360;
	WheelConfig480.EncoderLinesPerRev = ENCODER_CODES_PER_REVOLUTION_480;

	WheelFL = new AsynchCANJaguar ( JagServer, 7, WheelConfig480 );
	WheelFR = new AsynchCANJaguar ( JagServer, 1, WheelConfig360 );
	WheelRL = new AsynchCANJaguar ( JagServer, 6, WheelConfig480 );
	WheelRR = new AsynchCANJaguar ( JagServer, 2, WheelConfig360 );

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

	/*MasterBeltConfig.Mode = CANJaguar :: kVoltage;
	MasterBeltConfig.NeutralAction = CANJaguar :: kNeutralMode_Coast;
	MasterBeltConfig.MaxVoltage = 14;
	MasterBeltConfig.FaultTime = 0.51;*/

	SlaveBeltConfig.Mode = CANJaguar :: kVoltage;
	SlaveBeltConfig.NeutralAction = CANJaguar :: kNeutralMode_Coast;
	SlaveBeltConfig.MaxVoltage = 14;
	SlaveBeltConfig.FaultTime = 0.51;

	BeltL = new AsynchCANJaguar ( JagServer, 5, MasterBeltConfig );
	BeltR = new AsynchCANJaguar ( JagServer, 8, MasterBeltConfig );
	BeltLSlave = new AsynchCANJaguar ( JagServer, 10, SlaveBeltConfig );
	BeltRSlave = new AsynchCANJaguar ( JagServer, 11, SlaveBeltConfig );

	Belts = new ShooterBelts ( BeltL, BeltR );

	Belts -> SetMotorScale ( BELT_SPEED_SCALE );
	Belts -> SetInverted ( true, false );

	// ARMS

	ArmServoConfig.Mode = CANJaguar :: kPosition;
	ArmServoConfig.P = 200.0;
	ArmServoConfig.I = 0.0;
	ArmServoConfig.D = 10.0;
	ArmServoConfig.PotentiometerTurns = 10;
	ArmServoConfig.PosRef = CANJaguar :: kPosRef_Potentiometer;
	ArmServoConfig.NeutralAction = CANJaguar :: kNeutralMode_Coast;
	ArmServoConfig.MaxVoltage = 10.0;
	ArmServoConfig.FaultTime = 0.51;
	ArmServoConfig.LowPosLimit = 0.5;
	ArmServoConfig.HighPosLimit = 9.5;
	ArmServoConfig.Limiting = CANJaguar :: kLimitMode_SoftPositionLimits;

	ArmFreeConfig = ArmServoConfig;
	ArmFreeConfig.Mode = CANJaguar :: CANJaguar :: kPercentVbus;

	ArmL = new AsynchCANJaguar ( JagServer, 4, ArmServoConfig );
	ArmR = new AsynchCANJaguar ( JagServer, 3, ArmServoConfig );

	Arms = new CollectorArms ( ArmL, ArmR, ArmServoConfig, ArmFreeConfig );
	Arms -> SetPreScale ( 1.0 );
	Arms -> SetInverted ( false, false );
	Arms -> Disable ();

	ArmsConfig = new ArmConfig ( Arms );
	RobotConfig -> AddConfigSection ( ArmsConfig );

	// WINCH

	WinchServoConfig.Mode = CANJaguar :: kPosition;
	WinchServoConfig.P = 2000.0;
	WinchServoConfig.I = 0.0;
	WinchServoConfig.D = 8.0;
	WinchServoConfig.PotentiometerTurns = 10;
	WinchServoConfig.PosRef = CANJaguar :: kPosRef_Potentiometer;
	WinchServoConfig.NeutralAction = CANJaguar :: kNeutralMode_Brake;
	WinchServoConfig.MaxVoltage = 12.0;
	WinchServoConfig.FaultTime = 0.51;
	WinchServoConfig.LowPosLimit = 0.5;
	WinchServoConfig.HighPosLimit = 9.5;
	WinchServoConfig.Limiting = CANJaguar :: kLimitMode_SoftPositionLimits;

	WinchFreeConfig = WinchServoConfig;
	WinchFreeConfig.Mode = CANJaguar :: kVoltage;

	WinchM = new AsynchCANJaguar ( JagServer, 9, WinchServoConfig );

	Winch = new ShooterWinch ( WinchM, WinchServoConfig, WinchFreeConfig );
	Winch -> SetInverted ( true );

	WinchConf = new WinchConfig ( Winch );
	RobotConfig -> AddConfigSection ( WinchConf );

};

void Robot :: InitControls ()
{

	Log -> Log ( Logger :: LOG_DEBUG, "Initializing Controls\n" );

	StrafeStick = new Joystick ( 1 );
	RotateStick = new Joystick ( 2 );
	CancelStick = new Joystick ( 3 );

	OnShiftDelegate = new ClassDelegate <Robot, void> ( this, & Robot :: OnShift );

	GearStepper = new NumericStepper ( StrafeStick, 5, 4 );
	GearStepper -> SetRange ( 1, 3 );
	GearStepper -> SetChangeListener ( OnShiftDelegate );
	GearStepper -> Set ( 2 );

	OnShift ();

};

void Robot :: InitBehaviors ()
{

	Log -> Log ( Logger :: LOG_DEBUG, "Initializing Behaviors\n" );

	Behaviors = new BehaviorController ();

	TELEOP_DRIVE_BEHAVIOR = "TeleopDrive";
	TeleopDrive = new TeleopDriveBehavior ( Drive, Belts, Arms, Winch, StrafeStick, RotateStick, CancelStick, GearStepper, OnShiftDelegate );

	EMERGENCEY_ARMS_BEHAVIOR = "EmergenceyArms";
	EmergenceyArms = new EmergenceyArmsBehavior ( Arms );

	BALL_PICKUP_BEHAVIOR = "BallPickup";
	BallPickup = new BallPickupBehavior ( Drive, Arms, Winch, Belts, GearStepper, OnShiftDelegate, BallSensor, BallLimit );

	AUTONOMOUS_BEHAVIOR = "Autonomous";
	Autonomous = new AutonomousBehavior ( Drive, Belts, Arms, Winch, OnShiftDelegate, GearStepper );

	Behaviors -> AddBehavior ( TELEOP_DRIVE_BEHAVIOR, TeleopDrive );
	Behaviors -> AddBehavior ( EMERGENCEY_ARMS_BEHAVIOR, EmergenceyArms );
	Behaviors -> AddBehavior ( BALL_PICKUP_BEHAVIOR, BallPickup );
	Behaviors -> AddBehavior ( AUTONOMOUS_BEHAVIOR, Autonomous );

};

void Robot :: InitDecorations ()
{

};

void Robot :: LoadConfiguration ()
{

	BallSensorConfig -> LoadSensorCalibration ();
	ArmsConfig -> LoadArmZeros ();
	WinchConf -> LoadZero ();

}

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

			WheelConfig360.P = P_GEAR1;
			WheelConfig360.I = I_GEAR1;
			WheelConfig360.D = D_GEAR1;
			WheelConfig480.P = P_GEAR1;
			WheelConfig480.I = I_GEAR1;
			WheelConfig480.D = D_GEAR1;

			WheelFL -> Configure ( WheelConfig480 );
			WheelFR -> Configure ( WheelConfig360 );
			WheelRL -> Configure ( WheelConfig480 );
			WheelRR -> Configure ( WheelConfig360 );

			break;

		case 2:

			Drive -> SetMotorScale ( SPEED_SCALE_GEAR2 );

			GearRPM = SPEED_SCALE_GEAR2;

			WheelConfig360.P = P_GEAR2;
			WheelConfig360.I = I_GEAR2;
			WheelConfig360.D = D_GEAR2;
			WheelConfig480.P = P_GEAR2;
			WheelConfig480.I = I_GEAR2;
			WheelConfig480.D = D_GEAR2;

			WheelFL -> Configure ( WheelConfig480 );
			WheelFR -> Configure ( WheelConfig360 );
			WheelRL -> Configure ( WheelConfig480 );
			WheelRR -> Configure ( WheelConfig360 );

			break;

		case 3:

			Drive -> SetMotorScale ( SPEED_SCALE_GEAR3 );

			GearRPM = SPEED_SCALE_GEAR3;
	
			WheelConfig360.P = P_GEAR3;
			WheelConfig360.I = I_GEAR3;
			WheelConfig360.D = D_GEAR3;
			WheelConfig480.P = P_GEAR3;
			WheelConfig480.I = I_GEAR3;
			WheelConfig480.D = D_GEAR3;

			WheelFL -> Configure ( WheelConfig480 );
			WheelFR -> Configure ( WheelConfig360 );
			WheelRL -> Configure ( WheelConfig480 );
			WheelRR -> Configure ( WheelConfig360 );

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

};

//============================================================//
//==========================[COMMON]==========================//
//============================================================//

void Robot :: PeriodicCommon ()
{

	Behaviors -> Update ();

	if ( IsEnabled () )
	{

		BeltLSlave -> Set ( BeltL -> GetOutputVoltage () * ( ( BeltL -> Get () > 0 ) ? - 1 : 1 ) );
		BeltRSlave -> Set ( BeltR -> GetOutputVoltage () * ( ( BeltR -> Get () > 0 ) ? - 1 : 1 ) );

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

	Behaviors -> StartBehavior ( AUTONOMOUS_BEHAVIOR );

};

void Robot :: AutonomousPeriodic ()
{

	AutoCount ++;

	PeriodicCommon ();

};

void Robot :: AutonomousEnd ()
{

	Log -> Log ( Logger :: LOG_EVENT, "================\n= Autonomous X =\n================\n" );

	Behaviors -> StopBehavior ( AUTONOMOUS_BEHAVIOR );

	ArmsConfig -> SetArmZeros ();
	WinchConf -> SetZero ();
	RobotConfig -> Write ();

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
	DsLcd -> UpdateLCD ();

	Behaviors -> StartBehavior ( TELEOP_DRIVE_BEHAVIOR );

	LowestVoltage = 14.0;

};

void Robot :: TeleopPeriodic ()
{

	TeleCount ++;

	PeriodicCommon ();

	if ( Behaviors -> GetBehaviorActive ( TELEOP_DRIVE_BEHAVIOR ) )
	{

		if ( TeleopDrive -> DoEmergenceyArms () && ! Behaviors -> GetBehaviorActive ( EMERGENCEY_ARMS_BEHAVIOR ) )
			Behaviors -> StartBehavior ( EMERGENCEY_ARMS_BEHAVIOR );

		if ( ! TeleopDrive -> DoEmergenceyArms () && Behaviors -> GetBehaviorActive ( EMERGENCEY_ARMS_BEHAVIOR ) )
			Behaviors -> StopBehavior ( EMERGENCEY_ARMS_BEHAVIOR );

		if ( TeleopDrive -> DoPickup () && ! Behaviors -> GetBehaviorActive ( BALL_PICKUP_BEHAVIOR ) )
			Behaviors -> StartBehavior ( BALL_PICKUP_BEHAVIOR );

		if ( ! TeleopDrive -> DoPickup () && Behaviors -> GetBehaviorActive ( BALL_PICKUP_BEHAVIOR ) )
			Behaviors -> StopBehavior ( BALL_PICKUP_BEHAVIOR );

	}

	if ( m_ds -> GetBatteryVoltage () < LowestVoltage )
		LowestVoltage = m_ds -> GetBatteryVoltage ();

};

void Robot :: TeleopEnd ()
{

	Log -> Log ( Logger :: LOG_EVENT, "================\n=   Teleop X   =\n================\n" );

	if ( Behaviors -> GetBehaviorActive ( TELEOP_DRIVE_BEHAVIOR ) )
		Behaviors -> StopBehavior ( TELEOP_DRIVE_BEHAVIOR );

	if ( Behaviors -> GetBehaviorActive ( EMERGENCEY_ARMS_BEHAVIOR ) )
		Behaviors -> StopBehavior ( EMERGENCEY_ARMS_BEHAVIOR );

	if ( Behaviors -> GetBehaviorActive ( BALL_PICKUP_BEHAVIOR ) )
		Behaviors -> StopBehavior ( BALL_PICKUP_BEHAVIOR );

	TeleopTask -> Stop ();

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
	DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line4, "Press 3 To Cal." );
	DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line5, "Press 8 To Save." );
	DsLcd -> UpdateLCD ();

	TestPeriodMode = 0;
	LastSaveButtonState = false;

};

void Robot :: TestPeriodic ()
{

	TestCount ++;

	PeriodicCommon ();

	switch ( TestPeriodMode )
	{

	case 0:

		if ( StrafeStick -> GetRawButton ( 3 ) )
		{

			BallSensor -> CalibHighPoint ( 0.2 );

			BallSensorConfig -> SetSensorCalibration ();

			TestPeriodMode = 1;

			DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line3, "0.8 Meters," );
			DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line4, "Press 2 To Cal." );

		}

		break;

	case 1:

		if ( StrafeStick -> GetRawButton ( 2 ) )
		{

			BallSensor -> CalibLowPoint ( 0.8 );

			BallSensorConfig -> SetSensorCalibration ();

			TestPeriodMode = 0;

			DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line3, "0.2 Meters," );
			DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line4, "Press 3 To Cal." );

		}

		break;

	default:
		break;

	}

	if ( StrafeStick -> GetRawButton ( 8 ) && ! LastSaveButtonState )
		RobotConfig -> Write ();

	LastSaveButtonState = StrafeStick -> GetRawButton ( 8 );

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

	DsLcd -> Clear ();
	DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line1, "Disabled\n" );
	DsLcd -> UpdateLCD ();

	Belts -> Disable ();
	Drive -> Disable ();
	Arms -> Disable ();
	Winch -> Disable ();

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

//============================================================//
/*=========================[DISASTER]=========================*/
//============================================================//

void Robot :: HolyFuck ()
{

	Belts -> Disable ();
	Drive -> Disable ();
	Arms -> Disable ();
	Winch -> Disable ();

};

START_ROBOT_CLASS ( Robot );
