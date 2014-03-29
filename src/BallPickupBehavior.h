#ifndef SHS_2605_BALL_PICKUP_BEHAVIOR_H
#define SHS_2605_BALL_PICKUP_BEHAVIOR_H

#include "WPILib.h"

#include "Behaviors/Behavior.h"

#include "Util/Delegate.h"

#include "Controls/NumericStepper.h"

#include "SubSystems/MecanumDrive.h"
#include "SubSystems/ShooterBelts.h"
#include "SubSystems/CollectorArms.h"
#include "SubSystems/ShooterWinch.h"

#include "Sensors/IRDistanceSensor.h"

#include "Logging/Logger.h"

#define ANGLE_BALL_PICKUP 0.18
#define ANGLE_BALL_CLAMP 0.0

#define POSITION_LEFT_PICKUP 0.90
#define POSITION_RIGHT_PICKUP 1.10

#define BALL_THRESHOLD 0.25

#define POSITION_LEFT_CLAMP 0.60
#define POSITION_RIGHT_CLAMP 0.92

class BallPickupBehavior : public Behavior
{
public:

	BallPickupBehavior ( MecanumDrive * Drive, CollectorArms * Arms, ShooterWinch * Winch, ShooterBelts * Belts, NumericStepper * Gear, Delegate <void> * OnShift, IRDistanceSensor * BallSensor, DigitalInput * BallLimit );
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
		STATE_DRIVE_BACK,
		STATE_PICKUP,
		STATE_FINISHED

	} BallPickupState;

	BallPickupState State;

	bool Done;

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

	// Ball sensors

	IRDistanceSensor * BallSensor;
	DigitalInput * BallLimit;

	// Logging

	Logger * Log;

};

#endif
