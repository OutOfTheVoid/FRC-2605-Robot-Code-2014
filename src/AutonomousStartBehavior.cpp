#include "AutonomousStartBehavior.h"

AutonomousStartBehavior :: AutonomousStartBehavior ( CollectorArms * Arms, ShooterWinch * Winch, MecanumDrive * Drive, ShooterBelts * Belts )
{

	this -> Arms = Arms;
	this -> Winch = Winch;
	this -> Drive = Drive;
	this -> Belts = Belts;

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

	if ( ! Drive -> GetEnabled () )
		Drive -> Enable ();

	if ( ! Belts -> GetEnabled () )
		Belts -> Enable ();

	Arms -> SetZeros ();
	Winch -> SetZero ();

	Log -> Log ( Logger :: LOG_DEBUG, "Autonomous Start Behavior: State change to STATE_START!!!\n" );

	this -> State = STATE_START;

};

void AutonomousStartBehavior :: Stop ()
{

	Arms -> Disable ();
	Winch -> Disable ();
	Drive -> Disable ();
	Belts -> Disable ();

};

void AutonomousStartBehavior :: Restart () 
{

};

void AutonomousStartBehavior :: Update ()
{

	switch ( State )
	{

	case STATE_START:

		TimerStart = Timer :: GetPPCTimestamp ();
		
		State = STATE_DRIVE_FORWARD;

		/*State = STATE_WINCH_BALLDROP;

		break;

	case STATE_WINCH_BALLDROP:

		Winch -> DriveAngle ( WINCH_BALLDROP );

		if ( Winch -> WithinAngle ( 0.03, WINCH_BALLDROP ) )
		{

			State = STATE_BALL_CLAMP;
			TimerStart = Timer :: GetPPCTimestamp ();

		}

		break;

	case STATE_BALL_CLAMP:

		Winch -> DriveAngle ( WINCH_BALL_ANGLE );
		State = STATE_DRIVE_FORWARD;

		TimerStart = Timer :: GetPPCTimestamp ();

		break;*/

	case STATE_DRIVE_FORWARD:

		Drive -> SetTranslation ( 0.0, - 1.0 );
		Drive -> PushTransform ();
		Winch -> DrivePWM ( 0.2 );

		if ( Timer :: GetPPCTimestamp () - TimerStart > 1.6 )
		{

			State = STATE_END;

			TimerStart = Timer :: GetPPCTimestamp ();

		}

		break;

	case STATE_SHOOT:

		Drive -> SetTranslation ( 0.0, 0.0 );
		Drive -> PushTransform ();

		Belts -> SetSpeed ( 1.0 );
		Belts -> PushSpeeds ();

		if ( Timer :: GetPPCTimestamp () - TimerStart > 2.0 )
		{

			State = STATE_END;

		}

		break;

	case STATE_END:

		Belts -> SetSpeed ( 0.0 );
		Belts -> PushSpeeds ();

		break;

	default:
		break;

	}

};
