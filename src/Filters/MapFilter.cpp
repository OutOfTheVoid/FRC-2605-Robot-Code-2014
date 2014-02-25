#include "MapFilter.h"

MapFilter :: MapFilter ():
	RIn ( 0, 1 ),
	ROut ( 0, 1 )
{
	
	Off = 0;
	Mul = 1;
	
}

MapFilter :: MapFilter ( range_t In, range_t Out ):
	RIn ( In ),
	ROut ( Out )
{

	Mul = ( ROut.High - ROut.Low ) / ( RIn.High - RIn.Low );

	Off = RIn.Low;
	Off -= ROut.Low / Mul;

};

MapFilter :: MapFilter ( double Offset, double Multiplier ):
	RIn ( 0, 1 ),
	ROut ( 0, 1 )
{
	
	RIn.Low = 0;
	RIn.High = 1;
	ROut.Low = Offset;
	ROut.High = Offset + Multiplier;

	Off = Offset;
	Mul = Multiplier;

};

MapFilter :: ~MapFilter ()
{



};

void MapFilter :: SetRangeIn ( range_t In )
{

	RIn = In;

	Mul = ( ROut.High - ROut.Low ) / ( RIn.High - RIn.Low );
	
	Off = RIn.Low;
	Off -= ROut.Low / Mul;

};

void MapFilter :: SetRangeOut ( range_t Out )
{

	ROut = Out;

	Mul = ( ROut.High - ROut.Low ) / ( RIn.High - RIn.Low );
	
	Off = RIn.Low;
	Off -= ROut.Low / Mul;

};

void MapFilter :: Set ( double Offset, double Multiplier )
{
	
	RIn.Low = 0;
	RIn.High = 1;
	ROut.Low = Offset;
	ROut.High = Offset + Multiplier;
	
	Off = Offset;
	Mul = Multiplier;
	
};

range_t MapFilter :: GetRangeIn ()
{

	return RIn;

};

range_t MapFilter :: GetRangeOut ()
{

	return ROut;

};

double MapFilter :: Compute ( double Value )
{

	return ( Value - Off ) * Mul;

};
