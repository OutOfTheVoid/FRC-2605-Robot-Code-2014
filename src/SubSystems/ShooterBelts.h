#ifndef SHS_2605_SHOOOTERBELTS_H
#define SHS_2605_SHOOOTERBELTS_H

#include "WPILib.h"
#include "src/Sensors/AnalogSensor.h"
#include "src/Math/SHSMath.h"

typedef struct
{
	
	SpeedController * Motor;
	bool Inverted;
	
} BeltMotor;

class ShooterBelts
{
public:

	ShooterBelts ( SpeedController * RollerBL, SpeedController * RollerBR);
	~ShooterBelts ();

	void SetMotors ( SpeedController * BeltBL, SpeedController * BeltBR );
	void SetInverted ( bool BL, bool BR );

	void SetPreScale ( double Prescale );

	void SetMotorScale ( double Scale );

	void SetSpeed ( double Speed );

	void PushSpeeds ();

	bool Enable ();
	void Disable ();
	
	bool GetEnabled ();

private:

	bool Enabled;

	double S, Scale, Prescale;

	BeltMotor RollerL, RollerR;

};

#endif
