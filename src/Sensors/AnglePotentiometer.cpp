#include "AnglePotentiometer.h"

AnglePotentiometer :: AnglePotentiometer ( AnalogChannel * Channel, range_t OutputRange, bool Average )
{

	this -> Channel = Channel;
	Attenuator = new MapFilter ( range_t ( 0, 5 ), OutputRange );

	UseAverage = Average;

};

AnglePotentiometer :: AnglePotentiometer ( AnalogChannel * Channel, bool Average )
{

	this -> Channel = Channel;
	Attenuator = new MapFilter ( range_t ( 0, 5 ), range_t ( 0, 1 ) );

	UseAverage = Average;

};

AnglePotentiometer :: ~AnglePotentiometer ()
{

	delete Attenuator;

};

void AnglePotentiometer :: SetInputVoltageRange ( range_t In )
{

	Attenuator -> SetRangeIn ( In );

};

void AnglePotentiometer :: SetOutputRange ( range_t Out )
{

	Attenuator -> SetRangeOut ( Out );

};

range_t AnglePotentiometer :: GetOutputRange ( range_t Out )
{

	return Attenuator -> GetRangeOut ();

};

void AnglePotentiometer :: CalibLowPoint ()
{

	range_t CalRange = Attenuator -> GetRangeIn ();
	CalRange.Low = ( UseAverage ? Channel -> GetAverageVoltage () : Channel -> GetVoltage () );
	Attenuator -> SetRangeIn ( CalRange );

};

void AnglePotentiometer :: CalibHighPoint ()
{

	range_t CalRange = Attenuator -> GetRangeIn ();
	CalRange.Low = ( UseAverage ? Channel -> GetAverageVoltage () : Channel -> GetVoltage () );
	Attenuator -> SetRangeIn ( CalRange );

};

double AnglePotentiometer :: Get ()
{

	return Attenuator -> Compute ( UseAverage ? Channel -> GetAverageVoltage () : Channel -> GetVoltage () );

};
