#ifndef SHS_2605_DEABAND_FILTER_H
#define SHS_2605_DEABAND_FILTER_H

#include <math.h>

#include "Filter.h"
#include "src/Math/SHSMath.h"

class DeadbandFilter : public Filter
{
public:

	DeadbandFilter ( range_t Deadzone );
	~DeadbandFilter ();

	double Compute ( double Value );

	void SetDeadzone ( range_t Deadzone );

private:

	range_t Deadzone;

};

#endif
