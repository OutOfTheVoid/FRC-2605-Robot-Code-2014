#ifndef SHS_2605_WENCH_H
#define SHS_2605_WENCH_H

#include "WPILib.h"

class ShooterWench
{
public:

	ShooterWench ( SpeedController * WenchMotor );
	~ShooterWench ();

	bool Enable ();
	void Disable ();
	bool GetEnabled ();

	void SetMotor ( SpeedController * WenchMotor );
	void SetInverted ( bool Inverted );
	void SetMotorScale ( double Scale );

	void Open ();
	void Close ();
	void Stop ();

	void PushSpeed ();

private:

	bool Inverted;

	SpeedController * Motor;

	double Scale;

	bool Enabled;

	bool Direction;
	bool Running;

};

#endif
