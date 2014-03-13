#ifndef SHS_2605_IRDISTANCE_SENSOR
#define SHS_2605_IRDISTANCE_SENSOR

#include "WPILib.h"

#include "AnalogSensor.h"

#include "src/Math/SHSMath.h"

#include "src/Filters/MapFilter.h"

class IRDistanceSensor : AnalogSensor
{
public:

	typedef enum
	{

		kMeters,
		kFeet,
		kInches,
		kCentimeters,

	} MeasurementUnit;

	IRDistanceSensor ( AnalogChannel * Channel );
	~IRDistanceSensor ();

	void SetMeasurementUnits ( MeasurementUnit Unit );

	void SetInputVoltageRange ( range_t In );
	void SetOutputRange ( range_t Out );

	void CalibLowPoint ( double IntendedDist );
	void CalibHighPoint ( double IntendedDist );

	void SetUseAverage ( bool UseAverage );

	range_t GetOutputRange ( range_t Out );

	double Get ();

private:

	MeasurementUnit Unit;

	AnalogChannel * Channel;

	bool UseAverage;

	double OffLow;
	double InLow;

	MapFilter * InAttenuator;
	MapFilter * OutAttenuator;

};

#endif
