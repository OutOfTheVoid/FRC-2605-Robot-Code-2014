#include "BallPickupBehavior.h"

BallPickupBehavior :: BallPickupBehavior ()
{



};

BallPickupBehavior :: ~BallPickupBehavior ()
{



};

void BallPickupBehavior :: Start ()
{

	Done = false;

	State = STATE_START;

};

void BallPickupBehavior :: Restart ()
{

	Start ();

};

void BallPickupBehavior :: Update ()
{

	switch ( State )
	{

		case STATE_START:

			State = STATE_FINISHED;

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
