#include "DeadbandFilter.h"

DeadbandFilter :: DeadbandFilter ( range_t Deadzone ):
	Deadzone ( 0, 0 )
{

	SetDeadzone ( Deadzone );

};

DeadbandFilter :: ~DeadbandFilter ()
{
};

double DeadbandFilter :: Compute ( double Value )
{

	double Center = ( Deadzone.High + Deadzone.Low ) / 2.0;
	double Range = fabs ( Deadzone.High - Deadzone.Low ) / 2.0;

	Value -= Center;

	bool Sign = ( Value < 0 );

	Value = fabs ( Value );
	Value -= Range;

	if ( Value < 0 )
		Value = 0;

	if ( Sign )
		Value *= -1;

	Value += Center;

	return Value;

};

void DeadbandFilter :: SetDeadzone ( range_t Deadzone )
{
	
	this -> Deadzone = Deadzone;

};
