#include "EmergenceyArmsBehavior.h"

EmergenceyArmsBehavior :: EmergenceyArmsBehavior ( CollectorArms * Arms )
{

	this -> Arms = Arms;

};

EmergenceyArmsBehavior :: ~EmergenceyArmsBehavior ()
{

 	Stop ();

};

void EmergenceyArmsBehavior :: Start ()
{

	State = STATE_START;

	MoveDone = false;

};

void EmergenceyArmsBehavior :: Restart ()
{

	Arms -> Stop ();

	State = STATE_START;

};

void EmergenceyArmsBehavior :: Update ()
{

	switch ( State )
	{

	case STATE_START:

		ArmRightStart = Arms -> GetPositionRight ();

		State = STATE_LEFT_IN;

		break;

	case STATE_LEFT_IN:

		Arms -> DrivePositions ( ARM_LEFT_IN, ArmRightStart );

		if ( Arms -> ArmPositionsWithin ( 0.2, ARM_LEFT_IN, ArmRightStart ) )
			State = STATE_RIGHT_IN;

		break;

	case STATE_RIGHT_IN:

		Arms -> DrivePositions ( ARM_LEFT_IN, ARM_RIGHT_IN );

		if ( Arms -> ArmPositionsWithin ( 0.2, ARM_LEFT_IN, ARM_RIGHT_IN ) )
			State = STATE_FINISHED;

		break;

	case STATE_FINISHED:

		Arms -> Stop ();

		MoveDone = true;

		break;

	}

};

void EmergenceyArmsBehavior :: Stop ()
{

	Arms -> Stop ();

};

bool EmergenceyArmsBehavior :: IsMoveDone ()
{

	return MoveDone;

};
