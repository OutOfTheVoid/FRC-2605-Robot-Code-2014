#ifndef SHS_2605_FILTER_H
#define SHS_2605_FILTER_H

/*
* Copyright (C) 2014 Liam Taylor
* FRC Team Sehome Semonsters 2605
*/

class Filter
{
public:

	virtual ~Filter () {};

	virtual double Compute ( double Value ) = 0;

};

#endif
