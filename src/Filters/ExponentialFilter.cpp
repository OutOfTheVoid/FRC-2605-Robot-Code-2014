#include "ExponentialFilter.h"

/*
* Copyright (C) 2014 Liam Taylor
* FRC Team Sehome Semonsters 2605
*/

ExponentialFilter :: ExponentialFilter ( double Exponent )
{

	SetExponent ( Exponent );

};

ExponentialFilter :: ~ExponentialFilter ()
{
};

void ExponentialFilter :: SetExponent ( double Exponent )
{

	this -> Exponent = Exponent;

};

double ExponentialFilter :: Compute ( double Value )
{

	bool Sign = ( Value < 0 );
	return fabs ( pow ( Value, Exponent ) ) * ( Sign ? - 1.0f : 1.0f );

};
