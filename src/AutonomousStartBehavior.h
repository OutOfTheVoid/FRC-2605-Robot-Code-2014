#ifndef SHS_2605_AUTONOMOUS_START_BEHAVIOR
#define SHS_2605_AUTONOMOUS_START_BEHAVIOR

#include "SubSystems/CollectorArms.h"
#include "SubSystems/ShooterWinch.h"

#include "Behaviors/Behavior.h"

#include "Logging/Logger.h"

#define ARM_LEFT_OUT - 0.67014
#define ARM_RIGHT_OUT 0.23472

#define WINCH_ARMOUT - 0.07

#define WINCH_BALLDROP - 0.20

#define WINCH_BALL_ANGLE - 0.08

class AutonomousStartBehavior : public Behavior
{
public:

	AutonomousStartBehavior ( CollectorArms * Arms, ShooterWinch * Winch );
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
		STATE_WINCH_ARMDEPLOY,
		STATE_BALL_DEPLOY,
		STATE_BALL_CLAMP,
		STATE_END,

	} BehaviorState;

	BehaviorState State;
	double TimerStart;

	CollectorArms * Arms;
	ShooterWinch * Winch;

	Logger * Log;

};

#endif
