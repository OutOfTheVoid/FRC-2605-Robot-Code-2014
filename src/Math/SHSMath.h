#ifndef SHS_2605_MATH_H
#define SHS_2605_MATH_H

typedef struct range_struct
{

	range_struct ( double Low, double High )
	{

		this -> Low = Low;
		this -> High = High;

	};

	double Low;
	double High;

} range_t;

#endif
