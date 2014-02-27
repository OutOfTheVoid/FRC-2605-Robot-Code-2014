#include "Robot.h"

Robot :: Robot ():
	WheelConfig (),
	BeltConfig (),
	ArmConfig (),
	WenchConfig ()
{

	Mode = RobotStartMode;

	DsLcd = DriverStationLCD :: GetInstance ();

	JagServer = new CANJaguarServer ();

	JagServer -> Start ();
	
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

	RotationGyro = new Gyro ( 1 );
	RotationGyro -> SetSensitivity ( 0.007 );
	RotationGyro -> SetPIDSourceParameter ( PIDSource :: kRate );

	RotationValue = new PIDOutputSensor ();
	RotationController = new PIDController ( ROTATION_P, ROTATION_I, ROTATION_D, ROTATION_F, RotationGyro, RotationValue );

	StrafeStick = new Joystick ( 1 );
	RotateStick = new Joystick ( 2 );
	ShootStick = new Joystick ( 3 );

	DriveFilter = new ExponentialFilter ( DRIVE_RESPONSE_CURVE );
	StickFilter = new DeadbandFilter ( range_t ( -0.08, 0.08 ) );

	AutonomousTask = new Task ( "2605_Autonomous", (FUNCPTR) & Robot :: AutonomousTaskStub );
	TeleopTask = new Task ( "2605_Teleop", (FUNCPTR) & Robot :: TeleopTaskStub );
	VisionTask = new Task ( "2605_Vision", (FUNCPTR) & Robot :: VisionTaskStub, VISION_PRIORITY );

	TargetingCamera = & AxisCamera :: GetInstance ( "10.26.5.11" );

	DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line2, "GEAR 1\n" );
	DsLcd -> UpdateLCD ();

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

	ArmConfig.Mode = CANJaguar :: kPosition;
	ArmConfig.P = ARM_P;
	ArmConfig.I = ARM_I;
	ArmConfig.D = ARM_D;
	ArmConfig.EncoderLinesPerRev = ARM_ENCODER_CODES_PER_REVOLUTION;
	ArmConfig.PosRef = CANJaguar :: kPosRef_QuadEncoder;
	ArmConfig.NeutralAction = CANJaguar :: kNeutralMode_Brake;
	ArmConfig.LowPosLimit = - 0.2;
	ArmConfig.HighPosLimit = 0.2;

	/*ArmL = new AsynchCANJaguar ( JagServer, 3, ArmConfig );
	ArmR = new AsynchCANJaguar ( JagServer, 4, ArmConfig );

	IntendedArmPosition = 0.0;

	Shooter = new ShooterBelts ( BeltL, BeltR, ArmL, ArmR );

	Shooter -> SetInverted ( false, true, true, false );

	Shooter -> SetBeltMotorScale (  BELT_SPEED_SCALE );
	Shooter -> SetArmMotorScale ( ARM_SPEED_SCALE );

	WenchM = new AsynchCANJaguar ( JagServer, 9, WenchConfig );

	Wench = new ShooterWench ( WenchM );
	Wench -> SetMotorScale ( WENCH_SPEED_SCALE );*/

	/*PICServoControl = new PICServoController ();

	printf ( "ADDING PIC SERVO...\n" );

	PICServoControl -> AddPICServo ( 1, false, 4, 3 );

	printf ( "ADDED!\n" );

	TestPICServo = PICServoControl -> GetModule ( 1 );
	TestPICServo -> SetControlMode ( PICServo :: kPWM );*/

	PServer = new AnalogCANJaguarPipeServer ();

	PServer -> Start ();
	AnalogCANJaguarPipe_t Pipe = PServer -> AddPipe ( 4, 3, 1 );
	PServer -> EnablePipe ( Pipe );

	SCom = new PICServoCom ();

};

Robot :: ~Robot ()
{



};

//============================================================//
/*========================[DRIVE STUFF]=======================*/
//============================================================//

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

};

void Robot :: AutonomousPeriodic ()
{



};

void Robot :: AutonomousEnd ()
{

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

	printf ( "[1]\n" );

	DisabledEnd ();
	Mode = TeleopMode;

	GearUpPreState = false;
	GearDownPreState = false;

	Gear = 1;
	ShiftVGear ( Gear );
	DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line2, "GEAR %i", Gear );

	DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line1, "Teleop" );
	DsLcd -> UpdateLCD ();

	Drive -> Enable ();
	Shooter -> Enable ();
	Wench -> Enable ();

	ArmL -> Enable ( 0 );
	ArmR -> Enable ( 0 );

	RotationController -> Reset ();
	RotationController -> Enable ();

	TeleopTask -> Start ( reinterpret_cast <uint32_t> ( this ) );

};

void Robot :: TeleopPeriodic ()
{

	if ( GearDownPreState && ! StrafeStick -> GetRawButton ( 4 ) )
		GearDownPreState = false;

	if ( GearUpPreState && ! StrafeStick -> GetRawButton ( 5 ) )
		GearUpPreState = false;

	if ( ! GearDownPreState && StrafeStick -> GetRawButton ( 4 ) )
	{

		if ( Gear > 1 )
		{

			Gear --;
			ShiftVGear ( Gear );

			DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line2, "GEAR %i", Gear );
			DsLcd -> UpdateLCD ();

			Wait ( 0.05 );

		}

		GearDownPreState = true;
		
	}

	if ( ! GearUpPreState && StrafeStick -> GetRawButton ( 5 ) )
	{

		if ( Gear < 3 )
		{

			Gear ++;
			ShiftVGear ( Gear );

			DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line2, "GEAR %i", Gear );
			DsLcd -> UpdateLCD ();

			Wait ( 0.05 );

		}

		GearUpPreState = true;

	}

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

	RotationController -> SetSetpoint ( TR * GearRPM * 1.5 );

	//Drive -> SetRotation ( - RotationValue -> GetOutput () );
	Drive -> SetRotation ( TR );

	Drive -> PushTransform ();

	if ( ShootStick -> GetRawButton ( 1 ) )
	{

		Shooter -> SetBeltSpeed ( 1.0 );

	}
	else if ( ShootStick -> GetRawButton ( 3 ) )
		Shooter -> SetBeltSpeed ( 0.17 );
	else if ( ShootStick -> GetRawButton ( 2 ) )
		Shooter -> SetBeltSpeed ( - 0.17 );
	else
		Shooter -> SetBeltSpeed ( 0 );

	if ( ShootStick -> GetRawButton ( 6 ) )
	{
		
		//Shooter -> SetArmPosition ( 2 );
		/*IntendedArmPosition += 0.01;

		DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line3, "Arm Position: %f", IntendedArmPosition );
		DsLcd -> UpdateLCD ();*/

	}

	else if ( ShootStick -> GetRawButton ( 7 ) )
	{

		//Shooter -> SetArmPosition ( - 2 );

		/*IntendedArmPosition -= 0.01;
		
		DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line3, "Arm Position: %f", IntendedArmPosition );
		DsLcd -> UpdateLCD ();*/

	}
	else
		Shooter -> SetArmPosition ( 0 );

	if ( IntendedArmPosition >= 0.2 )
		IntendedArmPosition = 0.2;

	if ( IntendedArmPosition <= -0.2 )
		IntendedArmPosition = -0.2;

	//Shooter -> SetArmPosition ( IntendedArmPosition );

	printf("ARM POSITION: LEFT: %f, RIGHT: %f\n", ArmL -> GetPosition (), ArmR -> GetPosition () );

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

	RotationController -> Disable ();

	IntendedArmPosition = 0;

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

	printf ( "================\n=    Test !    =\n================\n" );

	DsLcd -> PrintfLine ( DriverStationLCD :: kUser_Line1, "Test" );
	DsLcd -> UpdateLCD ();

};

void Robot :: TestPeriodic ()
{

	if ( StrafeStick -> GetRawButton ( 6 ) )
		SCom -> ModuleLoadTrajectory ( 1, 0, 0, 0, 20, false, false, false, true, false, false, false, true );
	else
		SCom -> ModuleLoadTrajectory ( 1, 0, 0, 0, 0, false, false, false, true, false, false, false, true );

	SCom -> ReceiveStatusPacket ();

};

void Robot :: TestEnd ()
{

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

	RotationGyro -> Reset ();

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
