#ifndef SHS_2605_IR_DISTANCE_CONFIG
#define SHS_2605_IR_DISTANCE_CONFIG

#include "src/Config/ConfigSection.h"
#include "IRDistanceSensor.h"

class IRDistanceConfig : public ConfigSection
{
public:

	IRDistanceConfig ( IRDistanceSensor * Sensor );
	~IRDistanceConfig ();

	void LoadSensorCalibration ();
	void SetSensorCalibration ();

private:

	IRDistanceSensor * Sensor;

};

#endif
