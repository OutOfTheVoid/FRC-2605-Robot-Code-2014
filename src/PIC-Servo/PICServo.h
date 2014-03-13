#ifndef SHS_2605_PICSERVO_H
#define SHS_2605_PICSERVO_H

#include "PICServoController.h"
#include "AnalogCANJaguarPipeServer.h"
#include "PICServoCom.h"

#define DBL_MAX 1.7976931348623157e+308

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

	bool GetLimit1 ();
	bool GetLimit2 ();

	void SetHardwearLimitingEnabled ( bool Enabled );

	bool GetMoveDone ();

	void Set ( double Value );

	void ResetPosition ();
	void SetCurrentPosition ( double Position );

	void SetPID ( double P, double I, double D, double OutputLimit = 1, double PositionErrorLimit = DBL_MAX, double DeadbandCompensation = 0.0 );

	double GetLastP ();
	double GetLastI ();
	double GetLastD ();

	void CalibrateAnalog ();
	void SetAnalogInverted ( bool Inverted );

private:

	PICServo ( uint8_t ModuleAddress, PICServoController * Controller, AnalogCANJaguarPipe_t MotorPipe );
	~PICServo ();

	PICServoController * Controller;
	AnalogCANJaguarPipe_t MotorPipe;

	double LastSet;

	double Acceleration;
	double Velocity;
	double LastSetValue;

	double P;
	double I;
	double D;

	bool NewAcceleration;
	bool NewVelocity;

	uint8_t ModuleNumber;
	uint32_t EncoderCount;

	PICServoControlMode ControlMode;
	bool Enabled;

	uint8_t StatusType;

	double LastStatusTime;

	PICServoCom :: PICServoStatus_t LastStatus;

};

#endif
