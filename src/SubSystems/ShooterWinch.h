#ifndef SHS_2605_WENCH_H
#define SHS_2605_WENCH_H

#include "src/CANJagServer/AsynchCANJaguar.h"

#include "src/CANJaguarUtils/CANJaguarUtils.h"

#include <cmath>

class ShooterWinch
{
public:

	ShooterWinch ( AsynchCANJaguar * Motor, CANJagConfigInfo ServoConfig, CANJagConfigInfo OpenConfig );
	~ShooterWinch ();

	bool Enable ();
	void Disable ();
	bool GetEnabled ();

	void SetInverted ( bool Inverted );
	void SetPreScale ( double PreScale );

	void SetZero ();

	double GetAngle ();

	bool WithinAngle ( double Threshold, double Angle );
	void DriveAngle ( double Angle );
	void Stop ();

private:

	bool Inverted;

	AsynchCANJaguar * M;

	CANJagConfigInfo ServoConfig, OpenConfig;

	double Zero;
	double PreScale;

	bool Enabled;

};

#endif
