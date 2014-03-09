#include "BallPickupBehavior.h"

BallPickupBehavior :: BallPickupBehavior ( ShooterBelts * Belts, CollectorArms * Arms, MecanumDrive * Drive, NumericStepper * GearStepper, Delegate <void> * OnShiftDelegate, IRDistanceSensor * BallSensor, DigitalInput * BallLimit )
{

	this -> Belts = Belts;
	this -> Arms = Arms;
	this -> Drive = Drive;

	this -> Gear = GearStepper;
	this -> OnShift = OnShiftDelegate;

	this -> BallSensor = BallSensor;
	this -> BallLimit = BallLimit;

};

BallPickupBehavior :: ~BallPickupBehavior ()
{



};

void BallPickupBehavior :: Start ()
{

	State = STATE_START;

	Gear -> Set ( 2 );
	OnShift -> Call ();

	if ( ! Drive -> GetEnabled () )
		Drive -> Enable ();

	if ( ! Arms -> GetEnabled () )
		Arms -> Enable ();

	if ( ! Belts -> GetEnabled () )
		Belts -> Enable ();

	BallSensor -> SetMeasurementUnits ( IRDistanceSensor :: kMeters );

};

void BallPickupBehavior :: Update ()
{

	switch ( State )
	{

	case STATE_START:
	default:
		break;

	}

};

void BallPickupBehavior :: Stop ()
{

	Drive -> Disable ();
	Arms -> Disable ();
	Belts -> Disable ();

};

void BallPickupBehavior :: Restart ()
{

	State = STATE_START;

};
