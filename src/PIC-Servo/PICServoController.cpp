#include "PICServoController.h"

PICServoController :: PICServoController ( uint8_t GroupAddress )
{

	this -> GroupAddress = GroupAddress;

	Modules = new PICServo * [ 256 ];

	for ( uint8_t i = 0; i != 255; i ++ )
		Modules [ i ] = NULL;

	Com = new PICServoCom ();
	PipeServer = new AnalogCANJaguarPipeServer ();

	PipeServer -> Start ();

};

PICServoController :: ~PICServoController ()
{



};

void PICServoController :: AddPICServo ( uint8_t ModuleNumber, bool Initialize, CAN_ID JaguarID, uint8_t AnalogChannel )
{

	AddPICServo ( ModuleNumber, Initialize, JaguarID, AnalogChannel, SensorBase :: GetDefaultAnalogModule () );

};

void PICServoController :: AddPICServo ( uint8_t ModuleNumber, bool Initialize, CAN_ID JaguarID, uint8_t AnalogChannel, uint8_t AnalogModule )
{

	PICServo * Module;

	Com -> SerialTaskLock ();

	if ( Modules [ ModuleNumber ] != NULL )
	{

		Module = Modules [ ModuleNumber ];

		PipeServer -> DisablePipe ( Module -> MotorPipe );
		PipeServer -> RemovePipe ( Module -> MotorPipe );

		Module -> MotorPipe = PipeServer -> AddPipe ( JaguarID, AnalogChannel, AnalogModule );

		Com -> ModuleStopMotor ( ModuleNumber, false, true, true );
		Com -> ReceiveStatusPacket ();

	}
	else
	{

		Module = new PICServo ( ModuleNumber, this, PipeServer -> AddPipe ( JaguarID, AnalogChannel, AnalogModule ) );
		Modules [ ModuleNumber ] = Module;

		if ( Initialize )
		{

			Com -> ModuleSetAddress ( 0, ModuleNumber, GroupAddress );
			Com -> ReceiveStatusPacket ();

		}


		Com -> ModuleStopMotor ( ModuleNumber, false, true, true );
		Com -> ReceiveStatusPacket ();

	}

	Com -> SerialTaskUnlock ();

};

PICServo * PICServoController :: GetModule ( uint8_t Module )
{

	return Modules [ Module ];

};

void PICServoController :: PICServoEnable ( uint8_t ModuleNumber )
{

	PICServo * Module = Modules [ ModuleNumber ];

	PipeServer -> EnablePipe ( Module -> MotorPipe );

};

void PICServoController :: PICServoDisable ( uint8_t ModuleNumber )
{

	PICServo * Module = Modules [ ModuleNumber ];

	PipeServer -> DisablePipe ( Module -> MotorPipe );

};

void PICServoController :: PICServoSetPWM ( uint8_t ModuleNumber, int16_t PWM )
{

	Com -> SerialTaskLock ();

	Com -> ModuleLoadTrajectory ( ModuleNumber, 0, 0, 0, PWM, false, false, false, true, false, false, false, true );
	Com -> ReceiveStatusPacket ();

	Com -> SerialTaskUnlock ();

};

void PICServoController :: PICServoSetPosition ( uint8_t ModuleNumber, double Position )
{

	Com -> SerialTaskLock ();

	Com -> ModuleLoadTrajectory ( ModuleNumber, static_cast <uint32_t> ( Position ), 0.0, 0.0, 0, true, false, false, false, true, false, false, true );
	Com -> ReceiveStatusPacket ();

	Com -> SerialTaskUnlock ();

};

void PICServoController :: PICServoSetPositionV ( uint8_t ModuleNumber, double Position, double Velocity )
{

	Com -> SerialTaskLock ();

	Com -> ModuleLoadTrajectory ( ModuleNumber, static_cast <uint32_t> ( Position ), static_cast <uint32_t> ( Velocity ), 0.0, 0, true, true, false, false, true, false, false, true );
	Com -> ReceiveStatusPacket ();

	Com -> SerialTaskUnlock ();

};

void PICServoController :: PICServoSetPositionA ( uint8_t ModuleNumber, double Position, double Acceleration )
{

	Com -> SerialTaskLock ();

	Com -> ModuleLoadTrajectory ( ModuleNumber, static_cast <uint32_t> ( Position ), 0.0, static_cast <uint32_t> ( Acceleration ), 0, true, false, true, false, true, false, false, true );
	Com -> ReceiveStatusPacket ();

	Com -> SerialTaskUnlock ();

};

void PICServoController :: PICServoSetPositionVA ( uint8_t ModuleNumber, double Position, double Velocity, double Acceleration )
{

	Com -> SerialTaskLock ();

	Com -> ModuleLoadTrajectory ( ModuleNumber, static_cast <uint32_t> ( Position ), Velocity, Acceleration, 0, true, true, true, false, true, false, false, true );
	Com -> ReceiveStatusPacket ();

	Com -> SerialTaskUnlock ();

};

void PICServoController :: PICServoResetPosition ( uint8_t ModuleNumber )
{

	Com -> SerialTaskLock ();

	Com -> ModuleResetPosition ( ModuleNumber, false );
	Com -> ReceiveStatusPacket ();

	Com -> SerialTaskUnlock ();

};

void PICServoController :: PICServoSetCurrentPosition ( uint8_t ModuleNumber, double Position )
{

	Com -> SerialTaskLock ();

	Com -> ModuleOverwritePosition ( ModuleNumber, static_cast <int32_t> ( Position ) );
	Com -> ReceiveStatusPacket ();

	Com -> SerialTaskUnlock ();

};

double PICServoController :: PICServoReadPosition ( uint8_t ModuleNumber )
{

	PICServoCom :: PICServoStatus_t Status;

	Com -> SerialTaskLock ();

	Com -> ModuleReadStatus ( ModuleNumber, PICSERVO_STATUS_TYPE_POSITION, & Status );

	Com -> SerialTaskUnlock ();

	return Status.Position;

};

void PICServoController :: PICServoSetPID ( uint8_t ModuleNumber, double P, double I, double D )
{

	Com -> SerialTaskLock ();

	Com -> ModuleSetMetrics ( ModuleNumber, static_cast <uint16_t> ( P * 1024 ), static_cast <uint16_t> ( I * 1024 ), static_cast <uint16_t> ( D * 1024 ) );
	Com -> ReceiveStatusPacket ();

	Com -> SerialTaskUnlock ();

};

void PICServoController :: PICServoSetVelocityA ( uint8_t ModuleNumber, double Velocity, double Acceleration )
{

	Com -> SerialTaskLock ();

	Com -> ModuleLoadTrajectory ( ModuleNumber, 0, Velocity, Acceleration, 0, false, true, true, false, true, true, false, true );
	Com -> ReceiveStatusPacket ();

	Com -> SerialTaskUnlock ();


};

void PICServoController :: PICServoSetVelocity ( uint8_t ModuleNumber, double Velocity )
{

	Com -> SerialTaskLock ();

	Com -> ModuleLoadTrajectory ( ModuleNumber, 0, Velocity, 0, 0, false, true, false, false, true, true, false, true );
	Com -> ReceiveStatusPacket ();

	Com -> SerialTaskUnlock ();

};

/*AnalogCANJaguarPipe_t PICServoController :: GetPipeID ( uint8_t ModuleNumber )
{

	if (  )

};
*/
