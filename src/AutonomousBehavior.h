#ifndef SHS_2605_AUTONOMOUS_BEHAVIOR_H
#define SHS_2605_AUTONOMOUS_BEHAVIOR_H

#include "WPILib.h"

#include "Behaviors/Behavior.h"

#include "Util/Delegate.h"

#include "Controls/NumericStepper.h"

#include "SubSystems/MecanumDrive.h"
#include "SubSystems/ShooterBelts.h"
#include "SubSystems/CollectorArms.h"
#include "SubSystems/ShooterWinch.h"

#include "Logging/Logger.h"

#define TIME_DRIVE_FORWARD 3.0
#define TIME_BALL_DROP 1.6
#define TIME_BALL_SHOOT 1.0

#define POSITION_LEFT_OUT 0.67
#define POSITION_RIGHT_OUT 0.36
#define POSITION_RIGHT_OUT_FULL 0.88

#define ANGLE_ARM_DEPLOY 0.30
#define ANGLE_BALL_DROP 0.47
#define ANGLE_BALL_CLAMP 0.0

class AutonomousBehavior : public Behavior
{
public:

	AutonomousBehavior ( MecanumDrive * Drive, ShooterBelts * Belts, CollectorArms * Arms, ShooterWinch * Winch, Delegate <void> * OnShift, NumericStepper * Gear );
	~AutonomousBehavior ();

	void Start ();
	void Restart ();
	void Update ();
	void Stop ();

private:

	typedef enum
	{

		STATE_START = 0,
		STATE_ARML_DEPLOY = 1,
		STATE_ARMR_DEPLOY = 2,
		STATE_BALL_DROP = 3,
		STATE_BALL_SHOOT = 4,
		STATE_BALL_PRE_SHOOT = 5,
		STATE_FINISHED = 6

	} AutonomousState;

	// State machine...

	double TimerStart;

	AutonomousState State;

	// Drive

	MecanumDrive * Drive;

	Delegate <void> * OnShift;
	NumericStepper * Gear;

	// Belts

	ShooterBelts * Belts;

	// Arms

	CollectorArms * Arms;

	// Winch

	ShooterWinch * Winch;

	// Logging

	Logger * Log;

};

#endif
