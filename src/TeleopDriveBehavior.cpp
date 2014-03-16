#include "TeleopDriveBehavior.h"

TeleopDriveBehavior :: TeleopDriveBehavior ( MecanumDrive * DriveSystem, Joystick * Strafe, Joystick * Rotate, NumericStepper * GearStepper, Delegate <void> * OnShiftDelegate, CollectorArms * ArmSystem, ShooterBelts * Belts, Joystick * Cancel, ShooterWinch * Winch )
{

	Drive = DriveSystem;

	StrafeStick = Strafe;
	RotateStick = Rotate;
	CancelStick = Cancel;

	DriveVelocityRamp = new ExponentialFilter ( DRIVE_RESPONSE_CURVE );
	JoystickDeadband = new DeadbandFilter ( range_t ( -0.08, 0.08 ) );

	Gear = GearStepper;
	OnShift = OnShiftDelegate;

	Arms = ArmSystem;

	this -> Belts = Belts;
	this -> Winch = Winch;

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

	if ( ! Belts -> GetEnabled () )
		Belts -> Enable ();

	if ( ! Winch -> GetEnabled () )
		Winch -> Enable ();

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
	Winch -> Disable ();

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
		Belts -> SetSpeed ( 1.0 );
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

	if ( CancelStick -> GetRawButton ( 3 ) )
		Arms -> DrivePositions ( T_ARM_LEFT_EMERGENCY_IN, T_ARM_RIGHT_EMERGENCY_IN );
	else
		Arms -> DrivePositions ( T_ARM_LEFT_OUT + ( CancelStick -> GetZ () + 1 ) / 7, T_ARM_RIGHT_OUT - ( CancelStick -> GetZ () + 1 ) / 7 );

	if ( CancelStick -> GetRawButton ( 8 ) )
		Winch -> DrivePWM ( - 0.5 );
	else if ( CancelStick -> GetRawButton ( 9 ) )
		Winch -> DrivePWM ( 0.5 );
	else
		Winch -> DriveAngle ( Winch -> GetAngle () );

}
