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

	ShooterBelts ( SpeedController * RollerBL, SpeedController * RollerBR, SpeedController * ArmSL, SpeedController * ArmSR );
	~ShooterBelts ();

	void SetMotors ( SpeedController * BeltBL, SpeedController * BeltBR, SpeedController * ArmSL, SpeedController * ArmRS );
	void SetInverted ( bool BL, bool BR, bool SL, bool SR );

	void SetBeltPreScale ( double Prescale );
	void SetArmPreScale ( double Prescale );

	void SetBeltMotorScale ( double Scale );
	void SetArmMotorScale ( double Scale );

	void SetArmPosition ( double Position );
	void SetBeltSpeed ( double Speed );

	void PushSpeeds ();

	bool Enable ();
	void Disable ();
	
	bool GetEnabled ();

private:

	bool Enabled;

	double S, A, ScaleR, ScaleA, PrescaleR, PrescaleA;

	BeltMotor RollerL, RollerR, ArmL, ArmR;

};

#endif
