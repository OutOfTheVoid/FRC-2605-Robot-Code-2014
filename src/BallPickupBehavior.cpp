#include "BallPickupBehavior.h"

BallPickupBehavior :: BallPickupBehavior ( ShooterBelts * Belts, CollectorArms * Arms, MecanumDrive * Drive, NumericStepper * GearStepper, Delegate <void> * OnShiftDelegate  )
{

	this -> Belts = Belts;
	this -> Arms = Arms;
	this -> Drive = Drive;

	this -> Gear = GearStepper;
	this -> OnShift = OnShiftDelegate;

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

};

void BallPickupBehavior :: Update ()
{

	switch ( State )
	{

	case STATE_START:

		Belts -> SetSpeed ( BELT_COLLECTION_SPEED );

		if ( ! Arms -> ArmsCalibrated () )
		{

			State = STATE_CALIBRATE_ARMS;
			TimerStart = Timer :: GetPPCTimestamp ();

		}
		else
		{

			State = STATE_MOVE_ARMS_OUT;
			TimerStart = Timer :: GetPPCTimestamp ();

		}

	case STATE_CALIBRATE_ARMS:

		if ( Arms -> DriveToLimitsAndCalibrate () || ( ( Timer :: GetPPCTimestamp () - TimerStart ) > MAX_POSITION_WAIT ) )
		{

			State = STATE_DRIVE_TO_BALL;
			TimerStart = Timer :: GetPPCTimestamp ();

		}
		else
			break;

	case STATE_MOVE_ARMS_OUT:

		if ( Arms -> DriveAngle ( ARM_OUT_POSTION ) || ( ( Timer :: GetPPCTimestamp () - TimerStart ) > MAX_POSITION_WAIT ) )
		{

			State = STATE_DRIVE_TO_BALL;

		}
		else
			break;

	case STATE_DRIVE_TO_BALL:

		Drive -> SetTranslation ( 0, - 0.5 );
		Drive -> PushTransform ();



		break;

	default:
		break;

	}

};

void BallPickupBehavior :: Stop ()
{

	Drive -> Disable ();


};

void BallPickupBehavior :: Restart ()
{

	State = STATE_START;

};
