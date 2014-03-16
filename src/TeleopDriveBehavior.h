#ifndef SHS_2605_TELEOP_DRIVE_BEHAVIOR_H
#define SHS_2605_TELEOP_DRIVE_BEHAVIOR_H

#include "Behaviors/Behavior.h"

#include "SubSystems/MecanumDrive.h"
#include "SubSystems/CollectorArms.h"
#include "SubSystems/ShooterBelts.h"
#include "SubSystems/ShooterWinch.h"

#include "Filters/ExponentialFilter.h"
#include "Filters/DeadbandFilter.h"

#include "Util/Delegate.h"

#include "Controls/NumericStepper.h"

#include "WPILib.h"

#define DRIVE_RESPONSE_CURVE 2.0

#define T_ARM_LEFT_OUT - 0.93611
#define T_ARM_RIGHT_OUT 1.16875

#define T_ARM_LEFT_EMERGENCY_IN - 0.53611
#define T_ARM_RIGHT_EMERGENCY_IN 0.66875

class TeleopDriveBehavior : public Behavior
{
public:

	TeleopDriveBehavior ( MecanumDrive * DriveSystem, Joystick * Strafe, Joystick * Rotate, NumericStepper * GearStepper, Delegate <void> * OnShiftDelegate, CollectorArms * ArmSystem, ShooterBelts * Belts, Joystick * CancelStick, ShooterWinch * Winch );
	~TeleopDriveBehavior ();

	void Start ();
	void Update ();
	void Stop ();
	void Restart ();

	bool DoPickup ();

private:

	void ControlDrive ();
	void ControlArms ();
	void ControlBelts ();

	// Drive

	MecanumDrive * Drive;

	Joystick * StrafeStick;
	Joystick * RotateStick;
	Joystick * CancelStick;

	ExponentialFilter * DriveVelocityRamp;
	DeadbandFilter * JoystickDeadband;

	Delegate <void> * OnShift;
	NumericStepper * Gear;

	// Arms

	CollectorArms * Arms;

	// Belts

	ShooterBelts * Belts;

	bool Pickup;

	// Winch

	ShooterWinch * Winch;

};

#endif
