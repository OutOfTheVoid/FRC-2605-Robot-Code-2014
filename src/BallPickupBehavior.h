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

#define BELT_COLLECTION_SPEED 0.2

#define ARM_OUT_POSTION 0.01
#define ARM_IN_POSITION 0.3

#define BALL_SENSOR_THRESHOLD 0.20

#define MAX_POSITION_WAIT 1.5

class BallPickupBehavior : public Behavior
{
public:

	BallPickupBehavior ( ShooterBelts * Belts, CollectorArms * Arms, MecanumDrive * Drive, NumericStepper * GearStepper, Delegate <void> * OnShiftDelegate, IRDistanceSensor * BallSensor, DigitalInput * BallLimit );
	~BallPickupBehavior ();

	void Start ();
	void Update ();
	void Stop ();
	void Restart ();

private:

	typedef enum BehaviorState
	{

		STATE_START = 0,
		STATE_MOVE_ARMS_OUT,
		STATE_DRIVE_TO_BALL,
		STATE_CLAMP_BALL,

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
