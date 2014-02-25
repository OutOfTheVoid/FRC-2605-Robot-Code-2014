#ifndef SHS_2605_DIGITALSENSOR_H
#define SHS_2605_DIGITALSENSOR_H

class DigitalSensor
{
public:

	virtual ~DigitalSensor () {};

	virtual bool Get () { return false; };

};

#endif
