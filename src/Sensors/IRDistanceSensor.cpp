#include "IRDistanceSensor.h"

IRDistanceSensor :: IRDistanceSensor ( AnalogChannel * Channel )
{

	this -> Channel = Channel;

	Unit = IRDistanceSensor :: kMeters;
	UseAverage = true;

	range_t In ( 0.3, 3.3 );
	range_t Out ( 0.8, 0.09 );

	In.Low = 0.3;
	In.High = 3.3;
	Out.Low = 0.8;
	Out.High = 0.09;

	Attenuator = new MapFilter ( In, Out );

};

IRDistanceSensor :: ~IRDistanceSensor ()
{

	delete Attenuator;

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

void IRDistanceSensor :: CalibLowPoint ( double Intended )
{

	range_t CalRange = Attenuator -> GetRangeIn ();
	range_t IntRange = Attenuator -> GetRangeOut ();
	CalRange.High = ( UseAverage ? Channel -> GetAverageVoltage () : Channel -> GetVoltage () );
	IntRange.Low = Intended;
	Attenuator -> SetRangeIn ( CalRange );
	Attenuator -> SetRangeOut ( CalRange );

};

void IRDistanceSensor :: CalibHighPoint ( double Intended )
{

	range_t CalRange = Attenuator -> GetRangeIn ();
	range_t IntRange = Attenuator -> GetRangeOut ();
	CalRange.Low = ( UseAverage ? Channel -> GetAverageVoltage () : Channel -> GetVoltage () );
	IntRange.High = Intended;
	Attenuator -> SetRangeIn ( CalRange );
	Attenuator -> SetRangeOut ( CalRange );

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
