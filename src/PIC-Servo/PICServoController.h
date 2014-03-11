#ifndef SHS_2605_PIC_SERVO_CONTROLLER_H
#define SHS_2605_PIC_SERVO_CONTROLLER_H

#include "PICServo.h"
#include "PICServoCom.h"
#include "AnalogCANJaguarPipeServer.h"

#include "WPILib.h"

#include "src/Logging/Logger.h"

#define PICSERVO_CONTROL_TASK_PRIORITY 50
#define PICSERVO_CONTROL_TASK_STACKSIZE 0x20000
#define MAXIMUM_UPDATE_DELTA_TIME 0.001

class PICServoController
{

	friend class PICServo;

public:

	PICServoController ( uint8_t GroupAddress = 0xFF );
	~PICServoController ();

	void AddPICServo ( uint8_t ModuleNumber, bool Initialize, CAN_ID JaguarID, uint8_t AnalogChannel, uint8_t AnalogModuleNumber );
	void AddPICServo ( uint8_t ModuleNumber, bool Initialize, CAN_ID JaguarID, uint8_t AnalogChannel );

	PICServo * GetModule ( uint8_t ModuleNumber );

private:

	typedef struct
	{

		uint32_t Command;
		uint32_t Data;

	} ServerMessage;

	typedef struct 
	{
		
		uint8_t Index;
		int16_t	Value;

	} SetPWMMessage;

	typedef struct
	{

		uint8_t Index;
		double Position;

	} SetPositionMessage;

	typedef struct
	{

		uint8_t Index;
		double Position;
		double Velocity;

	} SetPositionVMessage;

	typedef struct
	{

		uint8_t Index;
		double Position;
		double Acceleration;

	} SetPositionAMessage;

	typedef struct
	{
		
		uint8_t Index;
		double Position;
		double Velocity;
		double Acceleration;

	} SetPositionVAMessage;

	typedef SetPositionMessage SetCurrentPositionMessage;

	typedef struct
	{

		uint8_t Index;
		double Velocity;

	} SetVelocityMessage;

	typedef struct
	{

		uint8_t Index;
		double Velocity;
		double Acceleration;

	} SetVelocityAMessage;

	typedef struct
	{
		
		uint8_t Index;
		double P;
		double I;
		double D;
		double MaxOutput;
		uint32_t PositionError;
		uint8_t DeadbandCompensation;

	} SetPIDMessage;

	enum PICServoMessageType
	{

		PICSERVO_DISABLE_MESSAGE = 0,
		PICSERVO_ENABLE_MESSAGE,
		PICSERVO_SETPWM_MESSAGE,
		PICSERVO_SETPOSITION_MESSAGE,
		PICSERVO_SETPOSITIONV_MESSAGE,
		PICSERVO_SETPOSITIONA_MESSAGE,
		PICSERVO_SETPOSITIONVA_MESSAGE,
		PICSERVO_RESETPOSITION_MESSAGE,
		PICSERVO_SETCURRENTPOSITION_MESSAGE,
		PICSERVO_READPOSITION_MESSAGE,
		PICSERVO_SETVELOCITY_MESSAGE,
		PICSERVO_SETVELOCITYA_MESSAGE,
		PICSERVO_SETPID_MESSAGE,
		PICSERVO_INIT_MESSAGE,
		PICSERVO_REINIT_MESSAGE,
		PICSERVO_GETLIMIT_MESSAGE,
		PICSERVO_GETMOVEDONE_MESSAGE,
		PICSERVO_SETLIMITING_MESSAGE,

	};

	void PICServoEnable ( uint8_t ModuleNumber );
	void PICServoDisable ( uint8_t ModuleNumber );

	void PICServoSetPWM ( uint8_t ModuleNumber, int16_t PWM );

	void PICServoSetPosition ( uint8_t ModuleNumber, double Position );
	void PICServoSetPositionV ( uint8_t ModuleNumber, double Position, double Velocity );
	void PICServoSetPositionA ( uint8_t ModuleNumber, double Position, double Acceleration );
	void PICServoSetPositionVA ( uint8_t ModuleNumber, double Position, double Velocity, double Acceleration );

	void PICServoResetPosition ( uint8_t ModuleNumber );
	void PICServoSetCurrentPosition ( uint8_t ModuleNumber, double Position );

	int32_t PICServoReadPosition ( uint8_t ModuleNumber );

	void PICServoSetVelocityA ( uint8_t ModuleNumber, double Velocity, double Acceleration );
	void PICServoSetVelocity ( uint8_t ModuleNumber, double Velocity );

	void PICServoSetPID ( uint8_t ModuleNumber, double P, double I, double D, double MaxOutput, uint32_t PositionErrorLimit, uint8_t DeadbandCompensation );

	void PICServoCalibrateAnalog ( uint8_t ModuleNumber );
	void PICServoSetAnalogInverted ( uint8_t ModuleNumber, bool Inverted );

	bool PICServoGetLimit1 ( uint8_t ModuleNumber );
	bool PICServoGetLimit2 ( uint8_t ModuleNumber );

	bool PICServoGetMoveDone ( uint8_t ModuleNumber );

	void PICServoSetLimiting ( uint8_t ModuleNumber, bool Limiting );

	void RunLoop ();

	PICServo ** Modules;

	PICServoCom * Com;
	AnalogCANJaguarPipeServer * PipeServer;

	uint8_t GroupAddress;

	MSG_Q_ID SendMessageQueue;
	MSG_Q_ID ReceiveMessageQueue;
	SEM_ID ResponseSemaphore;
	SEM_ID ModuleSemaphore;

	Task * ServerTask;

	bool Started;

	Logger * Log;

	static int _StartServerTask ( PICServoController * This );

};

#endif
