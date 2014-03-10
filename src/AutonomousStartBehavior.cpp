#include "AutonomousStartBehavior.h"

AutonomousStartBehavior :: AutonomousStartBehavior ( CollectorArms * Arms )
{

	this -> Arms = Arms;

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

	Arms -> SetZeros ();

	Log -> Log ( Logger :: LOG_DEBUG, "Autonomous Start Behavior: State change to STATE_START!!!\n" );

	this -> State = STATE_START;

};

void AutonomousStartBehavior :: Stop ()
{

	Arms -> Disable ();

};

void AutonomousStartBehavior :: Restart ()
{

	Arms -> SetZeros ();

};

void AutonomousStartBehavior :: Update ()
{

	switch ( State )
	{

	case STATE_START:

		if ( Arms -> DrivePositions ( ARM_LEFT_OUT, 0 ) )
		{

			Log -> Log ( Logger :: LOG_DEBUG, "Autonomous Start Behavior: State change to STATE_ARML_OUT!!!\n" );

			State = STATE_ARML_OUT;

		}

	case STATE_ARML_OUT:

		Arms -> Stop ();
		State = STATE_END;

	case STATE_END:
	default:
		break;

	}

};
