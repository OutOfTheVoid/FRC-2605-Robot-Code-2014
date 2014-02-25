#include "PICServo.h"

PICServo :: PICServo ( uint8_t ModuleAddress, PICServoController * Controller, AnalogCANJaguarPipe_t MotorPipe )
{

	ModuleNumber = ModuleAddress;
	this -> Controller = Controller;
	this -> MotorPipe = MotorPipe;

	ControlMode = kPWM;

	Acceleration = 0;
	Velocity = 0;

	LastSet = 0;

	NewVelocity = false;
	NewAcceleration = false;

};

PICServo :: ~PICServo ()
{
};

void PICServo :: SetControlMode ( PICServoControlMode ControlMode )
{

	if ( Enabled )
		return;

	this -> ControlMode = ControlMode;

};

PICServo :: PICServoControlMode PICServo :: GetControlMode ()
{

	return ControlMode;

};

void PICServo :: SetEncoderResolution ( uint32_t EncoderCountsPerRev )
{

	if ( Enabled )
		return;

	EncoderCount = EncoderCountsPerRev;

};

void PICServo :: Enable ()
{

	Controller -> PICServoEnable ( ModuleNumber );

	if ( ControlMode == kPosition )
		ResetPosition ();

	Enabled = true;

};

void PICServo :: Disable ()
{

	Controller -> PICServoDisable ( ModuleNumber );

	Enabled = false;

};

double PICServo :: Get ()
{

	return LastSet;

};

double PICServo :: GetPosition ()
{

	return static_cast <double> ( Controller -> PICServoReadPosition ( ModuleNumber ) ) / static_cast <double> ( EncoderCount );

};

void PICServo :: ConfigVelocity ( double Velocity )
{

	if ( this -> Velocity != Velocity )
	{

		this -> Velocity = Velocity;

		NewVelocity = true;

	}

};

void PICServo :: ConfigAcceleration ( double Acceleration )
{

	if ( this -> Acceleration != Acceleration )
	{

		this -> Acceleration = Acceleration;

		NewAcceleration = true;

	}

};

void PICServo :: Set ( double Value )
{

	switch ( ControlMode )
	{
			
	case kPWM:

		Controller -> PICServoSetPWM ( ModuleNumber, static_cast <int16_t> ( Value * 0xFF ) );

		break;

	case kPosition:

		Value *= static_cast <double> ( EncoderCount );

		if ( NewVelocity )
		{

			if ( NewAcceleration )
				Controller -> PICServoSetPositionVA ( ModuleNumber, Value, Velocity, Acceleration );
			else
				Controller -> PICServoSetPositionV ( ModuleNumber, Value, Velocity );

		}
		else
		{

			if ( NewAcceleration )
				Controller -> PICServoSetPositionA ( ModuleNumber, Value, Acceleration );
			else
				Controller -> PICServoSetPosition ( ModuleNumber, Value );

		}

		NewAcceleration = false;
		NewVelocity = false;

		break;

	case kVelocity:

		Value *= static_cast <double> ( EncoderCount );
		Value /= PICSERVO_SERVO_RATE;

		if ( NewAcceleration )
			Controller -> PICServoSetVelocityA ( ModuleNumber, Value, Acceleration );
		else
			Controller -> PICServoSetVelocity ( ModuleNumber, Value );

			break;

	default:

		Controller -> PICServoSetPWM ( ModuleNumber, 0 );

		break;

	}

	LastSet = Value;

};

void PICServo :: ResetPosition ()
{

	Controller -> PICServoResetPosition ( ModuleNumber );

};

void PICServo :: SetCurrentPosition ( double Position )
{

	Controller -> PICServoSetCurrentPosition ( ModuleNumber, Position * static_cast <double> ( EncoderCount ) );

};

void PICServo :: SetPID ( double P, double I, double D )
{

	Controller -> PICServoSetPID ( ModuleNumber, P, I, D );

};
