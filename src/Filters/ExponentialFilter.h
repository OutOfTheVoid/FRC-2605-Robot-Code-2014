#ifndef SSM_2605_EXPONENTIAL_FILTER_H
#define SSM_2605_EXPONENTIAL_FILTER_H

/*
* Copyright (C) 2014 Liam Taylor
* FRC Team Sehome Semonsters 2605
*/

#include "Filter.h"
#include <math.h>

class ExponentialFilter : public Filter
{
public:

	ExponentialFilter ( double Exponent );
	~ExponentialFilter ();

	void SetExponent ( double Exponent );

	double Compute ( double Value );

private:

	double Exponent;

};

#endif
