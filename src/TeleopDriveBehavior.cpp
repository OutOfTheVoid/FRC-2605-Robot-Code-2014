#include "TeleopDriveBehavior.h"

TeleopDriveBehavior :: TeleopDriveBehavior ( MecanumDrive * DriveSystem, Joystick * Strafe, Joystick * Rotate, NumericStepper * GearStepper, Delegate <void> * OnShiftDelegate, CollectorArms * ArmSystem, ShooterBelts * Belts )
{

	Drive = DriveSystem;
	StrafeStick = Strafe;
	RotateStick = Rotate;

	DriveVelocityRamp = new ExponentialFilter ( DRIVE_RESPONSE_CURVE );
	JoystickDeadband = new DeadbandFilter ( range_t ( -0.08, 0.08 ) );

	Gear = GearStepper;
	OnShift = OnShiftDelegate;

	Arms = ArmSystem;

	this -> Belts = Belts;

};

TeleopDriveBehavior :: ~TeleopDriveBehavior ()
{

	delete DriveVelocityRamp;
	delete JoystickDeadband;

};

void TeleopDriveBehavior :: Start ()
{

	if ( ! Drive -> GetEnabled () )
		Drive -> Enable ();

	if ( ! Arms -> GetEnabled () )
		Arms -> Enable ();

	if ( !Belts -> GetEnabled () )
		Belts -> Enable ();

	Gear -> Set ( 1 );
	OnShift -> Call ();

	Pickup = false;

};

void TeleopDriveBehavior :: Update ()
{

	ControlDrive ();
	ControlArms ();
	ControlBelts ();

};

void TeleopDriveBehavior :: Stop ()
{

	Drive -> Disable ();
	Arms -> Disable ();
	Belts -> Disable ();

};

void TeleopDriveBehavior :: Restart ()
{

	Gear -> Set ( 1 );
	OnShift -> Call ();

	Pickup = false;

};

void TeleopDriveBehavior :: ControlDrive ()
{

	Gear -> Update ();

	double TX = - JoystickDeadband -> Compute ( StrafeStick -> GetX () );
	double TY = JoystickDeadband -> Compute ( StrafeStick -> GetY () );

	double StickAnge = atan2 ( TX, TY );
	double Magnitude = sqrt ( TX * TX + TY * TY );

	Magnitude = DriveVelocityRamp -> Compute ( Magnitude );

	TX = cos ( StickAnge ) * Magnitude;
	TX = sin ( StickAnge ) * Magnitude;

	double TR = - JoystickDeadband -> Compute ( RotateStick -> GetX () );

	TR = DriveVelocityRamp -> Compute ( TR );

	Drive -> SetTranslation ( TX, TY );
	Drive -> SetRotation ( TR );

	Drive -> PushTransform ();

	Pickup = StrafeStick -> GetRawButton ( 2 );

};

void TeleopDriveBehavior :: ControlBelts ()
{

	if ( RotateStick -> GetRawButton ( 1 ) )
		Belts -> SetSpeed ( 1.0 );
	else if ( RotateStick -> GetRawButton ( 3 ) )
		Belts -> SetSpeed ( 0.17 );
	else if ( RotateStick -> GetRawButton ( 2 ) )
		Belts -> SetSpeed ( - 0.17 );
	else
		Belts -> SetSpeed ( 0 );

	Belts -> PushSpeeds ();

};

bool TeleopDriveBehavior :: DoPickup ()
{

	return Pickup;

};

void TeleopDriveBehavior :: ControlArms ()
{

	if ( StrafeStick -> GetRawButton ( 11 ) )
		Arms -> DrivePWM ( 0.5 );
	if ( StrafeStick -> GetRawButton ( 10 ) )
		Arms -> DrivePWM ( - 0.5 );
	else
		Arms -> DrivePositions ( ARM_LEFT_OUT, ARM_RIGHT_OUT );

}
