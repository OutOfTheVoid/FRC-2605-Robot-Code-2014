#ifndef SHS_2605_MECANUM_DRIVE_H
#define SHS_2605_MECANUM_DRIVE_H

/*
* Copyright (C) 2014 Liam Taylor
* FRC Team Sehome Semonsters 2605
*/

#include "WPILib.h"
#include <math.h>

#define PI_Div_4 0.78539816339
#define SQRT_2 1.41421356237

typedef struct
{
	
	SpeedController * Motor;
	bool Inverted;
	
} MecMotor;

class MecanumDrive
{
public:

	MecanumDrive ( SpeedController * WwheelFL, SpeedController * WheelFR, SpeedController * WheelRL, SpeedController * WheelRR );
	~MecanumDrive ();
	
	void SetMotors ( SpeedController * WwheelFL, SpeedController * WheelFR, SpeedController * WheelRL, SpeedController * WheelRR );
	
	void SetInverted ( bool FL, bool FR, bool RL, bool RR );

	bool Enable ();
	void Disable ();
	
	bool GetEnabled ();
	
	void SetMotorScale ( double s );
	double GetMotorScale ();
	
	void SetPreScale ( double Translation, double Rotation );
	double GetPreScaleTranslation ();
	double GetPreScaleRotation ();
	
	void SetTranslation ( double X, double Y );
	void SetRotation ( double R );
	
	void SetSineInversion ( bool Inverted );

	void DebugValues ();
	
	void PushTransform ();
	
private:
	
	MecMotor MotorFL, MotorFR, MotorRL, MotorRR;
	
	double TX, TY, TR, Scale, PrescaleR, PrescaleT;

	bool Enabled, SineInverted;
	
};

#endif
