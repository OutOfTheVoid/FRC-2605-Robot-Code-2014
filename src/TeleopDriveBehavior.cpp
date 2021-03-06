#include "TeleopDriveBehavior.h"

TeleopDriveBehavior :: TeleopDriveBehavior ( MecanumDrive * Drive, ShooterBelts * Belts, CollectorArms * Arms, ShooterWinch * Winch, Joystick * Strafe, Joystick * Rotate, Joystick * Cancel, NumericStepper * GearStepper, Delegate <void> * OnShiftDelegate )
{

	StrafeStick = Strafe;
	RotateStick = Rotate;
	CancelStick = Cancel;

	DriveVelocityRamp = new ExponentialFilter ( DRIVE_RESPONSE_CURVE );
	JoystickDeadband = new DeadbandFilter ( range_t ( -0.08, 0.08 ) );

	Gear = GearStepper;
	OnShift = OnShiftDelegate;

	this -> Belts = Belts;
	this -> Drive = Drive;
	this -> Arms = Arms;
	this -> Winch = Winch;

	Log = Logger :: GetInstance ();

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

	if ( ! Arms -> GetEnabled () )
		Arms -> Enable ();

	if ( ! Winch -> GetEnabled () )
		Winch -> Enable ();

	Gear -> Set ( 1 );
	OnShift -> Call ();

	Pickup = false;
	EmergenceyArms = false;

};

void TeleopDriveBehavior :: Update ()
{

	EmergenceyArms = CancelStick -> GetRawButton ( 3 );
	Pickup = RotateStick -> GetRawButton ( 5 );

	ControlDrive ();
	ControlBelts ();

	if ( ! Pickup || ! EmergenceyArms )
	ControlArms ();

	if ( ! Pickup || ! EmergenceyArms )
		ControlWinch ();

};

void TeleopDriveBehavior :: Stop ()
{

	Drive -> Disable ();
	Belts -> Disable ();
	Arms -> Disable ();
	Winch -> Disable ();

};

void TeleopDriveBehavior :: Restart ()
{

	Start ();

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

	// MODIFIED FOR OPEN LOOP
	if ( RotateStick -> GetRawButton ( 1 ) )
		Belts -> SetSpeed ( 5.0 );
	else if ( RotateStick -> GetRawButton ( 3 ) )
		Belts -> SetSpeed ( 0.07 );
	else if ( RotateStick -> GetRawButton ( 2 ) )
		Belts -> SetSpeed ( - 0.07 );
	else
		Belts -> SetSpeed ( 0 );

	Belts -> PushSpeeds ();

};

void TeleopDriveBehavior :: ControlArms ()
{

	Arms -> DrivePositions ( CancelStick -> GetZ () / 2 + 0.6, CancelStick -> GetZ () / 2 + 0.80 );

};

void TeleopDriveBehavior :: ControlWinch ()
{

	//Winch -> DriveAngle ( ( CancelStick -> GetZ () + 1 ) / 2 );

	if ( CancelStick -> GetRawButton ( 6 ) )
		Winch -> DriveAngle ( ( 0.18 ) );
	else if ( CancelStick -> GetRawButton ( 7 ) )
		Winch -> DriveAngle ( 0.45 );
	else
		Winch -> DriveAngle ( 0.0 );

};

bool TeleopDriveBehavior :: DoPickup ()
{

	return Pickup;

};

bool TeleopDriveBehavior :: DoEmergenceyArms ()
{

	return EmergenceyArms;

};
