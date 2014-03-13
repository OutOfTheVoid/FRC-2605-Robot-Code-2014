#ifndef SHS_2605_WENCH_H
#define SHS_2605_WENCH_H

#include "src/PIC-Servo/PICServo.h"

#include <cmath>

class ShooterWinch
{
public:

	ShooterWinch ( PICServo * WinchServo );
	~ShooterWinch ();

	bool Enable ();
	void Disable ();
	bool GetEnabled ();

	void SetInverted ( bool Inverted );

	void SetZero ();

	bool WithinAngle ( double Threshold, double Angle );

	bool DriveAngle ( double Angle );
	void DrivePWM ( double Value );

	void Stop ();

private:

	bool Inverted;

	PICServo * M;

	bool Enabled;

};

#endif
