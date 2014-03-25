#ifndef SHS_2605_TELEOP_DRIVE_BEHAVIOR_H
#define SHS_2605_TELEOP_DRIVE_BEHAVIOR_H

#include "Behaviors/Behavior.h"

#include "SubSystems/MecanumDrive.h"
#include "SubSystems/ShooterBelts.h"
#include "SubSystems/Col.h"

#include "Filters/ExponentialFilter.h"
#include "Filters/DeadbandFilter.h"

#include "Util/Delegate.h"

#include "Controls/NumericStepper.h"

#include "WPILib.h"

#define DRIVE_RESPONSE_CURVE 2.0

/*
* TELEOP DRIVE BEHAVIOR:
*
* Defacto Teleoperated control behavior. Assumes at start that the robot's systems it uses are disabled, and on start enables them, giving appropriate control with every update.
*
* This behavior has flags for:
* - doing the automated pickup behavior ( bool DoPickup () )
* - doing the emergencey arms behavior ( bool DoEmergenceyArms () )
*
* INPUT: Strafe Joystick, Rotate Joystick, Cancel Joystick
* OUTPUT: Drive, Belts, Arms
*/

class TeleopDriveBehavior : public Behavior
{
public:

	TeleopDriveBehavior ( MecanumDrive * DriveSystem, ShooterBelts * Belts, CollectorArms * Arms, Joystick * Strafe, Joystick * Rotate, Joystick * CancelStick, NumericStepper * GearStepper, Delegate <void> * OnShiftDelegate );
	~TeleopDriveBehavior ();

	void Start ();
	void Update ();
	void Stop ();
	void Restart ();

	bool DoPickup ();
	bool DoEmergenceyArms ();

private:

	void ControlDrive ();
	void ControlBelts ();
	void ControlArms ();

	// Drive

	MecanumDrive * Drive;

	Joystick * StrafeStick;
	Joystick * RotateStick;
	Joystick * CancelStick;

	ExponentialFilter * DriveVelocityRamp;
	DeadbandFilter * JoystickDeadband;

	Delegate <void> * OnShift;
	NumericStepper * Gear;

	// Belts

	ShooterBelts * Belts;

	// Arms

	CollectorArms * Arms;

	// Flags

	bool Pickup;
	bool EmergenceyArms;

};

#endif
