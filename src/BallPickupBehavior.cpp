#include "BallPickupBehavior.h"

BallPickupBehavior :: BallPickupBehavior ( ShooterBelts * Belts, CollectorArms * Arms, MecanumDrive * Drive, NumericStepper * GearStepper, Delegate <void> * OnShiftDelegate, IRDistanceSensor * BallSensor, DigitalInput * BallLimit )
{

	this -> Belts = Belts;
	this -> Arms = Arms;
	this -> Drive = Drive;

	this -> Gear = GearStepper;
	this -> OnShift = OnShiftDelegate;

	this -> BallSensor = BallSensor;
	this -> BallLimit = BallLimit;

};

BallPickupBehavior :: ~BallPickupBehavior ()
{



};

void BallPickupBehavior :: Start ()
{

	State = STATE_START;

	Gear -> Set ( 2 );
	OnShift -> Call ();

	if ( ! Drive -> GetEnabled () )
		Drive -> Enable ();

	if ( ! Arms -> GetEnabled () )
		Arms -> Enable ();

	if ( ! Belts -> GetEnabled () )
		Belts -> Enable ();

	BallSensor -> SetMeasurementUnits ( IRDistanceSensor :: kMeters );

};

void BallPickupBehavior :: Update ()
{

	switch ( State )
	{

	case STATE_START:

		Drive -> SetTranslation ( 0, DRIVE_BACK_SPEED );
		Drive -> SetRotation ( 0 );
		Drive -> PushTransform ();

		Belts -> SetSpeed ( SPEED_COLLECT );
		Belts -> PushSpeeds ();

		State = STATE_DRIVE_BACK;

		break;

	case STATE_DRIVE_BACK:

		if ( BallSensor -> Get () <= BALL_THRESHOLD )
		{

			State = STATE_CLAMPING;

		}

		break;

	case STATE_CLAMPING:

		Arms -> DrivePositions ( ARM_LEFT_CLOSE, ARM_RIGHT_CLOSE );

		if ( BallLimit -> Get () )
		{

			State = STATE_HAVE_BALL;

		}

		break;

	case STATE_HAVE_BALL:

		Belts -> SetSpeed ( 0 );
		Belts -> PushSpeeds ();

		Drive -> SetTranslation ( 0, 0 );
		Drive -> PushTransform ();

	default:
		break;

	}

};

bool BallPickupBehavior :: IsDone ()
{

	return State == STATE_HAVE_BALL;

};

void BallPickupBehavior :: Stop ()
{

	Drive -> Disable ();
	Arms -> Disable ();
	Belts -> Disable ();

};

void BallPickupBehavior :: Restart ()
{

	State = STATE_START;

};
