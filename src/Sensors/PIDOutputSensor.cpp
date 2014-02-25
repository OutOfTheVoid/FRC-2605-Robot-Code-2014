#include "PIDOutputSensor.h"

PIDOutputSensor :: PIDOutputSensor ()
{

	Value = 0;

	AccessSemaphore = semMCreate ( SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE );

};

PIDOutputSensor :: ~PIDOutputSensor ()
{

	semDelete ( AccessSemaphore );

};

float PIDOutputSensor :: GetOutput ()
{

	semTake ( AccessSemaphore, WAIT_FOREVER );

	double v = Value;
	
	semGive ( AccessSemaphore );

	return v;

};

void PIDOutputSensor :: PIDWrite ( float Value )
{

	semTake ( AccessSemaphore, WAIT_FOREVER );

	this -> Value = Value;

	semGive ( AccessSemaphore );

};
