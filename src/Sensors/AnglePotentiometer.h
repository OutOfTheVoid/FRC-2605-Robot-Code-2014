#ifndef SHS_2605_POTENTIOMETER_H
#define SHS_2605_POTENTIOMETER_H

#include "WPILib.h"

#include "AnalogSensor.h"
#include "src/Filters/MapFilter.h"

class AnglePotentiometer : virtual public AnalogSensor
{
public:

	AnglePotentiometer ( AnalogChannel * Channel, range_t OutputRange, bool Average = false );
	AnglePotentiometer ( AnalogChannel * Channel, bool Average = false );

	~AnglePotentiometer ();

	void SetInputVoltageRange ( range_t In );
	void SetOutputRange ( range_t Out );

	void CalibLowPoint ();
	void CalibHighPoint ();

	void SetUseAverage ( bool UseAverage );

	range_t GetOutputRange ( range_t Out );

	double Get ();

private:

	AnalogChannel * Channel;
	bool UseAverage;

	MapFilter * Attenuator;

};

#endif
