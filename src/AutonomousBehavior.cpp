#include "AutonomousBehavior.h"

AutonomousBehavior :: AutonomousBehavior ( MecanumDrive * Drive, ShooterBelts * Belts, CollectorArms * Arms, ShooterWinch * Winch, Delegate <void> * OnShift, NumericStepper * Gear )
{

	this -> Drive = Drive;
	this -> Belts = Belts;
	this -> Arms = Arms;
	this -> Winch = Winch;

	this -> OnShift = OnShift;
	this -> Gear = Gear;

};

AutonomousBehavior :: ~AutonomousBehavior ()
{

	Stop ();

};

void AutonomousBehavior :: Start ()
{

	State = STATE_START;

	Gear -> Set ( 2 );
	OnShift -> Call ();

	Winch -> SetZero ();
	Arms -> SetZeros ();

	if ( ! Drive -> GetEnabled () )
		Drive -> Enable ();

	if ( ! Belts -> GetEnabled () )
		Belts -> Enable ();

	if ( ! Arms -> GetEnabled () )
		Arms -> Enable ();

	if ( ! Winch -> GetEnabled () )
		Winch -> Enable ();

	TimerStart = Timer :: GetPPCTimestamp ();

	Drive -> SetTranslation ( 0.0, - 0.5 );
	Drive -> SetRotation ( 0.0 );
	Drive -> PushTransform ();

	Arms -> DrivePositions ( POSITION_LEFT_OUT, 0 );
	Winch -> DriveAngle ( ANGLE_ARM_DEPLOY );

};

void AutonomousBehavior :: Restart ()
{

 	Start ();

};

void AutonomousBehavior :: Update ()
{

	switch ( State )
	{

	case STATE_START:

		if ( Timer :: GetPPCTimestamp () - TimerStart >= TIME_DRIVE_FORWARD )
		{

			Drive -> SetTranslation ( 0.0, 0.0 );
			Drive -> PushTransform ();

			State = STATE_ARML_DEPLOY;

		}

		break;

	case STATE_ARML_DEPLOY:

		if ( Winch -> WithinAngle ( 0.03, ANGLE_ARM_DEPLOY ) && Arms -> ArmPositionsWithin ( 0.02, POSITION_LEFT_OUT, 0 ) )
		{

			Arms -> DrivePositions ( POSITION_LEFT_OUT, POSITION_RIGHT_OUT );

			State = STATE_ARMR_DEPLOY;

		}

		break;

	case STATE_ARMR_DEPLOY:

		if ( Arms -> ArmPositionsWithin ( 0.02, POSITION_LEFT_OUT, POSITION_RIGHT_OUT ) )
		{

			Winch -> DriveAngle ( ANGLE_BALL_DROP );

			TimerStart = Timer :: GetPPCTimestamp ();
			State = STATE_BALL_DROP;

		}

		break;

	case STATE_BALL_DROP:

		if ( Timer :: GetPPCTimestamp () - TimerStart >= TIME_BALL_DROP )
		{

			Winch -> DriveAngle ( ANGLE_BALL_CLAMP );

			State = STATE_BALL_SHOOT;

		}

		break;

	case STATE_BALL_SHOOT:

		if ( Winch -> WithinAngle ( 0.03, ANGLE_ARM_DEPLOY ) )
		{

			Belts -> SetSpeed ( 1.0 );
			Belts -> PushSpeeds ();

			TimerStart = Timer :: GetPPCTimestamp ();
			State = STATE_FINISHED;

		}

		break;

	case STATE_FINISHED:

		if ( Timer :: GetPPCTimestamp () - TimerStart >= TIME_BALL_SHOOT )
		{

			Belts -> SetSpeed ( 0.0 );
			Belts -> PushSpeeds ();

		}

		break;

	}

};

void AutonomousBehavior :: Stop ()
{



};
