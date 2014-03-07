#include "BehaviorController.h"

BehaviorController :: BehaviorController ()
{

	Behaviors = new Vector <BehaviorControllerList_t> ();

};

BehaviorController :: ~BehaviorController ()
{

	for ( uint32_t i = 0; i < Behaviors -> GetLength (); i ++ )
		if ( ( * Behaviors ) [ i ].Active )
			( * Behaviors ) [ i ].Item -> Stop ();


};

void BehaviorController :: AddBehavior ( const char * Name, Behavior * NewBehavior )
{

	if ( Name == NULL || NewBehavior == NULL )
		return;

	if ( GetBehaviorIndex ( Name ) != -1 )
		return;

	BehaviorControllerList_t BehaviorListElement;

	BehaviorListElement.Name = Name;
	BehaviorListElement.Item = NewBehavior;
	BehaviorListElement.Active = false;

	Behaviors -> Push ( BehaviorListElement );

};

void BehaviorController :: RemoveBehavior ( const char * Name )
{

	int32_t Index = GetBehaviorIndex ( Name );

	if ( Index == -1 )
		return;

	if ( ( * Behaviors ) [ Index ].Active )
		( * Behaviors ) [ Index ].Item -> Stop ();

	Behaviors -> Remove ( Index, 1 );

};

void BehaviorController :: StartBehavior ( const char * Name )
{

	int32_t Index = GetBehaviorIndex ( Name );

	if ( Index != -1 )
	{

		if ( ! ( * Behaviors ) [ Index ].Active )
		{
			
			( * Behaviors ) [ Index ].Active = true;
			( * Behaviors ) [ Index ].Item -> Start ();

		}
		else
			( * Behaviors ) [ Index ].Item -> Restart ();

	}

};

void BehaviorController :: StopBehavior ( const char * Name )
{

	int32_t Index = GetBehaviorIndex ( Name );

	if ( Index != -1 )
	{

		if ( ( * Behaviors ) [ Index ].Active )
		{
			
			( * Behaviors ) [ Index ].Active = false;
			( * Behaviors ) [ Index ].Item -> Stop ();

		}

	}

};

bool BehaviorController :: GetBehaviorActive ( const char * Name )
{

	int32_t Index = GetBehaviorIndex ( Name );

	if ( Index != -1 )
		return ( * Behaviors ) [ Index ].Active;

	return false;

};

void BehaviorController :: Update ()
{

	for ( uint32_t i = 0; i < Behaviors -> GetLength (); i ++ )
		if ( ( * Behaviors ) [ i ].Active )
			( * Behaviors ) [ i ].Item -> Update ();

};

int32_t BehaviorController :: GetBehaviorIndex ( const char * Name )
{

	for ( uint32_t i = 0; i < Behaviors -> GetLength (); i ++ )
		if ( strcmp ( Name, ( * Behaviors ) [ i ].Name ) == 0 )
			return i;

	return -1;

};
