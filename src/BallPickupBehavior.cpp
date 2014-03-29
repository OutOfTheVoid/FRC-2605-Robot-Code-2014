#include "BallPickupBehavior.h"

BallPickupBehavior :: BallPickupBehavior ( MecanumDrive * Drive, CollectorArms * Arms, ShooterWinch * Winch, ShooterBelts * Belts, NumericStepper * Gear, Delegate <void> * OnShift, IRDistanceSensor * BallSensor, DigitalInput * BallLimit )
{

	this -> Drive = Drive;
	this -> Arms = Arms;
	this -> Winch = Winch;
	this -> Belts = Belts;
	this -> Gear = Gear;
	this -> OnShift = OnShift;
	this -> BallSensor = BallSensor;
	this -> BallLimit = BallLimit;

	Log = Logger :: GetInstance ();

};

BallPickupBehavior :: ~BallPickupBehavior ()
{



};

void BallPickupBehavior :: Start ()
{

	Done = false;

	Gear -> Set ( 2 );
	OnShift -> Call ();

	Drive -> SetTranslation ( 0.0, 0.0 );
	Drive -> SetRotation ( 0.0 );
	Drive -> PushTransform ();

	Arms -> DrivePositions ( POSITION_LEFT_PICKUP, POSITION_RIGHT_PICKUP );
	Winch -> DriveAngle ( ANGLE_BALL_PICKUP );

	State = STATE_START;

};

void BallPickupBehavior :: Restart ()
{

	Start ();

};

// Set winch to ball pickup, swing arms out, drive backward till ball sensed then clamp while pulling with belts. Finally stop moving and raise the ball to the shooter limit switch.

void BallPickupBehavior :: Update () 
{

	switch ( State )
	{

		case STATE_START:

			if ( Arms -> ArmPositionsWithin ( 0.1, POSITION_LEFT_PICKUP, POSITION_RIGHT_PICKUP ) && Winch -> WithinAngle ( 0.13, ANGLE_BALL_PICKUP ) )
			{

				Drive -> SetTranslation ( 0.0, - 0.5 );
				Drive -> PushTransform ();

				Winch -> DriveAngle ( ANGLE_BALL_PICKUP );

				State = STATE_DRIVE_BACK;

			}

			break;

		case STATE_DRIVE_BACK:

			if ( BallSensor -> Get () <= BALL_THRESHOLD )
			{

				Arms -> DrivePositions ( POSITION_LEFT_CLAMP, POSITION_RIGHT_CLAMP );
				
				Belts -> SetSpeed ( 0.07 );
				Belts -> PushSpeeds ();

				State = STATE_PICKUP;

			}

			break;

		case STATE_PICKUP:

			if ( BallLimit -> Get () )
			{

				Belts -> SetSpeed ( 0.0 );
				Belts -> PushSpeeds ();

				Winch -> DriveAngle ( ANGLE_BALL_CLAMP );

				Drive -> SetTranslation ( 0.0, 0.0 );
				Drive -> SetRotation ( 0.0 );
				Drive -> PushTransform ();

				Arms -> DrivePositions ( POSITION_LEFT_PICKUP, POSITION_RIGHT_PICKUP );

				State = STATE_FINISHED;

			}

			break;

		case STATE_FINISHED:

			Done = true;

			break;

	}

};

void BallPickupBehavior :: Stop ()
{



};

bool BallPickupBehavior :: IsDone ()
{

	return Done;

};
