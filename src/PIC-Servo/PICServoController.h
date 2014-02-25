#ifndef SHS_2605_PIC_SERVO_CONTROLLER_H
#define SHS_2605_PIC_SERVO_CONTROLLER_H

#include "PICServo.h"
#include "PICServoCom.h"
#include "AnalogCANJaguarPipeServer.h"

class PICServoController
{

	friend class PICServo;

public:

	PICServoController ( uint8_t GroupAddress = 0xFF );
	~PICServoController ();

	void AddPICServo ( uint8_t ModuleNumber, bool Initialize, CAN_ID JaguarID, uint8_t AnalogChannel, uint8_t AnalogModuleNumber );
	void AddPICServo ( uint8_t ModuleNumber, bool Initialize, CAN_ID JaguarID, uint8_t AnalogChannel );

	PICServo * GetModule ( uint8_t ModuleNumber );

	//AnalogCANJaguarPipe_t GetPipeID ( uint8_t ModuleNumber );

private:

	void PICServoEnable ( uint8_t ModuleNumber );
	void PICServoDisable ( uint8_t ModuleNumber );

	void PICServoSetPWM ( uint8_t ModuleNumber, int16_t PWM );

	void PICServoSetPosition ( uint8_t ModuleNumber, double Position );
	void PICServoSetPositionV ( uint8_t ModuleNumber, double Position, double Velocity );
	void PICServoSetPositionA ( uint8_t ModuleNumber, double Position, double Acceleration );
	void PICServoSetPositionVA ( uint8_t ModuleNumber, double Position, double Velocity, double Acceleration );

	void PICServoResetPosition ( uint8_t ModuleNumber );
	void PICServoSetCurrentPosition ( uint8_t ModuleNumber, double Position );

	double PICServoReadPosition ( uint8_t ModuleNumber );

	void PICServoSetVelocityA ( uint8_t ModuleNumber, double Velocity, double Acceleration );
	void PICServoSetVelocity ( uint8_t ModuleNumber, double Velocity );

	void PICServoSetPID ( uint8_t ModuleNumber, double P, double I, double D );

	PICServo ** Modules;

	PICServoCom * Com;
	AnalogCANJaguarPipeServer * PipeServer;

	uint8_t GroupAddress;

};

#endif
