#include "TeleopDriveBehavior.h"

TeleopDriveBehavior :: TeleopDriveBehavior ( MecanumDrive * DriveSystem, Joystick * Strafe, Joystick * Rotate, NumericStepper * GearStepper, Delegate <void> * OnShiftDelegate, ShooterBelts * Belts, Joystick * Cancel )
{

	Drive = DriveSystem;

	StrafeStick = Strafe;
	RotateStick = Rotate;
	CancelStick = Cancel;

	DriveVelocityRamp = new ExponentialFilter ( DRIVE_RESPONSE_CURVE );
	JoystickDeadband = new DeadbandFilter ( range_t ( -0.08, 0.08 ) );

	Gear = GearStepper;
	OnShift = OnShiftDelegate;

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

	if ( ! Belts -> GetEnabled () )
		Belts -> Enable ();

	Gear -> Set ( 1 );
	OnShift -> Call ();

	Pickup = false;

};

void TeleopDriveBehavior :: Update ()
{

	ControlDrive ();
	ControlBelts ();

};

void TeleopDriveBehavior :: Stop ()
{

	Drive -> Disable ();
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

};

void TeleopDriveBehavior :: ControlBelts ()
{

	if ( RotateStick -> GetRawButton ( 1 ) )
		Belts -> SetSpeed ( 0.99 );
	else if ( RotateStick -> GetRawButton ( 3 ) )
		Belts -> SetSpeed ( 0.1 );
	else if ( RotateStick -> GetRawButton ( 2 ) )
		Belts -> SetSpeed ( - 0.1 );
	else
		Belts -> SetSpeed ( 0 );

	Belts -> PushSpeeds ();

};

bool TeleopDriveBehavior :: DoPickup ()
{

	return Pickup;

};
