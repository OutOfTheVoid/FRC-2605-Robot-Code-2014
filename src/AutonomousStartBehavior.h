#ifndef SHS_2605_AUTONOMOUS_START_BEHAVIOR
#define SHS_2605_AUTONOMOUS_START_BEHAVIOR

#include "SubSystems/CollectorArms.h"
#include "SubSystems/ShooterWinch.h"
#include "SubSystems/MecanumDrive.h"
#include "SubSystems/ShooterBelts.h"

#include "Behaviors/Behavior.h"

#include "Logging/Logger.h"

#define A_ARM_LEFT_OUT - 0.67014
#define A_ARM_RIGHT_OUT 0.23472

#define WINCH_ARMOUT - 0.24

#define WINCH_BALLDROP - 0.94

#define WINCH_BALL_ANGLE - 0.16

class AutonomousStartBehavior : public Behavior
{
public:

	AutonomousStartBehavior ( CollectorArms * Arms, ShooterWinch * Winch, MecanumDrive * Drive, ShooterBelts * Belts );
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
		STATE_BALL_CLAMP,
		STATE_WINCH_BALLDROP,
		STATE_DRIVE_FORWARD,
		STATE_SHOOT,
		STATE_END,

	} BehaviorState;

	BehaviorState State;
	double TimerStart;

	CollectorArms * Arms;
	ShooterWinch * Winch;
	MecanumDrive * Drive;
	ShooterBelts * Belts;

	Logger * Log;

};

#endif
