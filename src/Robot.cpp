#include "Robot.h"

Robot :: Robot ():
	WheelConfig (),
	BeltConfig (),
	WenchConfig ()
{

	printf ( "* 1\n" );

	Mode = RobotStartMode;

	DsLcd = DriverStationLCD :: GetInstance ();

	printf ( "* 2\n" );

	AutonomousTask = new Task ( "2605_Autonomous", (FUNCPTR) & Robot :: AutonomousTaskStub );
	TeleopTask = new Task ( "2605_Teleop", (FUNCPTR) & Robot :: TeleopTaskStub );
	VisionTask = new Task ( "2605_Vision", (FUNCPTR) & Robot :: VisionTaskStub, VISION_PRIORITY );

	TargetingCamera = & AxisCamera :: GetInstance ( "10.26.5.11" );

	printf ( "* 3\n" );

	PICServoControl = new PICServoController ();

	printf ( "* 4\n" );

	InitControls ();

	printf ( "* 5\n" );

	InitMotors ();

	printf ( "* 6\n" );

	PICServoControl -> AddPICServo ( 1, false, 4, 3 );

	printf ( "* 7\n" );

	TestPICServo = PICServoControl -> GetModule ( 1 );
	TestPICServo -> SetControlMode ( PICServo :: kPosition );
	TestPICServo -> ConfigVelocity ( 1 );
	TestPICServo -> ConfigAcceleration ( 1 );
	//TestPICServo -> SetPID ( 0.5, 0.01, 0.2 );
	TestPICServo -> SetEncoderResolution ( 360 * 4 );

	//-----------------------------------------------//

	//LEDS = new LEDStrip ( 1, 2, 3, 10 );

	//TestAnimation = BuildTestAnimation ( LEDS );

	printf ( "* 8\n" );

};

void Robot :: InitControls ()
{

	printf ( "* A\n" );

	StrafeStick = new Joystick ( 1 );
	RotateStick = new Joystick ( 2 );
	ShootStick = new Joystick ( 3 );

	printf ( "* B\n" );

	DriveFilter = new ExponentialFilter ( DRIVE_RESPONSE_CURVE );
	StickFilter = new DeadbandFilter ( range_t ( -0.08, 0.08 ) );

	printf ( "* C\n" );

	OnShiftDelegate = new ClassDelegate <Robot, void> ( this, & Robot :: OnShift );

	printf ( "* D\n" );

	GearStepper = new NumericStepper ( StrafeStick, 3, 4 );

	printf ( "* E\n" );

	GearStepper -> SetRange ( 1, 3 );

	printf ( "* F\n" );

	GearStepper -> SetChangeListener ( OnShiftDelegate );

	printf ( "* G\n" );

	GearStepper -> Set ( 1 );

	printf ( "* H\n" );

	OnShift ();

};

void Robot :: InitMotors ()
{

	// CANJaguar Server

	JagServer = new CANJaguarServer ();
	JagServer -> Start ();

	// DRIVE

	WheelConfig.Mode = CANJaguar :: kSpeed;
	WheelConfig.P = P_GEAR1;
	WheelConfig.I = I_GEAR1;
	WheelConfig.D = D_GEAR1;
	WheelConfig.EncoderLinesPerRev = ENCODER_CODES_PER_REVOLUTION;
	WheelConfig.SpeedRef = CANJaguar :: kSpeedRef_QuadEncoder;
	WheelConfig.NeutralAction = CANJaguar :: kNeutralMode_Coast;

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
	BeltConfig.P = BELT_P;
	BeltConfig.P = BELT_P;
	BeltConfig.EncoderLinesPerRev = BELT_ENCODER_CODES_PER_REVOLUTION;
	BeltConfig.SpeedRef = CANJaguar :: kSpeedRef_QuadEncoder;
	BeltConfig.NeutralAction = CANJaguar :: kNeutralMode_Coast;
	BeltConfig.MaxVoltage = 14;

	BeltL = new AsynchCANJaguar ( JagServer, 8, BeltConfig );
	BeltR = new AsynchCANJaguar ( JagServer, 6, BeltConfig );

	Shooter = new ShooterBelts ( BeltL, BeltR );

	Shooter -> SetMotorScale ( BELT_SPEED_SCALE );
	Shooter -> SetInverted ( false, true );

	// WENCH

	printf ( "* ^\n" );

	WenchM = new AsynchCANJaguar ( JagServer, 9, WenchConfig );

	printf ( "* &\n" );

	Wench = new ShooterWench ( WenchM );

};

Robot :: ~Robot ()
{



};

//============================================================//
/*========================[DRIVE STUFF]=======================*/
//============================================================//

void Robot :: OnShift ()
{

	printf ( "* >\n" );

	ShiftVGear ( GearStepper -> Get () );

	printf ( "* <\n" );

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

	printf ( "A" );

	Mode = TeleopMode;

	DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line1, "Teleop" );

	printf ( "B" );
	
	GearStepper -> Set ( 1 );

	printf ( "C" );

	OnShift ();

	printf ( "C" );
	
	Drive -> Enable ();

	printf ( "E" );

	Shooter -> Enable ();

	printf ( "F" );

	Wench -> Enable ();

	printf ( "G" );

	TeleopTask -> Start ( reinterpret_cast <uint32_t> ( this ) );

};

void Robot :: TeleopPeriodic ()
{

	GearStepper -> Update ();

	double TX = StickFilter -> Compute ( - StrafeStick -> GetX () );
	double TY = StickFilter -> Compute ( StrafeStick -> GetY () );

	double StickAnge = atan2 ( TX, TY );
	double Magnitude = sqrt ( TX * TX + TY * TY );

	Magnitude = DriveFilter -> Compute ( Magnitude );

	TX = cos ( StickAnge ) * Magnitude;
	TX = sin ( StickAnge ) * Magnitude;

	Drive -> SetTranslation ( TX, TY );

	double TR_X = RotateStick -> GetX ();
	double TR_Y = RotateStick -> GetY ();
	double TR = sqrt ( TR_X * TR_X + TR_Y * TR_Y );

	TR *= ( TR_X >= 0 ) ? -1 : 1;
	TR = StickFilter -> Compute ( TR );

	Drive -> SetRotation ( TR );

	Drive -> PushTransform ();

	if ( ShootStick -> GetRawButton ( 1 ) )
	{

		Shooter -> SetSpeed ( 1.0 );

	}
	else if ( ShootStick -> GetRawButton ( 3 ) )
		Shooter -> SetSpeed ( 0.17 );
	else if ( ShootStick -> GetRawButton ( 2 ) )
		Shooter -> SetSpeed ( - 0.17 );
	else
		Shooter -> SetSpeed ( 0 );

	Shooter -> PushSpeeds ();

	if ( ShootStick -> GetRawButton ( 11 ) )
		Wench -> Open ();
	else if ( ShootStick -> GetRawButton ( 10 ) )
		Wench -> Close ();
	else
		Wench -> Stop ();

	Wench -> PushSpeed ();

};

void Robot :: TeleopEnd ()
{

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

	TestPICServo -> ResetPosition ();

	TestPICServo -> Enable ();

};

void Robot :: TestPeriodic ()
{

	if ( StrafeStick -> GetRawButton ( 6 ) )
	{
		
		TestPICServo -> Set ( 0.25 );

	}
	else
	{

		TestPICServo -> Set ( 0 );

	}

	printf ( "Position: %f\n", TestPICServo -> GetPosition () );

};

void Robot :: TestEnd ()
{

	TestPICServo -> Disable ();

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
