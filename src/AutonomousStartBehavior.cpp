#include "AutonomousStartBehavior.h"

AutonomousStartBehavior :: AutonomousStartBehavior ( CollectorArms * Arms, ShooterWinch * Winch )
{

	this -> Arms = Arms;
	this -> Winch = Winch;

	Log = Logger :: GetInstance ();

};

AutonomousStartBehavior :: ~AutonomousStartBehavior ()
{



};

void AutonomousStartBehavior :: Start ()
{

	Log -> Log ( Logger :: LOG_DEBUG, "Autonomous Start Behavior: Started!!!\n" );

	if ( ! Arms -> GetEnabled () )
		Arms -> Enable ();

	if ( ! Winch -> GetEnabled () )
		Winch -> Enable ();

	Arms -> SetZeros ();
	Winch -> SetZero ();

	Log -> Log ( Logger :: LOG_DEBUG, "Autonomous Start Behavior: State change to STATE_START!!!\n" );

	this -> State = STATE_START;

};

void AutonomousStartBehavior :: Stop ()
{

	Arms -> Disable ();

};

void AutonomousStartBehavior :: Restart ()
{

};

void AutonomousStartBehavior :: Update ()
{

	switch ( State )
	{

	case STATE_START:

		Arms -> DrivePositions ( ARM_LEFT_OUT, 0 );

		if ( Arms -> ArmPositionsWithin ( 0.04, ARM_LEFT_OUT, 0 ) )
		{

			Log -> Log ( Logger :: LOG_DEBUG, "Autonomous Start Behavior: State change to STATE_ARML_OUT!!!\n" );

			State = STATE_ARML_OUT; 

		}

		break;

	case STATE_ARML_OUT:

		Winch -> DriveAngle ( WINCH_ARMOUT );

		if ( Winch -> WithinAngle ( 0.01, WINCH_ARMOUT ) )
		{

			State = STATE_WINCH_ARMDEPLOY;

			Log -> Log ( Logger :: LOG_DEBUG, "Autonomous Start Behavior: State change to STATE_WINCH_ARMDEPLOY!!!\n" );

		}

		break;

	case STATE_WINCH_ARMDEPLOY:

		Arms -> DrivePositions ( ARM_LEFT_OUT, ARM_RIGHT_OUT );

		if ( Arms -> ArmPositionsWithin ( 0.04, ARM_LEFT_OUT, ARM_RIGHT_OUT ) )
		{

			State = STATE_BALL_DEPLOY;

			Log -> Log ( Logger :: LOG_DEBUG, "Autonomous Start Behavior: State change to STATE_BALL_DEPLOY!!!\n" );

		}

	case STATE_BALL_DEPLOY:

		Winch -> DriveAngle ( WINCH_BALLDROP );

		if ( Winch -> WithinAngle ( 0.01, WINCH_BALLDROP ) )
		{

			State = STATE_BALL_CLAMP;

			Log -> Log ( Logger :: LOG_DEBUG, "Autonomous Start Behavior: State change to STATE_BALL_CLAMP!!!\n" );

		}

	case STATE_BALL_CLAMP:

		Winch -> DriveAngle ( WINCH_BALL_ANGLE );
		State = STATE_END;

		Log -> Log ( Logger :: LOG_DEBUG, "Autonomous Start Behavior: State change to STATE_END!!!\n" );

	case STATE_END:
	default:
		break;

	}

};
