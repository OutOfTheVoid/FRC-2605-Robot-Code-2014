#ifndef SHS_2605_BALL_PICKUP_BEHAVIOR_H
#define SHS_2605_BALL_PICKUP_BEHAVIOR_H

#include "Behaviors/Behavior.h"

class BallPickupBehavior : public Behavior
{
public:

	BallPickupBehavior ();
	~BallPickupBehavior ();

	void Start ();
	void Restart ();
	void Update ();
	void Stop ();

	bool IsDone ();

private:

	typedef enum
	{

		STATE_START = 0,
		STATE_FINISHED

	} BallPickupState;

	BallPickupState State;

	bool Done;

};

#endif
