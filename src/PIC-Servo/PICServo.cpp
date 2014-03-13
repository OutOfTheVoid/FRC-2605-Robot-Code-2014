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

	StatusType = 0x00;

	this -> Enabled = false;

	LastSetValue = - 1000000000000000000.0;

};

PICServo :: ~PICServo ()
{
};

void PICServo :: SetControlMode ( PICServoControlMode ControlMode )
{

	this -> ControlMode = ControlMode;

	LastSetValue = - 1000000000000000000.0;

};

PICServo :: PICServoControlMode PICServo :: GetControlMode ()
{

	return ControlMode;

};

void PICServo :: SetHardwearLimitingEnabled ( bool Enabled )
{

	Controller -> PICServoSetLimiting ( ModuleNumber, Enabled );

}

void PICServo :: SetEncoderResolution ( uint32_t EncoderCountsPerRev )
{

	if ( Enabled )
		return;

	EncoderCount = EncoderCountsPerRev;

};

void PICServo :: Enable ()
{

	Controller -> PICServoEnable ( ModuleNumber );

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

	return ( static_cast <double> ( Controller -> PICServoReadPosition ( ModuleNumber ) ) / static_cast <double> ( EncoderCount ) );

};

bool PICServo :: GetLimit1 ()
{

	return Controller -> PICServoGetLimit1 ( ModuleNumber );

};

bool PICServo :: GetLimit2 ()
{

	return Controller -> PICServoGetLimit2 ( ModuleNumber );

};

bool PICServo :: GetMoveDone ()
{

	return Controller -> PICServoGetMoveDone ( ModuleNumber );

};

void PICServo :: ConfigVelocity ( double Velocity )
{

	if ( this -> Velocity != Velocity )
	{

		this -> Velocity = Velocity;

		NewVelocity = true;
		LastSetValue = - 1000000000000000000.0;

	}

};

void PICServo :: ConfigAcceleration ( double Acceleration )
{

	if ( this -> Acceleration != Acceleration )
	{

		this -> Acceleration = Acceleration;

		NewAcceleration = true;
		LastSetValue = - 1000000000000000000.0;

	}

};

void PICServo :: Set ( double Value )
{

	if ( LastSetValue != Value )
	{

		LastSetValue = Value;

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

	}

	LastSet = Value;

};

void PICServo :: ResetPosition ()
{

	Controller -> PICServoResetPosition ( ModuleNumber );

	LastSetValue = - 1000000000000000000.0;

};

void PICServo :: SetCurrentPosition ( double Position )
{

	Controller -> PICServoSetCurrentPosition ( ModuleNumber, Position * static_cast <double> ( EncoderCount ) ); //

};

void PICServo :: SetPID ( double P, double I, double D, double OutputLimit, double PositionErrorLimit, double DeadbandCompensation )
{

	if ( PositionErrorLimit == DBL_MAX )
		PositionErrorLimit = 32767;
	else
		PositionErrorLimit = PositionErrorLimit * static_cast <double> ( EncoderCount );

	Controller -> PICServoSetPID ( ModuleNumber, P, I, D, OutputLimit, static_cast <uint32_t> ( PositionErrorLimit ), static_cast <uint8_t> ( DeadbandCompensation * 0xFF ) );

	this -> P = P;
	this -> I = I;
	this -> D = D;

};

double PICServo :: GetLastP ()
{

	return P;

};

double PICServo :: GetLastI ()
{

	return I;

};

double PICServo :: GetLastD ()
{

	return D;

};

void PICServo :: CalibrateAnalog ()
{

	Controller -> PICServoCalibrateAnalog ( ModuleNumber );

};

void PICServo :: SetAnalogInverted ( bool Inverted )
{

	Controller -> PICServoSetAnalogInverted ( ModuleNumber, Inverted );

};
