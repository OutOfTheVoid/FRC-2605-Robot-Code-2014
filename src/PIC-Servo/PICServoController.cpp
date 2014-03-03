#include "PICServoController.h"

PICServoController :: PICServoController ( uint8_t GroupAddress )
{

	this -> GroupAddress = GroupAddress;

	Modules = new PICServo * [ 256 ];

	for ( uint8_t i = 0; i != 255; i ++ )
		Modules [ i ] = NULL;

	Started = false;

	ServerTask = new Task ( "2605_PICServo_Control_Server_Task", reinterpret_cast <FUNCPTR> ( _StartServerTask ), PICSERVO_CONTROL_TASK_PRIORITY, PICSERVO_CONTROL_TASK_STACKSIZE );

	SendMessageQueue = msgQCreate ( 200, sizeof ( ServerMessage * ), MSG_Q_FIFO );

	if ( SendMessageQueue == NULL )
		throw "PICServoController Error: Couldn't allocate a message queue for asynch command loop.";

	ReceiveMessageQueue = msgQCreate ( 16, sizeof ( ServerMessage * ), MSG_Q_FIFO );

	if ( ReceiveMessageQueue == NULL )
	{

		msgQDelete ( SendMessageQueue );

		throw "PICServoController Error: Couldn't allocate a message queue for asynch command loop.";

	}

	ResponseSemaphore = semMCreate ( SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE );

	if ( ResponseSemaphore == NULL )
	{

		msgQDelete ( SendMessageQueue );
		msgQDelete ( ReceiveMessageQueue );

		throw "PICServoController Error: Couldn't create Response synchronization semaphore.";

	};

	ModuleSemaphore = semMCreate ( SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE );

	if ( ModuleSemaphore == NULL )
	{

		msgQDelete ( SendMessageQueue );
		msgQDelete ( ReceiveMessageQueue );
		semDelete ( ResponseSemaphore );

		throw "PICServoController Error: Couldn't create Module synchronization semaphore.";

	}

	if ( ! ServerTask -> Start ( reinterpret_cast <uint32_t> ( this ) ) )
	{

		msgQDelete ( SendMessageQueue );
		msgQDelete ( ReceiveMessageQueue );
		semDelete ( ResponseSemaphore );
		semDelete ( ModuleSemaphore );

		throw "PICServoController Error: Couldn't start server task.";

	}

};

PICServoController :: ~PICServoController ()
{

	semTake ( ResponseSemaphore, WAIT_FOREVER );
	semTake ( ModuleSemaphore, WAIT_FOREVER );

	msgQDelete ( SendMessageQueue );
	msgQDelete ( ReceiveMessageQueue );
	semDelete ( ResponseSemaphore );
	semDelete ( ModuleSemaphore );

	ServerTask -> Stop ();

	semGive ( ResponseSemaphore );

};

void PICServoController :: AddPICServo ( uint8_t ModuleNumber, bool Initialize, CAN_ID JaguarID, uint8_t AnalogChannel )
{

	AddPICServo ( ModuleNumber, Initialize, JaguarID, AnalogChannel, SensorBase :: GetDefaultAnalogModule () );

};

void PICServoController :: AddPICServo ( uint8_t ModuleNumber, bool Initialize, CAN_ID JaguarID, uint8_t AnalogChannel, uint8_t AnalogModule )
{

	PICServo * Module;

	ServerMessage * ResponseMessage = NULL;

	printf ( "Taking Module Access Semaphore...\n" );

	semTake ( ModuleSemaphore, WAIT_FOREVER );

	if ( Modules [ ModuleNumber ] != NULL )
	{

		printf ( "Reconfiguring existing module...\n" );

		Module = Modules [ ModuleNumber ];

		printf ( "Destroying Pipe...\n" );

		PipeServer -> DisablePipe ( Module -> MotorPipe );
		PipeServer -> RemovePipe ( Module -> MotorPipe );

		printf ( "Createing new Pipe...\n" );

		Module -> MotorPipe = PipeServer -> AddPipe ( JaguarID, AnalogChannel, AnalogModule );

		printf ( "Taking Response Semaphore...\n" );

		semTake ( ResponseSemaphore, WAIT_FOREVER );

		printf ( "Sending REINIT Message...\n" );

		ServerMessage * SendMessage = new ServerMessage ();

		SendMessage -> Command = PICSERVO_REINIT_MESSAGE;
		SendMessage -> Data = ModuleNumber;

		msgQSend ( SendMessageQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT );

		printf ( "Waiting on receive message...\n" );

		msgQReceive ( ReceiveMessageQueue, reinterpret_cast <char *> ( & ResponseMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER );

		delete ResponseMessage;

		semGive ( ResponseSemaphore );

	}
	else
	{

		printf ( "Creating PICServo object...\n" );

		Module = new PICServo ( ModuleNumber, this, PipeServer -> AddPipe ( JaguarID, AnalogChannel, AnalogModule ) );
		Modules [ ModuleNumber ] = Module;

		printf ( "Taking response semaphore...\n" );

		semTake ( ResponseSemaphore, WAIT_FOREVER );

		if ( Initialize )
		{

			printf ( "Sending INIT Message...\n" );

			ServerMessage * SendMessage = new ServerMessage ();

			SendMessage -> Command = PICSERVO_INIT_MESSAGE;
			SendMessage -> Data = ModuleNumber;

			msgQSend ( SendMessageQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT );

			printf ( "Waiting for response message...\n" );

			msgQReceive ( ReceiveMessageQueue, reinterpret_cast <char *> ( & ResponseMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER );

			delete ResponseMessage;
			ResponseMessage = NULL;

		}

		printf ( "Sending REINIT Message...\n" );

		ServerMessage * SendMessage = new ServerMessage ();

		SendMessage -> Command = PICSERVO_REINIT_MESSAGE;
		SendMessage -> Data = ModuleNumber;

		msgQSend ( SendMessageQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT );

		printf ( "Waiting for response...\n" );

		msgQReceive ( ReceiveMessageQueue, reinterpret_cast <char *> ( & ResponseMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER );

		delete ResponseMessage;

		semGive ( ResponseSemaphore );
		semGive ( ModuleSemaphore );

	}

	Com -> SerialTaskUnlock ();

};

PICServo * PICServoController :: GetModule ( uint8_t Module )
{

	return Modules [ Module ];

};

void PICServoController :: PICServoEnable ( uint8_t ModuleNumber )
{

	ServerMessage * SendMessage = new ServerMessage ();

	SendMessage -> Command = PICSERVO_ENABLE_MESSAGE;
	SendMessage -> Data = ModuleNumber;

	msgQSend ( SendMessageQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_NORMAL );

};

void PICServoController :: PICServoDisable ( uint8_t ModuleNumber )
{

	ServerMessage * SendMessage = new ServerMessage ();

	SendMessage -> Command = PICSERVO_DISABLE_MESSAGE;
	SendMessage -> Data = ModuleNumber;

	msgQSend ( SendMessageQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT );

};

void PICServoController :: PICServoSetPWM ( uint8_t ModuleNumber, int16_t PWM )
{

	ServerMessage * SendMessage = new ServerMessage ();

	SetPWMMessage * SPMessage = new SetPWMMessage ();

	SPMessage -> Index = ModuleNumber;
	SPMessage -> Value = PWM;

	SendMessage -> Command = PICSERVO_SETPWM_MESSAGE;
	SendMessage -> Data = reinterpret_cast <uint32_t> ( SPMessage );

	msgQSend ( SendMessageQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_NORMAL );

};

void PICServoController :: PICServoSetPosition ( uint8_t ModuleNumber, double Position )
{

	ServerMessage * SendMessage = new ServerMessage ();

	SetPositionMessage * PMessage = new SetPositionMessage ();

	PMessage -> Index = ModuleNumber;
	PMessage -> Position = Position;

	SendMessage -> Command = PICSERVO_SETPOSITION_MESSAGE;
	SendMessage -> Data = reinterpret_cast <uint32_t> ( PMessage );

	msgQSend ( SendMessageQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_NORMAL );

};

void PICServoController :: PICServoSetPositionV ( uint8_t ModuleNumber, double Position, double Velocity )
{

	ServerMessage * SendMessage = new ServerMessage ();

	SetPositionVMessage * PVMessage = new SetPositionVMessage ();

	PVMessage -> Index = ModuleNumber;
	PVMessage -> Position = Position;
	PVMessage -> Velocity = Velocity;

	SendMessage -> Command = PICSERVO_SETPOSITIONV_MESSAGE;
	SendMessage -> Data = reinterpret_cast <uint32_t> ( PVMessage );

	msgQSend ( SendMessageQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_NORMAL );

};

void PICServoController :: PICServoSetPositionA ( uint8_t ModuleNumber, double Position, double Acceleration )
{

	ServerMessage * SendMessage = new ServerMessage ();

	SetPositionAMessage * PAMessage = new SetPositionAMessage ();

	PAMessage -> Index = ModuleNumber;
	PAMessage -> Position = Position;
	PAMessage -> Acceleration = Acceleration;

	SendMessage -> Command = PICSERVO_SETPOSITIONA_MESSAGE;
	SendMessage -> Data = reinterpret_cast <uint32_t> ( PAMessage );

	msgQSend ( SendMessageQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_NORMAL );

};

void PICServoController :: PICServoSetPositionVA ( uint8_t ModuleNumber, double Position, double Velocity, double Acceleration )
{

	ServerMessage * SendMessage = new ServerMessage ();

	SetPositionVAMessage * PVAMessage = new SetPositionVAMessage ();

	PVAMessage -> Index = ModuleNumber;
	PVAMessage -> Position = Position;
	PVAMessage -> Velocity = Velocity;
	PVAMessage -> Acceleration = Acceleration;

	SendMessage -> Command = PICSERVO_SETPOSITIONVA_MESSAGE;
	SendMessage -> Data = reinterpret_cast <uint32_t> ( PVAMessage );

	msgQSend ( SendMessageQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_NORMAL );

};

void PICServoController :: PICServoResetPosition ( uint8_t ModuleNumber )
{

	ServerMessage * SendMessage = new ServerMessage ();

	SendMessage -> Command = PICSERVO_RESETPOSITION_MESSAGE;
	SendMessage -> Data = ModuleNumber;

	msgQSend ( SendMessageQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_NORMAL );

};

void PICServoController :: PICServoSetCurrentPosition ( uint8_t ModuleNumber, double Position )
{

	ServerMessage * SendMessage = new ServerMessage ();

	SetCurrentPositionMessage * SCPMessage = new SetCurrentPositionMessage ();

	SCPMessage -> Index = ModuleNumber;
	SCPMessage -> Position = Position;

	SendMessage -> Command = PICSERVO_SETCURRENTPOSITION_MESSAGE;
	SendMessage -> Data = reinterpret_cast <uint32_t> ( SCPMessage );

	msgQSend ( SendMessageQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_NORMAL );

};

double PICServoController :: PICServoReadPosition ( uint8_t ModuleNumber )
{

	ServerMessage * SendMessage = new ServerMessage ();

	SendMessage -> Command = PICSERVO_RESETPOSITION_MESSAGE;
	SendMessage -> Data = ModuleNumber;

	semTake ( ResponseSemaphore, WAIT_FOREVER );

	msgQSend ( SendMessageQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT );

	ServerMessage * ResponseMessage = NULL;
	msgQReceive ( ReceiveMessageQueue, reinterpret_cast <char *> ( & ResponseMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER );

	semGive ( ResponseSemaphore );

	double Position = static_cast <double> ( ResponseMessage -> Data );

	delete ResponseMessage;
	return Position;

};

void PICServoController :: PICServoSetPID ( uint8_t ModuleNumber, double P, double I, double D )
{

	ServerMessage * SendMessage = new ServerMessage ();

	SetPIDMessage * SPIDMessage = new SetPIDMessage ();

	SPIDMessage -> Index = ModuleNumber;
	SPIDMessage -> P = P;
	SPIDMessage -> I = I;
	SPIDMessage -> D = D;

	SendMessage -> Command = PICSERVO_SETPID_MESSAGE;
	SendMessage -> Data = reinterpret_cast <uint32_t> ( SPIDMessage );

	msgQSend ( SendMessageQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_NORMAL );

};

void PICServoController :: PICServoSetVelocity ( uint8_t ModuleNumber, double Velocity )
{

	ServerMessage * SendMessage = new ServerMessage ();

	SetVelocityMessage * VMessage = new SetVelocityMessage ();

	VMessage -> Index = ModuleNumber;
	VMessage -> Velocity = Velocity;

	SendMessage -> Command = PICSERVO_SETVELOCITY_MESSAGE;
	SendMessage -> Data = reinterpret_cast <uint32_t> ( VMessage );

	msgQSend ( SendMessageQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_NORMAL );
};

void PICServoController :: PICServoSetVelocityA ( uint8_t ModuleNumber, double Velocity, double Acceleration )
{

	ServerMessage * SendMessage = new ServerMessage ();

	SetVelocityAMessage * VAMessage = new SetVelocityAMessage ();

	VAMessage -> Index = ModuleNumber;
	VAMessage -> Velocity = Velocity;
	VAMessage -> Acceleration = Acceleration;

	SendMessage -> Command = PICSERVO_SETVELOCITYA_MESSAGE;
	SendMessage -> Data = reinterpret_cast <uint32_t> ( VAMessage );

	msgQSend ( SendMessageQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_NORMAL );

};

void PICServoController :: PICServoCalibrateAnalog ( uint8_t ModuleNumber )
{

	if ( ! Started )
		return;

	semTake ( ModuleSemaphore, WAIT_FOREVER );

	PICServo * Module = Modules [ ModuleNumber ];

	if ( Module != NULL )
		PipeServer -> ZeroPipe ( Module -> MotorPipe );

	semGive ( ModuleSemaphore );

};

void PICServoController :: PICServoSetAnalogInverted ( uint8_t ModuleNumber, bool Inverted )
{

	if ( ! Started )
		return;

	semTake ( ModuleSemaphore, WAIT_FOREVER );

	PICServo * Module = Modules [ ModuleNumber ];

	if ( Module != NULL )
		PipeServer -> SetPipeInverted ( Module -> MotorPipe, Inverted );

	semGive ( ModuleSemaphore );

};

void PICServoController :: RunLoop ()
{

	semTake ( ModuleSemaphore, WAIT_FOREVER );

	Started = true;

	Com = new PICServoCom ();

	PipeServer = new AnalogCANJaguarPipeServer ();
	PipeServer -> Start ();

	ServerMessage * Message = NULL;
	ServerMessage * ResponseMessage = NULL;

	PICServo * Module = NULL;

	semGive ( ModuleSemaphore );

	while ( true )
	{

		semTake ( ModuleSemaphore, WAIT_FOREVER );

		if ( msgQReceive ( SendMessageQueue, reinterpret_cast <char *> ( & Message ), sizeof ( ServerMessage * ), WAIT_FOREVER ) != ERROR )
		{

			switch ( Message -> Command )
			{

			case PICSERVO_DISABLE_MESSAGE:

				semTake ( ModuleSemaphore, WAIT_FOREVER );

				Module = Modules [ Message -> Data ];
				PipeServer -> DisablePipe ( Module -> MotorPipe );

				semGive ( ModuleSemaphore );

				delete Message;

				break;

			case PICSERVO_ENABLE_MESSAGE:

				semTake ( ModuleSemaphore, WAIT_FOREVER );

				Module = Modules [ Message -> Data ];
				PipeServer -> EnablePipe ( Module -> MotorPipe );

				semGive ( ModuleSemaphore );

				delete Message;

				break;

			case PICSERVO_SETPWM_MESSAGE:

				semTake ( ModuleSemaphore, WAIT_FOREVER );

				SetPWMMessage * SPMessage = reinterpret_cast <SetPWMMessage *> ( Message -> Data );

				Module = Modules [ SPMessage -> Index ];

				Com -> SetStatusType ( Module -> StatusType );
				Com -> ModuleLoadTrajectory ( SPMessage -> Index, 0, 0, 0, SPMessage -> Value, false, false, false, true, false, false, false, true );
				Com -> ReceiveStatusPacket ();
				Com -> GetStatus ( & Module -> LastStatus );

				semGive ( ModuleSemaphore );

				delete SPMessage;
				delete Message;

				break;

			case PICSERVO_SETPOSITION_MESSAGE:

				semTake ( ModuleSemaphore, WAIT_FOREVER );

				SetPositionMessage * PMessage = reinterpret_cast <SetPositionMessage *> ( Message -> Data );

				Module = Modules [ PMessage -> Index ];

				Com -> SetStatusType ( Module -> StatusType );
				Com -> ModuleLoadTrajectory ( PMessage -> Index, static_cast <uint32_t> ( PMessage -> Position ), 0.0, 0.0, 0, true, false, false, false, true, false, false, true );
				Com -> ReceiveStatusPacket ();
				Com -> GetStatus ( & Module -> LastStatus );

				semGive ( ModuleSemaphore );

				delete PMessage;
				delete Message;

				break;

			case PICSERVO_SETPOSITIONV_MESSAGE:

				semTake ( ModuleSemaphore, WAIT_FOREVER );

				SetPositionVMessage * PVMessage = reinterpret_cast <SetPositionVMessage *> ( Message -> Data );

				Module = Modules [ PVMessage -> Index ];

				Com -> SetStatusType ( Module -> StatusType );
				Com -> ModuleLoadTrajectory ( PMessage -> Index, static_cast <uint32_t> ( PVMessage -> Position ), PVMessage -> Velocity, 0.0, 0, true, true, false, false, true, false, false, true );
				Com -> ReceiveStatusPacket ();
				Com -> GetStatus ( & Module -> LastStatus );

				semGive ( ModuleSemaphore );

				delete PVMessage;
				delete Message;

				break;

			case PICSERVO_SETPOSITIONA_MESSAGE:

				semTake ( ModuleSemaphore, WAIT_FOREVER );

				SetPositionAMessage * PAMessage = reinterpret_cast <SetPositionAMessage *> ( Message -> Data );

				Module = Modules [ PAMessage -> Index ];

				Com -> SetStatusType ( Module -> StatusType );
				Com -> ModuleLoadTrajectory ( PAMessage -> Index, static_cast <uint32_t> ( PAMessage -> Position ), 0.0, PAMessage -> Acceleration, 0, true, false, true, false, true, false, false, true );
				Com -> ReceiveStatusPacket ();
				Com -> GetStatus ( & Module -> LastStatus );

				semGive ( ModuleSemaphore );

				delete PAMessage;
				delete Message;

				break;

			case PICSERVO_SETPOSITIONVA_MESSAGE:

				semTake ( ModuleSemaphore, WAIT_FOREVER );

				SetPositionVAMessage * PAVMessage = reinterpret_cast <SetPositionVAMessage *> ( Message -> Data );

				Module = Modules [ PAVMessage -> Index ];

				Com -> SetStatusType ( Module -> StatusType );
				Com -> ModuleLoadTrajectory ( PAVMessage -> Index, static_cast <uint32_t> ( PAVMessage -> Position ), PAVMessage -> Velocity, PAVMessage -> Acceleration, 0, true, true, true, false, true, false, false, true );
				Com -> ReceiveStatusPacket ();
				Com -> GetStatus ( & Module -> LastStatus );

				semGive ( ModuleSemaphore );

				delete PAVMessage;
				delete Message;

				break;

			case PICSERVO_RESETPOSITION_MESSAGE:

				semTake ( ModuleSemaphore, WAIT_FOREVER );

				Module = Modules [ Message -> Data ];

				Com -> SetStatusType ( Module -> StatusType );
				Com -> ModuleResetPosition ( Message -> Data, false );
				Com -> ReceiveStatusPacket ();
				Com -> GetStatus ( & Module -> LastStatus );

				semGive ( ModuleSemaphore );

				delete Message;

				break;

			case PICSERVO_SETCURRENTPOSITION_MESSAGE:

				semTake ( ModuleSemaphore, WAIT_FOREVER );

				SetCurrentPositionMessage * SCPMessage = reinterpret_cast <SetCurrentPositionMessage *> ( Message -> Data );

				Module = Modules [ SCPMessage -> Index ];

				Com -> SetStatusType ( Module -> StatusType );
				Com -> ModuleOverwritePosition ( SCPMessage -> Index, static_cast <int32_t> ( SCPMessage -> Position ) );
				Com -> ReceiveStatusPacket ();
				Com -> GetStatus ( & Module -> LastStatus );

				semGive ( ModuleSemaphore );

				delete SCPMessage;
				delete Message;

				break;

			case PICSERVO_READPOSITION_MESSAGE:

				PICServoCom :: PICServoStatus_t Status;
				Com -> ModuleReadStatus ( Message -> Data, PICSERVO_STATUS_TYPE_POSITION, & Status );

				ResponseMessage = new ServerMessage ();

				ResponseMessage -> Command = PICSERVO_READPOSITION_MESSAGE;
				ResponseMessage -> Data = Status.Position;

				msgQSend ( ReceiveMessageQueue, reinterpret_cast <char *> ( & ResponseMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT );

				delete Message;

				break;

			case PICSERVO_SETVELOCITY_MESSAGE:

				semTake ( ModuleSemaphore, WAIT_FOREVER );

				SetVelocityMessage * SVMessage = reinterpret_cast <SetVelocityMessage *> ( Message -> Data );

				Module = Modules [ SVMessage -> Index ];

				Com -> SetStatusType ( Module -> StatusType );
				Com -> ModuleLoadTrajectory ( SVMessage -> Index, 0, SVMessage -> Velocity, 0, 0, false, true, false, false, true, true, false, true );
				Com -> ReceiveStatusPacket ();
				Com -> GetStatus ( & Module -> LastStatus );

				semGive ( ModuleSemaphore );

				delete SVMessage;
				delete Message;

				break;

			case PICSERVO_SETVELOCITYA_MESSAGE:

				semTake ( ModuleSemaphore, WAIT_FOREVER );

				SetVelocityAMessage * SVAMessage = reinterpret_cast <SetVelocityAMessage *> ( Message -> Data );

				Module = Modules [ SVAMessage -> Index ];

				Com -> SetStatusType ( Module -> StatusType );
				Com -> ModuleLoadTrajectory ( SVAMessage -> Index, 0, SVAMessage -> Velocity, SVAMessage -> Acceleration, 0, false, true, true, false, true, true, false, true );
				Com -> ReceiveStatusPacket ();
				Com -> GetStatus ( & Module -> LastStatus );

				semGive ( ModuleSemaphore );

				delete SVAMessage;
				delete Message;

				break;

			case PICSERVO_SETPID_MESSAGE:

				semTake ( ModuleSemaphore, WAIT_FOREVER );

				SetPIDMessage * SPIDMessage = reinterpret_cast <SetPIDMessage *> ( Message -> Data );

				Module = Modules [ SPIDMessage -> Index ];

				Com -> SetStatusType ( Module -> StatusType );
				Com -> ModuleSetMetrics ( SPIDMessage -> Index, static_cast <uint16_t> ( SPIDMessage -> P * 1024 ), static_cast <uint16_t> ( SPIDMessage -> I * 1024 ), static_cast <uint16_t> ( SPIDMessage -> D * 1024 ) );
				Com -> ReceiveStatusPacket ();
				Com -> GetStatus ( & Module -> LastStatus );

				semGive ( ModuleSemaphore );

				delete SPIDMessage;
				delete Message;

				break;

			case PICSERVO_INIT_MESSAGE:

				semTake ( ModuleSemaphore, WAIT_FOREVER );

				Module = Modules [ Message -> Data ];

				Com -> SetStatusType ( Module -> StatusType );
				Com -> ModuleSetAddress ( 0, Message -> Data, GroupAddress );
				Com -> ReceiveStatusPacket ();
				Com -> GetStatus ( & Module -> LastStatus );

				semGive ( ModuleSemaphore );

				ResponseMessage = new ServerMessage ();

				ResponseMessage -> Command = PICSERVO_INIT_MESSAGE;
				ResponseMessage -> Data = 0;

				msgQSend ( ReceiveMessageQueue, reinterpret_cast <char *> ( & ResponseMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT );

				delete Message;

				break;

			case PICSERVO_REINIT_MESSAGE:

				semTake ( ModuleSemaphore, WAIT_FOREVER );

				Module = Modules [ Message -> Data ];

				Com -> SetStatusType ( Module -> StatusType );
				Com -> ModuleStopMotor ( Message -> Data, false, true, true );
				Com -> ReceiveStatusPacket ();
				Com -> GetStatus ( & Module -> LastStatus );

				semGive ( ModuleSemaphore );

				ResponseMessage = new ServerMessage ();

				ResponseMessage -> Command = PICSERVO_REINIT_MESSAGE;
				ResponseMessage -> Data = 0;

				msgQSend ( ReceiveMessageQueue, reinterpret_cast <char *> ( & ResponseMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT );

				delete Message;

				break;

			default:

				if ( Message != NULL )
					delete Message;

				break;

			}

		}

	}

};

int PICServoController :: _StartServerTask ( PICServoController * This )
{

	This -> RunLoop ();

	return 0;

};
