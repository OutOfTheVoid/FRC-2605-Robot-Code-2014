#ifndef SHS_2605_AUTONOMOUS_START_BEHAVIOR
#define SHS_2605_AUTONOMOUS_START_BEHAVIOR

#include "SubSystems/CollectorArms.h"

#include "Behaviors/Behavior.h"

#include "Logging/Logger.h"

#define ARM_LEFT_OUT 0.67014

class AutonomousStartBehavior : public Behavior
{
public:

	AutonomousStartBehavior ( CollectorArms * Arms );
	~AutonomousStartBehavior ();

	void Start ();
	void Stop ();
	void Restart ();
	void Update ();

private:

	typedef enum BehaviorState
	{

		STATE_START = 0,
		STATE_ARML_OUT,
		STATE_ARMR_OUT,
		STATE_END,

	} BehaviorState;

	BehaviorState State;
	double TimerStart;

	CollectorArms * Arms;

	Logger * Log;

};

#endif
