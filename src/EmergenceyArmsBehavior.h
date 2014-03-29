#ifndef SHS_2605_EMERGENCEY_ARMS_BEHAVIOR_H
#define SHS_2605_EMERGENCEY_ARMS_BEHAVIOR_H

#include "Behaviors/Behavior.h"

#include "SubSystems/CollectorArms.h"

/*
* EMERGENCEY ARMS BEHAVIOR:
*
* Emergencey behavior to be invoked when the arms must be brought in quickly to avoid damage to the robot. This behavior assumes that the arms are enabled, and that the shooting mechanism is clear of any balls.
*
* This behavior has flags for:
* - When the arms have finished moving to the safetey position
*
* INPUT: None
* OUTPUT: Arms
*/

#define ARM_LEFT_IN 0.0
#define ARM_RIGHT_IN 0.3

class EmergenceyArmsBehavior : public Behavior
{
public:

	EmergenceyArmsBehavior ( CollectorArms * Arms );
	~EmergenceyArmsBehavior ();

	void Start ();
	void Update ();
	void Stop ();
	void Restart ();

	bool IsMoveDone ();

private:

	typedef enum
	{

		STATE_START,
		STATE_LEFT_IN,
		STATE_RIGHT_IN,
		STATE_FINISHED

	} EmergenceyArmState;

	CollectorArms * Arms;

	EmergenceyArmState State;

	bool MoveDone;

	double ArmRightStart;

};

#endif
