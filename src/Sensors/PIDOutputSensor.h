#ifndef SHS_2605_PID_OUTPUT_SENSOR
#define SHS_2605_PID_OUTPUT_SENSOR

#include "WPILib.h"

class PIDOutputSensor : public PIDOutput
{
public:

	PIDOutputSensor ();
	virtual ~PIDOutputSensor ();

	float GetOutput ();
	void PIDWrite ( float Value );

private:

	float Value;
	SEM_ID AccessSemaphore;

};

#endif
