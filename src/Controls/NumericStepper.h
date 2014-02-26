#ifndef SHS_2605_NUMERIC_STEPPER_H
#define SHS_2605_NUMERIC_STEPPER_H

#include "WPILib.h"
#include "src/Util/Delegate.h"

class NumericStepper
{
public:

	NumericStepper ( GenericHID * Device, uint32_t UpButton, uint32_t DownButton );
	~NumericStepper ();

	void SetRange ( int32_t Min, int32_t, Max );
	void SetStep ( int32_t Step );

	int32_t Get ();
	void Set ( int32_t Value );

	void Update ();

private:

	Delegate <void> * OnChange;

	GenericHID * Device;

	uint32_t UpButton;
	uint32_t DownButton;

	int32_t Value;

	int32_t Max;
	int32_t Min;
	
	int32_t Step;

	bool UpButtonState;
	bool DownButtonState;

};

#endif