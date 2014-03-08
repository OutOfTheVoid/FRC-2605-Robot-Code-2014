#include "IRDistanceSensor.h"

IRDistanceSensor :: IRDistanceSensor ( AnalogChannel * Channel )
{

	this -> Channel = Channel;

	Unit = IRDistanceSensor :: kMeters;
	UseAverage = true;

};

IRDistanceSensor :: ~IRDistanceSensor ()
{



};

void IRDistanceSensor :: SetMeasurementUnits ( MeasurementUnit Units )
{

	Unit = Units;

};

void IRDistanceSensor :: SetInputVoltageRange ( range_t In )
{

	Attenuator -> SetRangeIn ( In );

};

void IRDistanceSensor :: SetOutputRange ( range_t Out )
{

	Attenuator -> SetRangeOut ( Out );

};

range_t IRDistanceSensor :: GetOutputRange ( range_t Out )
{

	return Attenuator -> GetRangeOut ();

};

void IRDistanceSensor :: CalibLowPoint ()
{

	range_t CalRange = Attenuator -> GetRangeIn ();
	CalRange.Low = ( UseAverage ? Channel -> GetAverageVoltage () : Channel -> GetVoltage () );
	Attenuator -> SetRangeIn ( CalRange );

};

void IRDistanceSensor :: CalibHighPoint ()
{

	range_t CalRange = Attenuator -> GetRangeIn ();
	CalRange.Low = ( UseAverage ? Channel -> GetAverageVoltage () : Channel -> GetVoltage () );
	Attenuator -> SetRangeIn ( CalRange );

};

double IRDistanceSensor :: Get ()
{

	switch ( Unit )
	{

		case kMeters:
		default:

			return Attenuator -> Compute ( UseAverage ? Channel -> GetAverageVoltage () : Channel -> GetVoltage () );

		case kCentimeters:

			return 100 * Attenuator -> Compute ( UseAverage ? Channel -> GetAverageVoltage () : Channel -> GetVoltage () );

		case kFeet:

			return 3.28084 * Attenuator -> Compute ( UseAverage ? Channel -> GetAverageVoltage () : Channel -> GetVoltage () );

		case kInches:

			return 39.3701 * Attenuator -> Compute ( UseAverage ? Channel -> GetAverageVoltage () : Channel -> GetVoltage () );

	}

	return 0;

};
