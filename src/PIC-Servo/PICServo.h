#ifndef SHS_2605_PICSERVO_H
#define SHS_2605_PICSERVO_H

#include "PICServoController.h"
#include "AnalogCANJaguarPipeServer.h"

#define PICSERVO_SERVO_RATE 1953.125

class PICServo
{

	friend class PICServoController;

public:

	typedef enum
	{ 

		kPWM = 0,
		kVelocity, 
		kPosition 
	
	} PICServoControlMode;

	void SetControlMode ( PICServoControlMode ControlMode );
	PICServoControlMode GetControlMode ();

	void SetEncoderResolution ( uint32_t EncoderCountsPerRev );

	void Enable ();
	void Disable ();

	double Get ();
	double GetPosition ();

	void ConfigVelocity ( double Velocity );
	void ConfigAcceleration ( double Acceleration );

	void Set ( double Value );

	void ResetPosition ();
	void SetCurrentPosition ( double Position );

	void SetPID ( double P, double I, double D );

private:

	PICServo ( uint8_t ModuleAddress, PICServoController * Controller, AnalogCANJaguarPipe_t MotorPipe );
	~PICServo ();

	PICServoController * Controller;
	AnalogCANJaguarPipe_t MotorPipe;

	double LastSet;

	double Acceleration;
	double Velocity;

	bool NewAcceleration;
	bool NewVelocity;

	uint8_t ModuleNumber;
	uint32_t EncoderCount;

	PICServoControlMode ControlMode;
	bool Enabled;

};

#endif
