#ifndef SHS_2605_BEHAVIOR_CONTROLLER_H
#define SHS_2605_BEHAVIOR_CONTROLLER_H

#include "Behavior.h"
#include "src/Util/Vector.h"

#include <string.h>
#include <stdint.h>

typedef struct
{

	Behavior * Item;
	const char * Name;
	bool Active;

} BehaviorControllerList_t;

class BehaviorController
{
public:

	BehaviorController ();
	~BehaviorController ();

	void AddBehavior ( const char * Name, Behavior * NewBehavior );
	void RemoveBehavior ( const char * Name );

	void StartBehavior ( const char * Name );
	void StopBehavior ( const char * Name );

	bool GetBehaviorActive ( const char * Name );

	void Update ();

private:

	int32_t GetBehaviorIndex ( const char * Name );

	Vector <BehaviorControllerList_t> * Behaviors;

};

#endif
