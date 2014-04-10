#include "AutonomousBehavior.h"

AutonomousBehavior :: AutonomousBehavior ( MecanumDrive * Drive, ShooterBelts * Belts, CollectorArms * Arms, ShooterWinch * Winch, Delegate <void> * OnShift, NumericStepper * Gear )
{

	this -> Drive = Drive;
	this -> Belts = Belts;
	this -> Arms = Arms;
	this -> Winch = Winch;

	this -> OnShift = OnShift;
	this -> Gear = Gear;

	Log = Logger :: GetInstance ();

};

AutonomousBehavior :: ~AutonomousBehavior ()  
{

	Stop ();

};

void AutonomousBehavior :: Start ()
{

	Log -> Log ( Logger :: LOG_DEBUG, "\n" ); 

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

	Arms -> DrivePositions ( POSITION_LEFT_OUT, 0 );
	Winch -> DriveAngle ( ANGLE_ARM_DEPLOY );

	Drive -> SetTranslation ( 0.0, - 0.8 );
	Drive -> SetRotation ( 0.0 );
	Drive -> PushTransform ();

	Log -> Log ( Logger :: LOG_DEBUG, "STATE CHANGED: STATE_START\n" );

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

			Log -> Log ( Logger :: LOG_DEBUG, "STATE CHANGED: STATE_ARML_DEPLOY\n" );

		}

		break;

	case STATE_ARML_DEPLOY:

		if ( ( Arms -> ArmPositionsWithin ( 0.08, POSITION_LEFT_OUT, Arms -> GetPositionRight () ) ) )
		{

			Arms -> DrivePositions ( POSITION_LEFT_OUT, POSITION_RIGHT_OUT );

			State = STATE_ARMR_DEPLOY;

			Log -> Log ( Logger :: LOG_DEBUG, "STATE CHANGED: STATE_ARMR_DEPLOY\n" );

		}

		break;

	case STATE_ARMR_DEPLOY:

		if ( Arms -> ArmPositionsWithin ( 0.08, POSITION_LEFT_OUT, POSITION_RIGHT_OUT ) )
		{

			Winch -> DriveAngle ( ANGLE_BALL_DROP );

			TimerStart = Timer :: GetPPCTimestamp ();
			State = STATE_BALL_DROP;

			Log -> Log ( Logger :: LOG_DEBUG, "STATE CHANGED: STATE_BALL_DROP\n" );

		}

		break;

	case STATE_BALL_DROP:

		if ( Timer :: GetPPCTimestamp () - TimerStart >= TIME_BALL_DROP ) 
		{

			State = STATE_BALL_PRE_SHOOT;

			Log -> Log ( Logger :: LOG_DEBUG, "STATE CHANGED: STATE_BALL_SHOOT\n" );

		}

		break;

	case STATE_BALL_PRE_SHOOT:

		Winch -> DriveAngle ( ANGLE_BALL_CLAMP );

		if ( Winch -> WithinAngle ( 0.13, ANGLE_BALL_CLAMP ) )  
		{

			Arms -> DrivePositions ( POSITION_LEFT_OUT, POSITION_RIGHT_OUT_FULL );

			State = STATE_BALL_SHOOT;

			Log -> Log ( Logger :: LOG_DEBUG, "STATE CHANGED: STATE_BALL_SHOOT\n" );  

		}

		break;

	case STATE_BALL_SHOOT:

		if ( Arms -> ArmPositionsWithin ( 0.08, POSITION_LEFT_OUT, POSITION_RIGHT_OUT_FULL ) )
		{

			Belts -> SetSpeed ( 2.0 );
			Belts -> PushSpeeds ();

			TimerStart = Timer :: GetPPCTimestamp ();
			State = STATE_FINISHED;

			Log -> Log ( Logger :: LOG_DEBUG, "STATE CHANGED: STATE_FINISHED\n" );

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

	Drive -> Disable ();
	Belts -> Disable ();
	Arms -> Disable ();
	Winch -> Disable ();

};
