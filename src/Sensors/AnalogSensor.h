#ifndef SHS_2605_SENSOR_H
#define SHS_2605_SENSOR_H

class AnalogSensor
{
public:

	virtual ~AnalogSensor () {};

	virtual double Get () { return 0; };

};

#endif
