#ifndef SHS_2605_CANJAGUAR_SERVER_H
#define SHS_2605_CANJAGUAR_SERVER_H

/*
* Copyright (C) 2014 Liam Taylor
* FRC Team Sehome Semonsters 2605
*/

#include "WPILib.h"
#include "src/CANJaguarUtils/CANJaguarUtils.h"
#include "src/Util/Vector.h"
#include "src/Logging/Logger.h"

#define CANJAGSERVER_PARSE_TIMEOUT_DEFAULT 100
#define CANJAGSERVER_COMMAND_TIMEOUT_DEFAULT 200

#define CANJAGSERVER_CHECKINTERVAL_DEFAULT 0.1

#define CANJAGSERVER_CANBUS_UPDATEINTERVAL_DEFAULT 0.01

#define CANJAGSERVER_MESSAGEQUEUE_LENGTH 200

#define CANJAGSERVER_PRIORITY 10
#define CANJAGSERVER_STACKSIZE 0x40000

/*
* This "server" is essentially a motor control thread. It implements a continuous message loop for receiving commands to send to the jaguars. 
* I initially implemented it as a way to keep CANJaguar objects in one thread context, due to problems with cross-thread usage not properly setting speeds.
*/

class CANJaguarServer
{
public:

	CANJaguarServer ( bool DoBrownOutCHeck = true, double BrownOutCheckInterval = CANJAGSERVER_CHECKINTERVAL_DEFAULT, double CANBusUpdateInterval = CANJAGSERVER_CANBUS_UPDATEINTERVAL_DEFAULT, uint32_t CommandTimeout = CANJAGSERVER_COMMAND_TIMEOUT_DEFAULT, uint32_t ParseTimeout = CANJAGSERVER_PARSE_TIMEOUT_DEFAULT );
	~CANJaguarServer ();

	void SetParseMessageTimeout ( uint32_t ParseTimeout );
	void SetCommandMessageTimeout ( uint32_t CommandTimeout );
	void SetBrownOutCheckEnabled ( bool DoBrownOutCheck );

	void SetJagCheckInterval ( double Interval );
	void SetCANBusUpdateInterval ( double Interval );

	bool Start ();
	void Stop ();

	bool WaitForServerActive ();

	void IsRunning ();

	void AddJag ( CAN_ID ID, CANJagConfigInfo Info );
	void RemoveJag ( CAN_ID ID );

	void DisableJag ( CAN_ID ID );
	void EnableJag ( CAN_ID ID, double EncoderInitialPosition = 0.0 );

	void ConfigJag ( CAN_ID, CANJagConfigInfo );

	void SetJag ( CAN_ID ID, float Speed, uint8_t SyncGroup = 0 );
	float GetJag ( CAN_ID ID );
	float GetJagPosition ( CAN_ID ID );

	float GetJagBusVoltage ( CAN_ID ID );
	float GetJagOutputVoltage ( CAN_ID ID );
	float GetJagOutputCurrent ( CAN_ID ID );

	bool CheckSendError ();
	void ClearSendError ();

	void UpdateJagSyncGroup ( uint8_t SyncGroup );

	void RunLoop ();

	enum CANJagServerSendMessageType
	{

		SEND_MESSAGE_NOP = 0,
		SEND_MESSAGE_JAG_DISABLE,
		SEND_MESSAGE_JAG_ENABLE,
		SEND_MESSAGE_JAG_GET,
		SEND_MESSAGE_JAG_SET,
		SEND_MESSAGE_JAG_ADD,
		SEND_MESSAGE_JAG_REMOVE,
		SEND_MESSAGE_JAG_CONFIG,
		SEND_MESSAGE_JAG_UPDATE_SYNC_GROUP,
		SEND_MESSAGE_JAG_GET_BUS_VOLTAGE,
		SEND_MESSAGE_JAG_GET_OUTPUT_VOLTAGE,
		SEND_MESSAGE_JAG_GET_OUTPUT_CURRENT,
		SEND_MESSAGE_JAG_GET_POSITION,
		SEND_MESSAGE_WAIT_SERVER_UP,

	};

	typedef struct CANJagServerMessage
	{

		uint32_t Command;
		uint32_t Data;

	} CanJagServerMessage;

	typedef struct EnableCANJagMessage
	{

		CAN_ID ID;
		double EncoderInitialPosition;

	} EnableCANJagMessage;

	typedef struct ServerCANJagInfo
	{

		CAN_ID ID;
		CANJaguar * Jag;
		CANJagConfigInfo Info;

	} ServerCanJagInfo;

	typedef struct SetCANJagMessage
	{

		CAN_ID ID;
		float Speed;
		uint8_t SyncGroup;

	} SetJagMessage;

	typedef struct ConfigCANJagMessage
	{

		CAN_ID ID;
		CANJagConfigInfo Config;

	} ConfigCANJagMessage;

	typedef ConfigCANJagMessage AddCANJagMessage;

	typedef struct GetCANJagMessage
	{

		CAN_ID ID;
		float Value;

	};
	typedef GetCANJagMessage GetCANJagPositionMessage;
	typedef GetCANJagMessage GetCANJagBusVoltageMessage;
	typedef GetCANJagMessage GetCANJagOutputVoltageMessage;
	typedef GetCANJagMessage GetCANJagOutputCurrentMessage;

private:

	bool Running;
	bool SendError;

	Task * ServerTask;

	MSG_Q_ID MessageSendQueue;
	MSG_Q_ID MessageReceiveQueue;
	SEM_ID ResponseSemaphore;

	double CANUpdateInterval;
	double JagCheckInterval;

	bool CheckJags;

	uint32_t ParseWait;
	uint32_t CommandWait;

	Vector <ServerCanJagInfo> * Jags;

	static void _StartServerTask ( CANJaguarServer * Server );

	Logger * Log;

};

#endif
