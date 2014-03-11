#include "TeleopDriveBehavior.h"

TeleopDriveBehavior :: TeleopDriveBehavior ( MecanumDrive * DriveSystem, Joystick * Strafe, Joystick * Rotate, NumericStepper * GearStepper, Delegate <void> * OnShiftDelegate, CollectorArms * ArmSystem )
{

	Drive = DriveSystem;
	StrafeStick = Strafe;
	RotateStick = Rotate;

	DriveVelocityRamp = new ExponentialFilter ( DRIVE_RESPONSE_CURVE );
	JoystickDeadband = new DeadbandFilter ( range_t ( -0.08, 0.08 ) );

	Gear = GearStepper;
	OnShift = OnShiftDelegate;

	Arms = ArmSystem;

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

	//if ( ! Arms -> GetEnabled () )
	//	Arms -> Enable ();

	Gear -> Set ( 1 );
	OnShift -> Call ();

};

void TeleopDriveBehavior :: Update ()
{

	ControlDrive ();
	ControlArms ();

};

void TeleopDriveBehavior :: Stop ()
{

	Drive -> Disable ();
	Arms -> Disable ();

};

void TeleopDriveBehavior :: Restart ()
{

	Gear -> Set ( 1 );
	OnShift -> Call ();

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

void TeleopDriveBehavior :: ControlArms ()
{

	if ( StrafeStick -> GetRawButton ( 11 ) )
		Arms -> DrivePWM ( 0.5 );
	if ( StrafeStick -> GetRawButton ( 10 ) )
		Arms -> DrivePWM ( - 0.5 );
	else
		Arms -> DrivePWM ( 0 );

}
