#include "IRDistanceSensor.h"

IRDistanceSensor :: IRDistanceSensor ( AnalogChannel * Channel )
{

	this -> Channel = Channel;

	Unit = IRDistanceSensor :: kMeters;
	UseAverage = true;

	range_t In ( 0.3, 3.3 );
	range_t Out ( 0.8, 0.09 );
	range_t Calc ( 0.0, 1.0 );

	In.Low = 0.3;
	In.High = 3.3;
	Out.Low = 0.8;
	Out.High = 0.09;

	InAttenuator = new MapFilter ( In, Calc );
	OutAttenuator = new MapFilter ( Calc, Out );

};

IRDistanceSensor :: ~IRDistanceSensor ()
{

	delete InAttenuator;
	delete OutAttenuator;

};

void IRDistanceSensor :: SetMeasurementUnits ( MeasurementUnit Units )
{

	Unit = Units;

};

void IRDistanceSensor :: SetInputVoltageRange ( range_t In )
{

	InAttenuator -> SetRangeIn ( In );

};

void IRDistanceSensor :: SetOutputRange ( range_t Out )
{

	OutAttenuator -> SetRangeOut ( Out );

};

range_t IRDistanceSensor :: GetOutputRange ()
{

	return OutAttenuator -> GetRangeOut ();

};

void IRDistanceSensor :: CalibLowPoint ( double Intended )
{

	range_t CalRange = InAttenuator -> GetRangeIn ();
	range_t IntRange = OutAttenuator -> GetRangeOut ();
	CalRange.Low = ( UseAverage ? Channel -> GetAverageVoltage () : Channel -> GetVoltage () );
	IntRange.Low = Intended;
	InAttenuator -> SetRangeIn ( CalRange );
	OutAttenuator -> SetRangeOut ( IntRange );

};

void IRDistanceSensor :: CalibHighPoint ( double Intended )
{

	range_t CalRange = InAttenuator -> GetRangeIn ();
	range_t IntRange = OutAttenuator -> GetRangeOut ();
	CalRange.High = ( UseAverage ? Channel -> GetAverageVoltage () : Channel -> GetVoltage () );
	IntRange.High = Intended;
	InAttenuator -> SetRangeIn ( CalRange );
	OutAttenuator -> SetRangeOut ( IntRange );

};

double IRDistanceSensor :: Get ()
{

	double VIn = InAttenuator -> Compute ( UseAverage ? Channel -> GetAverageVoltage () : Channel -> GetVoltage () );
	double VOut = OutAttenuator -> Compute ( VIn );

	switch ( Unit )
	{

		case kMeters:

			return VOut;

			break;

		case kCentimeters:

			return 100 * VOut;

			break;

		case kFeet:

			return 3.28084 * VOut;

			break;

		case kInches:

			return 39.3701 * VOut;

			break;

		default:

			return VOut;

			break;

	}

	return 0;

};
