#ifndef SHS_2605_NUMERIC_STEPPER_H
#define SHS_2605_NUMERIC_STEPPER_H

#include "WPILib.h"
#include "src/Util/Delegate.h"

class NumericStepper
{
public:



private:

	Delegate <void> * OnChange;

	GenericHID * Device;

	uint32_t UpButton;
	uint32_t DownButton;

};

#endif