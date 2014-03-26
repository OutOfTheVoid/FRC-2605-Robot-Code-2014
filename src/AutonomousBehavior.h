#ifndef SHS_2605_AUTONOMOUS_BEHAVIOR_H
#define SHS_2605_AUTONOMOUS_BEHAVIOR_H

#include "Behaviors/Behavior.h"

#include "SubSystems/MecanumDrive.h"
#include "SubSystems/ShooterBelts.h"
#include "SubSystems/CollectorArms.h"
#include "SubSystems/ShooterWinch.h"

class AutonomousBehavior : public Behavior
{
public:

	AutonomousBehavior ();
	~AutonomousBehavior ();

	void Start ();
	void Restart ();
	void Update ();
	void Stop ();

private:

	typedef enum
	{

		STATE_START = 0,
		STATE_FINISHED

	} AutonomousState;

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

};

#endif
