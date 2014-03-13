#ifndef SHS_2605_BALL_PICKUP_BEHAVIOR_H
#define SHS_2605_BALL_PICKUP_BEHAVIOR_H

#include "Behaviors/Behavior.h"

#include "WPILib.h"

#include "src/SubSystems/ShooterBelts.h"
#include "src/SubSystems/CollectorArms.h"
#include "src/SubSystems/MecanumDrive.h"

#include "src/Sensors/IRDistanceSensor.h"

#include "src/Controls/NumericStepper.h"

#include "src/Util/Delegate.h"

#define DRIVE_BACK_SPEED 0.5

#define SPEED_COLLECT 0.2

#define BALL_THRESHOLD 0.3

#define ARM_LEFT_CLOSE - 0.661111
#define ARM_RIGHT_CLOSE 0.883333

class BallPickupBehavior : public Behavior
{
public:

	BallPickupBehavior ( ShooterBelts * Belts, CollectorArms * Arms, MecanumDrive * Drive, NumericStepper * GearStepper, Delegate <void> * OnShiftDelegate, IRDistanceSensor * BallSensor, DigitalInput * BallLimit );
	~BallPickupBehavior ();

	void Start ();
	void Update ();
	void Stop ();
	void Restart ();

	bool Active ();

	bool IsDone ();

private:

	typedef enum BehaviorState
	{

		STATE_START = 0,
		STATE_DRIVE_BACK,
		STATE_CLAMPING,
		STATE_HAVE_BALL,
		STATE_FINISHED

	} BehaviorState;

	BehaviorState State;
	double TimerStart;

	ShooterBelts * Belts;
	CollectorArms * Arms;
	MecanumDrive * Drive;

	Delegate <void> * OnShift;
	NumericStepper * Gear;

	IRDistanceSensor * BallSensor;
	DigitalInput * BallLimit;

};

#endif
