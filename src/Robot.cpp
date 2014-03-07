#include "Robot.h"

Robot :: Robot ():
	WheelConfig (),
	BeltConfig (),
	WenchConfig ()
{

	Mode = RobotStartMode;

	DsLcd = DriverStationLCD :: GetInstance ();

	AutonomousTask = new Task ( "2605_Autonomous", (FUNCPTR) & Robot :: AutonomousTaskStub );
	TeleopTask = new Task ( "2605_Teleop", (FUNCPTR) & Robot :: TeleopTaskStub );
	VisionTask = new Task ( "2605_Vision", (FUNCPTR) & Robot :: VisionTaskStub, VISION_PRIORITY );

	TargetingCamera = & AxisCamera :: GetInstance ( "10.26.5.11" );

	InitMotors ();
	InitControls ();
	InitBehaviors ();

	//-----------------------------------------------//

	//LEDS = new LEDStrip ( 1, 2, 3, 10 );

	//TestAnimation = BuildTestAnimation ( LEDS );

};

void Robot :: InitControls ()
{

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

	printf ( "INIT MOTORS\n" );

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

	// WENCH

	WenchM = new AsynchCANJaguar ( JagServer, 9, WenchConfig );

	Wench = new ShooterWench ( WenchM );

	// ARMS

	printf ( "PICSERVO\n" );

	PICServoControl -> AddPICServo ( 1, false, 4, 3 );
	PICServoControl -> AddPICServo ( 2, false, 3, 4 );

	ArmL = PICServoControl -> GetModule ( 1 );

	ArmL -> SetPID ( 0.35, 0.0, 0.2, 0.62745 );
	ArmL -> SetEncoderResolution ( 360 * 4 );
	ArmL -> ConfigVelocity ( 0.4 );
	ArmL -> ConfigAcceleration ( 0.02 );

	ArmR = PICServoControl -> GetModule ( 2 );

	ArmR -> SetPID ( 0.35, 0.0, 0.2, 0.62745 );
	ArmR -> SetEncoderResolution ( 360 * 4 );
	ArmR -> ConfigVelocity ( 0.4 );
	ArmR -> ConfigAcceleration ( 0.02 );

	printf ( "COLLECTORARMS\n" );

	Arms = new CollectorArms ( ArmL, ArmR );

	Arms -> SetInverted ( false, true );
	Arms -> SetFreeDrivePower ( 0.3 );

};

void Robot :: InitBehaviors ()
{

	printf ( "INIT BEHAVIORS\n" );

	Behaviors = new BehaviorController ();

	TELEOP_DRIVE_BEHAVIOR = "TeleopDrive";
	TeleopDrive = new TeleopDriveBehavior ( Drive, StrafeStick, RotateStick, GearStepper, OnShiftDelegate, Arms );

	Behaviors -> AddBehavior ( TELEOP_DRIVE_BEHAVIOR, TeleopDrive );

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

	DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line1, "Autonomous" );
	DsLcd -> UpdateLCD ();

	TargetFound = false;

	VisionTask -> Start ( reinterpret_cast <uint32_t> ( this ) );
	AutonomousTask -> Start ( reinterpret_cast <uint32_t> ( this ) );

	//LEDS -> Clear ();

	ArmL -> SetControlMode ( PICServo :: kPWM );
	ArmL -> Enable ();

};

void Robot :: AutonomousPeriodic ()
{

	/*TestAnimation -> Update ();

	if ( LEDS -> HasNewData () )
		LEDS -> PushColors ();*/

};

void Robot :: AutonomousEnd ()
{

	/*LEDS -> Clear ();
	LEDS -> PushColors ();*/

	ArmL -> Disable ();

	AutonomousTask -> Stop ();
	VisionTask -> Stop ();

	printf ( "================\n= Autonomous X =\n================\n" );

};

void Robot :: AutonomousTaskRoutine ()
{



};

//============================================================//
/*==========================[TELEOP]==========================*/
//============================================================//

void Robot :: TeleopInit ()
{

	printf ( "================\n=   Teleop !    =\n================\n" );

	DisabledEnd ();
	Mode = TeleopMode;

	DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line1, "Teleop" );
	
	Drive -> Enable ();
	Shooter -> Enable ();
	Wench -> Enable ();

	Behaviors -> StartBehavior ( TELEOP_DRIVE_BEHAVIOR );

	TeleopTask -> Start ( reinterpret_cast <uint32_t> ( this ) );

};

void Robot :: TeleopPeriodic ()
{

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

	if ( RotateStick -> GetRawButton ( 4 ) )
		Wench -> Open ();
	else if ( RotateStick -> GetRawButton ( 5 ) )
		Wench -> Close ();
	else
		Wench -> Stop ();

	Wench -> PushSpeed ();

};

void Robot :: TeleopEnd ()
{

	Behaviors -> StopBehavior ( TELEOP_DRIVE_BEHAVIOR );

	printf ( "================\n=   Teleop X   =\n================\n" );
	DsLcd -> UpdateLCD ();

	TeleopTask -> Stop ();

	Drive -> Disable ();
	Shooter -> Disable ();
	Wench -> Disable ();

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

	DisabledEnd ();
	Mode = TestMode;

	DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line1, "Test" );
	DsLcd -> UpdateLCD ();

	Arms -> Enable ();

};

void Robot :: TestPeriodic ()
{

	if ( ShootStick -> GetRawButton ( 6 ) )
		Arms -> DriveToLimitsAndCalibrate ();

};

void Robot :: TestEnd ()
{

	ArmL -> Disable ();

	printf ( "================\n=    Test X    =\n================\n" );

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

	printf ( "================\n=  Disabled !  =\n================\n" );

	DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line1, "Disabled\n" );
	DsLcd -> UpdateLCD ();

};

void Robot :: DisabledPeriodic ()
{

	Arms -> CalibratePICServoAnalogs ();

};

void Robot :: DisabledEnd ()
{

	printf ( "================\n=  Disabled X  =\n================\n" );

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
