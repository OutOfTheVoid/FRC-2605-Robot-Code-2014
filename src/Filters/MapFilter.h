#ifndef SHS_2605_MAP_FILTER_H
#define SHS_2605_MAP_FILTER_H

#include "src/Math/SHSMath.h"
#include "Filter.h"

class MapFilter : public Filter
{
public:

	MapFilter ();
	MapFilter ( range_t In, range_t Out );
	MapFilter ( double Offset, double Multiplier );
	~MapFilter ();

	void Set ( double Offset, double Multiplier );

	void SetRangeIn ( range_t In );
	void SetRangeOut ( range_t Out );

	range_t GetRangeIn ();
	range_t GetRangeOut ();

	double Compute ( double Value );

private:

	range_t RIn;
	range_t ROut;

	double Off;
	double Mul;

};

#endif
