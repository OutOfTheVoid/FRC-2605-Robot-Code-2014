#include "PICServoController.h"

PICServoController :: PICServoController ( uint8_t GroupAddress )
{

	Log = Logger :: GetInstance ();

	PipeServer = new AnalogCANJaguarPipeServer ();
	PipeServer -> Start ();

	this -> GroupAddress = GroupAddress;

	Modules = new PICServo * [ 256 ];

	for ( uint8_t i = 0; i != 255; i ++ )
		Modules [ i ] = NULL;

	Started = false;

	ServerTask = new Task ( "2605_PICServo_Control_Server_Task", reinterpret_cast <FUNCPTR> ( _StartServerTask ), PICSERVO_CONTROL_TASK_PRIORITY, PICSERVO_CONTROL_TASK_STACKSIZE );

	SendMessageQueue = msgQCreate ( 200, sizeof ( ServerMessage * ), MSG_Q_FIFO );

	if ( SendMessageQueue == NULL )
		Log -> Log ( Logger :: LOG_ERROR, "PICServoController Error: Couldn't allocate a message queue for asynch command loop." );

	ReceiveMessageQueue = msgQCreate ( 20, sizeof ( ServerMessage * ), MSG_Q_FIFO );

	if ( ReceiveMessageQueue == NULL )
	{

		msgQDelete ( SendMessageQueue );

		Log -> Log ( Logger :: LOG_ERROR, "PICServoController Error: Couldn't allocate a message queue for asynch command loop." );

	}

	ResponseSemaphore = semMCreate ( SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE );

	if ( ResponseSemaphore == NULL )
	{

		msgQDelete ( SendMessageQueue );
		msgQDelete ( ReceiveMessageQueue );

		Log -> Log ( Logger :: LOG_ERROR, "PICServoController Error: Couldn't create Response synchronization semaphore." );

	};

	ModuleSemaphore = semMCreate ( SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE );

	if ( ModuleSemaphore == NULL )
	{

		msgQDelete ( SendMessageQueue );
		msgQDelete ( ReceiveMessageQueue );
		semDelete ( ResponseSemaphore );

		Log -> Log ( Logger :: LOG_ERROR, "PICServoController Error: Couldn't create Module synchronization semaphore." );

	}

	if ( ! ServerTask -> Start ( reinterpret_cast <uint32_t> ( this ) ) )
	{

		msgQDelete ( SendMessageQueue );
		msgQDelete ( ReceiveMessageQueue );
		semDelete ( ResponseSemaphore );
		semDelete ( ModuleSemaphore );

		Log -> Log ( Logger :: LOG_ERROR, "PICServoController Error: Couldn't start server task." );

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

	delete PipeServer;
	delete Com;

};

void PICServoController :: AddPICServo ( uint8_t ModuleNumber, bool Initialize, CAN_ID JaguarID, uint8_t AnalogChannel )
{

	AddPICServo ( ModuleNumber, Initialize, JaguarID, AnalogChannel, SensorBase :: GetDefaultAnalogModule () );

};

void PICServoController :: AddPICServo ( uint8_t ModuleNumber, bool Initialize, CAN_ID JaguarID, uint8_t AnalogChannel, uint8_t AnalogModule )
{

	if ( ! Started )
		return;

	PICServo * Module;

	ServerMessage * ResponseMessage = NULL;

	semTake ( ModuleSemaphore, WAIT_FOREVER );
	semTake ( ResponseSemaphore, WAIT_FOREVER );

	if ( Modules [ ModuleNumber ] != NULL )
	{

		Module = Modules [ ModuleNumber ];

		PipeServer -> DisablePipe ( Module -> MotorPipe );
		PipeServer -> RemovePipe ( Module -> MotorPipe );

		new (Module) PICServo ( ModuleNumber, this, Module -> MotorPipe = PipeServer -> AddPipe ( JaguarID, AnalogChannel, AnalogModule ) );

		semTake ( ResponseSemaphore, WAIT_FOREVER );

		ServerMessage * SendMessage = new ServerMessage ();

		SendMessage -> Command = PICSERVO_REINIT_MESSAGE;
		SendMessage -> Data = ModuleNumber;

		if ( msgQSend ( SendMessageQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT ) )
			Log -> Log ( Logger :: LOG_ERROR, "msgQSend Failed in AddPICServo!\n" );

		if ( msgQReceive ( ReceiveMessageQueue, reinterpret_cast <char *> ( & ResponseMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER ) )
			Log -> Log ( Logger :: LOG_ERROR, "msgQReceive Failed in AddPICServo!\n" );

		Log -> Log ( Logger :: LOG_EVENT, "PICServo #%i Added Successfully.\n", ModuleNumber );

		delete ResponseMessage;

	}
	else
	{

		Module = new PICServo ( ModuleNumber, this, PipeServer -> AddPipe ( JaguarID, AnalogChannel, AnalogModule ) );

		Modules [ ModuleNumber ] = Module;

		if ( Initialize )
		{

			ServerMessage * SendMessage = new ServerMessage ();

			SendMessage -> Command = PICSERVO_INIT_MESSAGE;
			SendMessage -> Data = ModuleNumber;

			if ( msgQSend ( SendMessageQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT ) )
				Log -> Log ( Logger :: LOG_ERROR, "msgQSend Failed in AddPICServo!\n" );

			if ( msgQReceive ( ReceiveMessageQueue, reinterpret_cast <char *> ( & ResponseMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER ) )
				Log -> Log ( Logger :: LOG_ERROR, "msgQReceive Failed in AddPICServo!\n" );


			delete ResponseMessage;
			ResponseMessage = NULL;

		}

		ServerMessage * SendMessage = new ServerMessage ();

		SendMessage -> Command = PICSERVO_REINIT_MESSAGE;
		SendMessage -> Data = ModuleNumber;

		if ( msgQSend ( SendMessageQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT ) == ERROR )
			Log -> Log ( Logger :: LOG_ERROR, "msgQSend Failed in AddPICServo!\n" );

		if ( msgQReceive ( ReceiveMessageQueue, reinterpret_cast <char *> ( & ResponseMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER ) == ERROR )
			Log -> Log ( Logger :: LOG_ERROR, "msgQReceive Failed in AddPICServo!\n" );

		Log -> Log ( Logger :: LOG_EVENT, "PICServo #%i Added Successfully.\n", ModuleNumber );

		delete ResponseMessage;

	}

	semGive ( ResponseSemaphore );
	semGive ( ModuleSemaphore );

};

PICServo * PICServoController :: GetModule ( uint8_t Module )
{

	return Modules [ Module ];

};

void PICServoController :: PICServoEnable ( uint8_t ModuleNumber )
{

	if ( ! Started )
		return;

	ServerMessage * SendMessage = new ServerMessage ();

	SendMessage -> Command = PICSERVO_ENABLE_MESSAGE;
	SendMessage -> Data = ModuleNumber;

	msgQSend ( SendMessageQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_NORMAL );

};

void PICServoController :: PICServoDisable ( uint8_t ModuleNumber )
{

	if ( ! Started )
		return;

	ServerMessage * SendMessage = new ServerMessage ();

	SendMessage -> Command = PICSERVO_DISABLE_MESSAGE;
	SendMessage -> Data = ModuleNumber;

	msgQSend ( SendMessageQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT );

};

void PICServoController :: PICServoSetPWM ( uint8_t ModuleNumber, int16_t PWM )
{

	if ( ! Started )
		return;

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

	if ( ! Started )
		return;

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

	if ( ! Started )
		return;

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

	if ( ! Started )
		return;

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

	if ( ! Started )
		return;

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

	if ( ! Started )
		return;

	ServerMessage * SendMessage = new ServerMessage ();

	SendMessage -> Command = PICSERVO_RESETPOSITION_MESSAGE;
	SendMessage -> Data = ModuleNumber;

	msgQSend ( SendMessageQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_NORMAL );

};

void PICServoController :: PICServoSetCurrentPosition ( uint8_t ModuleNumber, double Position )
{

	if ( ! Started )
		return;

	ServerMessage * SendMessage = new ServerMessage ();

	SetCurrentPositionMessage * SCPMessage = new SetCurrentPositionMessage ();

	SCPMessage -> Index = ModuleNumber;
	SCPMessage -> Position = Position;

	SendMessage -> Command = PICSERVO_SETCURRENTPOSITION_MESSAGE;
	SendMessage -> Data = reinterpret_cast <uint32_t> ( SCPMessage );

	msgQSend ( SendMessageQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_NORMAL );

};

int32_t PICServoController :: PICServoReadPosition ( uint8_t ModuleNumber )
{

	if ( ! Started )
		return 0;

	ServerMessage * SendMessage = new ServerMessage ();

	SendMessage -> Command = PICSERVO_READPOSITION_MESSAGE;
	SendMessage -> Data = static_cast <uint32_t> ( ModuleNumber );

	semTake ( ResponseSemaphore, WAIT_FOREVER );

	msgQSend ( SendMessageQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT );

	ServerMessage * ResponseMessage = NULL;
	msgQReceive ( ReceiveMessageQueue, reinterpret_cast <char *> ( & ResponseMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER );

	semGive ( ResponseSemaphore );

	if ( ResponseMessage == NULL )
	{

		printf ( "Repsonse NULL error...\n" );
		return 0;

	}

	volatile int32_t Position = static_cast <int32_t> ( ResponseMessage -> Data );

	delete ResponseMessage;
	return Position;

};

void PICServoController :: PICServoSetPID ( uint8_t ModuleNumber, double P, double I, double D, double MaxOutput, uint32_t PositionError, uint8_t DeadbandCompensation )
{

	if ( ! Started )
		return;

	ServerMessage * SendMessage = new ServerMessage ();

	SetPIDMessage * SPIDMessage = new SetPIDMessage ();

	SPIDMessage -> Index = ModuleNumber;
	SPIDMessage -> P = P;
	SPIDMessage -> I = I;
	SPIDMessage -> D = D;
	SPIDMessage -> MaxOutput = MaxOutput;
	SPIDMessage -> PositionError = PositionError;
	SPIDMessage -> DeadbandCompensation = DeadbandCompensation;

	SendMessage -> Command = PICSERVO_SETPID_MESSAGE;
	SendMessage -> Data = reinterpret_cast <uint32_t> ( SPIDMessage );

	msgQSend ( SendMessageQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_NORMAL );

};

void PICServoController :: PICServoSetVelocity ( uint8_t ModuleNumber, double Velocity )
{

	if ( ! Started )
		return;

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

	if ( ! Started )
		return;

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

bool PICServoController :: PICServoGetLimit1 ( uint8_t ModuleNumber )
{

	if ( ! Started )
		return false;

	ServerMessage * SendMessage = new ServerMessage ();

	SendMessage -> Command = PICSERVO_GETLIMIT_MESSAGE;
	SendMessage -> Data = ModuleNumber;

	semTake ( ResponseSemaphore, WAIT_FOREVER );

	printf ( "MSGSEND\n" );

	msgQSend ( SendMessageQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT );

	ServerMessage * ResponseMessage = NULL;
	msgQReceive ( ReceiveMessageQueue, reinterpret_cast <char *> ( & ResponseMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER );

	printf ( "MSGRECV\n" );

	semGive ( ResponseSemaphore );

	if ( ResponseMessage == NULL )
	{

		printf ( "Repsonse NULL error...\n" );
		return false;

	}

	bool Switch = ( ResponseMessage -> Data & 0x01 ) != 0;

	delete ResponseMessage;

	return Switch;

};

bool PICServoController :: PICServoGetLimit2 ( uint8_t ModuleNumber )
{

	if ( ! Started )
		return false;

	ServerMessage * SendMessage = new ServerMessage ();

	SendMessage -> Command = PICSERVO_GETLIMIT_MESSAGE;
	SendMessage -> Data = ModuleNumber;

	semTake ( ResponseSemaphore, WAIT_FOREVER );

	msgQSend ( SendMessageQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT );

	ServerMessage * ResponseMessage = NULL;
	msgQReceive ( ReceiveMessageQueue, reinterpret_cast <char *> ( & ResponseMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER );

	semGive ( ResponseSemaphore );

	if ( ResponseMessage == NULL )
	{

		printf ( "Repsonse NULL error...\n" );
		return false;

	}

	bool Switch = ( ResponseMessage -> Data & 0x02 ) != 0;

	delete ResponseMessage;

	return Switch;

};

bool  PICServoController :: PICServoGetMoveDone ( uint8_t ModuleNumber )
{

	if ( ! Started )
		return false;

	ServerMessage * SendMessage = new ServerMessage ();

	SendMessage -> Command = PICSERVO_GETMOVEDONE_MESSAGE;
	SendMessage -> Data = ModuleNumber;

	semTake ( ResponseSemaphore, WAIT_FOREVER );

	msgQSend ( SendMessageQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT );

	ServerMessage * ResponseMessage = NULL;
	msgQReceive ( ReceiveMessageQueue, reinterpret_cast <char *> ( & ResponseMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER );

	semGive ( ResponseSemaphore );

	if ( ResponseMessage == NULL )
	{

		printf ( "Repsonse NULL error...\n" );
		return false;

	}

	bool Done = ResponseMessage -> Data != 0;

	delete ResponseMessage;

	return Done;

};

void PICServoController :: PICServoSetLimiting ( uint8_t ModuleNumber, bool Limiting )
{

	if ( ! Started )
		return;

	ServerMessage * SendMessage = new ServerMessage ();

	SendMessage -> Command = PICSERVO_SETLIMITING_MESSAGE;
	SendMessage -> Data = ModuleNumber | ( Limiting ? 0x100 : 0x000 );

	msgQSend ( SendMessageQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT );

};

void PICServoController :: RunLoop ()
{

	Started = true;

	Com = new PICServoCom ();

	ServerMessage * Message = NULL;
	ServerMessage * ResponseMessage = NULL;

	PICServo * Module = NULL;

	while ( true )
	{

		if ( msgQReceive ( SendMessageQueue, reinterpret_cast <char *> ( & Message ), sizeof ( ServerMessage * ), WAIT_FOREVER ) != ERROR )
		{

			switch ( Message -> Command )
			{

			case PICSERVO_DISABLE_MESSAGE:

				Log -> Log ( Logger :: LOG_DEBUG2, "PICSERVO_DISABLE\n" );

				semTake ( ModuleSemaphore, WAIT_FOREVER );

				Module = Modules [ Message -> Data ];
				PipeServer -> DisablePipe ( Module -> MotorPipe );

				semGive ( ModuleSemaphore );

				delete Message;

				break;

			case PICSERVO_ENABLE_MESSAGE:

				Log -> Log ( Logger :: LOG_DEBUG2, "PICSERVO_ENABLE\n" );

				semTake ( ModuleSemaphore, WAIT_FOREVER );

				Module = Modules [ Message -> Data ];
				PipeServer -> EnablePipe ( Module -> MotorPipe );

				semGive ( ModuleSemaphore );

				delete Message;

				break;

			case PICSERVO_SETPWM_MESSAGE:

				Log -> Log ( Logger :: LOG_DEBUG2, "PICSERVO_SETPWM\n" );

				semTake ( ModuleSemaphore, WAIT_FOREVER );

				SetPWMMessage * SPMessage = reinterpret_cast <SetPWMMessage *> ( Message -> Data );

				Module = Modules [ SPMessage -> Index ];

				Com -> SetStatusType ( Module -> StatusType );
				Com -> ModuleLoadTrajectory ( SPMessage -> Index, 0, 0, 0, SPMessage -> Value, false, false, false, true, false, false, false, true );
				Com -> ReceiveStatusPacket ();
				Com -> GetStatus ( & Module -> LastStatus );

				Module -> LastStatusTime = Timer :: GetPPCTimestamp ();

				semGive ( ModuleSemaphore );

				delete SPMessage;
				delete Message;

				break;

			case PICSERVO_SETPOSITION_MESSAGE:

				Log -> Log ( Logger :: LOG_DEBUG2, "PICSERVO_SETPOSITION\n" );

				semTake ( ModuleSemaphore, WAIT_FOREVER );

				SetPositionMessage * PMessage = reinterpret_cast <SetPositionMessage *> ( Message -> Data );

				Module = Modules [ PMessage -> Index ];

				Com -> SetStatusType ( Module -> StatusType );
				Com -> ModuleLoadTrajectory ( PMessage -> Index, static_cast <uint32_t> ( PMessage -> Position ), 0.0, 0.0, 0, true, false, false, false, true, false, false, true );
				Com -> ReceiveStatusPacket ();
				Com -> GetStatus ( & Module -> LastStatus );

				Module -> LastStatusTime = Timer :: GetPPCTimestamp ();

				semGive ( ModuleSemaphore );

				delete PMessage;
				delete Message;

				break;

			case PICSERVO_SETPOSITIONV_MESSAGE:

				Log -> Log ( Logger :: LOG_DEBUG2, "PICSERVO_SETPOSITION\n" );

				semTake ( ModuleSemaphore, WAIT_FOREVER );

				SetPositionVMessage * PVMessage = reinterpret_cast <SetPositionVMessage *> ( Message -> Data );

				Module = Modules [ PVMessage -> Index ];

				Com -> SetStatusType ( Module -> StatusType );
				Com -> ModuleLoadTrajectory ( PMessage -> Index, static_cast <uint32_t> ( PVMessage -> Position ), PVMessage -> Velocity, 0.0, 0, true, true, false, false, true, false, false, true );
				Com -> ReceiveStatusPacket ();
				Com -> GetStatus ( & Module -> LastStatus );

				Module -> LastStatusTime = Timer :: GetPPCTimestamp ();

				semGive ( ModuleSemaphore );

				delete PVMessage;
				delete Message;

				break;

			case PICSERVO_SETPOSITIONA_MESSAGE:

				Log -> Log ( Logger :: LOG_DEBUG2, "PICSERVO_SETPOSITION\n" );

				semTake ( ModuleSemaphore, WAIT_FOREVER );

				SetPositionAMessage * PAMessage = reinterpret_cast <SetPositionAMessage *> ( Message -> Data );

				Module = Modules [ PAMessage -> Index ];

				Com -> SetStatusType ( Module -> StatusType );
				Com -> ModuleLoadTrajectory ( PAMessage -> Index, static_cast <uint32_t> ( PAMessage -> Position ), 0.0, PAMessage -> Acceleration, 0, true, false, true, false, true, false, false, true );
				Com -> ReceiveStatusPacket ();
				Com -> GetStatus ( & Module -> LastStatus );

				Module -> LastStatusTime = Timer :: GetPPCTimestamp ();

				semGive ( ModuleSemaphore );

				delete PAMessage;
				delete Message;

				break;

			case PICSERVO_SETPOSITIONVA_MESSAGE:

				Log -> Log ( Logger :: LOG_DEBUG2, "PICSERVO_SETPOSITION\n" );

				semTake ( ModuleSemaphore, WAIT_FOREVER );

				SetPositionVAMessage * PAVMessage = reinterpret_cast <SetPositionVAMessage *> ( Message -> Data );

				Module = Modules [ PAVMessage -> Index ];

				Com -> SetStatusType ( Module -> StatusType );
				Com -> ModuleLoadTrajectory ( PAVMessage -> Index, static_cast <uint32_t> ( PAVMessage -> Position ), PAVMessage -> Velocity, PAVMessage -> Acceleration, 0, true, true, true, false, true, false, false, true );
				Com -> ReceiveStatusPacket ();
				Com -> GetStatus ( & Module -> LastStatus );

				Module -> LastStatusTime = Timer :: GetPPCTimestamp ();

				semGive ( ModuleSemaphore );

				delete PAVMessage;
				delete Message;

				break;

			case PICSERVO_RESETPOSITION_MESSAGE:

				Log -> Log ( Logger :: LOG_DEBUG2, "PICSERVO_SETPOSITION\n" );

				semTake ( ModuleSemaphore, WAIT_FOREVER );

				Module = Modules [ Message -> Data ];

				Com -> SetStatusType ( Module -> StatusType );
				Com -> ModuleResetPosition ( Message -> Data, false );
				Com -> ReceiveStatusPacket ();
				Com -> GetStatus ( & Module -> LastStatus );

				Module -> LastStatusTime = Timer :: GetPPCTimestamp ();

				semGive ( ModuleSemaphore );

				delete Message;

				break;

			case PICSERVO_SETCURRENTPOSITION_MESSAGE:

				Log -> Log ( Logger :: LOG_DEBUG2, "PICSERVO_SETCURRENTPOSITION\n" );

				semTake ( ModuleSemaphore, WAIT_FOREVER );

				SetCurrentPositionMessage * SCPMessage = reinterpret_cast <SetCurrentPositionMessage *> ( Message -> Data );

				Module = Modules [ SCPMessage -> Index ];

				Com -> SetStatusType ( Module -> StatusType );
				Com -> ModuleOverwritePosition ( SCPMessage -> Index, static_cast <int32_t> ( SCPMessage -> Position ) );
				Com -> ReceiveStatusPacket ();
				Com -> GetStatus ( & Module -> LastStatus );

				Module -> LastStatusTime = Timer :: GetPPCTimestamp ();

				semGive ( ModuleSemaphore );

				delete SCPMessage;
				delete Message;

				break;

			case PICSERVO_READPOSITION_MESSAGE:

				semTake ( ModuleSemaphore, WAIT_FOREVER );

				PICServoCom :: PICServoStatus_t Status;

				Com -> ModuleReadStatus ( Message -> Data, PICSERVO_STATUS_TYPE_POSITION | Module -> StatusType, & Status );
				Module -> LastStatusTime = Timer :: GetPPCTimestamp ();

				ResponseMessage = new ServerMessage ();

				ResponseMessage -> Command = PICSERVO_READPOSITION_MESSAGE;
				ResponseMessage -> Data = Status.Position;

				msgQSend ( ReceiveMessageQueue, reinterpret_cast <char *> ( & ResponseMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT );

				delete Message;

				break;

			case PICSERVO_SETVELOCITY_MESSAGE:

				Log -> Log ( Logger :: LOG_DEBUG2, "PICSERVO_SETVELOCITY\n" );

				semTake ( ModuleSemaphore, WAIT_FOREVER );

				SetVelocityMessage * SVMessage = reinterpret_cast <SetVelocityMessage *> ( Message -> Data );

				Module = Modules [ SVMessage -> Index ];

				Com -> SetStatusType ( Module -> StatusType );
				Com -> ModuleLoadTrajectory ( SVMessage -> Index, 0, SVMessage -> Velocity, 0, 0, false, true, false, false, true, true, false, true );
				Com -> ReceiveStatusPacket ();
				Com -> GetStatus ( & Module -> LastStatus );

				Module -> LastStatusTime = Timer :: GetPPCTimestamp ();

				semGive ( ModuleSemaphore );

				delete SVMessage;
				delete Message;

				break;

			case PICSERVO_SETVELOCITYA_MESSAGE:

				Log -> Log ( Logger :: LOG_DEBUG2, "PICSERVO_SETVELOCITY\n" );

				semTake ( ModuleSemaphore, WAIT_FOREVER );

				SetVelocityAMessage * SVAMessage = reinterpret_cast <SetVelocityAMessage *> ( Message -> Data );

				Module = Modules [ SVAMessage -> Index ];

				Com -> SetStatusType ( Module -> StatusType );
				Com -> ModuleLoadTrajectory ( SVAMessage -> Index, 0, SVAMessage -> Velocity, SVAMessage -> Acceleration, 0, false, true, true, false, true, true, false, true );
				Com -> ReceiveStatusPacket ();
				Com -> GetStatus ( & Module -> LastStatus );

				Module -> LastStatusTime = Timer :: GetPPCTimestamp ();

				semGive ( ModuleSemaphore );

				delete SVAMessage;
				delete Message;

				break;

			case PICSERVO_SETPID_MESSAGE:

				Log -> Log ( Logger :: LOG_DEBUG2, "PICSERVO_SETPID\n" );

				semTake ( ModuleSemaphore, WAIT_FOREVER );

				SetPIDMessage * SPIDMessage = reinterpret_cast <SetPIDMessage *> ( Message -> Data );

				Module = Modules [ SPIDMessage -> Index ];

				Com -> SetStatusType ( Module -> StatusType );
				Com -> ModuleSetMetrics ( SPIDMessage -> Index, static_cast <uint16_t> ( SPIDMessage -> P * 1024.0 ), static_cast <uint16_t> ( SPIDMessage -> I * 1024.0 ), static_cast <uint16_t> ( SPIDMessage -> D * 1024.0 ), 32767, static_cast <uint8_t> ( SPIDMessage -> MaxOutput * 255.0 ), 127, SPIDMessage -> PositionError, 1, static_cast <uint8_t> ( SPIDMessage -> DeadbandCompensation * 255.0 ) );
				Com -> ReceiveStatusPacket ();
				Com -> GetStatus ( & Module -> LastStatus );

				Module -> LastStatusTime = Timer :: GetPPCTimestamp ();

				semGive ( ModuleSemaphore );

				delete SPIDMessage;
				delete Message;

				break;

			case PICSERVO_INIT_MESSAGE:

				Log -> Log ( Logger :: LOG_DEBUG2, "PICSERVO_INIT\n" );

				Module = Modules [ Message -> Data ];

				Com -> SetStatusType ( Module -> StatusType );
				Com -> ModuleDefineStatus ( Message -> Data, Module -> StatusType );
				Com -> ReceiveStatusPacket ();

				Com -> ModuleSetAddress ( 0, Message -> Data, GroupAddress );
				Com -> ReceiveStatusPacket ();
				Com -> GetStatus ( & Module -> LastStatus );

				Com -> ModuleClearStatus ( Message -> Data );
				Com -> ReceiveStatusPacket ();

				Com -> ModuleStopMotor ( Message -> Data, false, true, true );
				Com -> ReceiveStatusPacket ();
				Com -> GetStatus ( & Module -> LastStatus );

				Module -> LastStatusTime = Timer :: GetPPCTimestamp ();

				ResponseMessage = new ServerMessage ();

				ResponseMessage -> Command = PICSERVO_INIT_MESSAGE;
				ResponseMessage -> Data = 0;

				msgQSend ( ReceiveMessageQueue, reinterpret_cast <char *> ( & ResponseMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT );

				delete Message;

				break;

			case PICSERVO_REINIT_MESSAGE:

				Log -> Log ( Logger :: LOG_DEBUG2, "PICSERVO_REINIT\n" );

				Module = Modules [ Message -> Data ];

				Com -> SetStatusType ( Module -> StatusType );
				Com -> ModuleDefineStatus ( Message -> Data, Module -> StatusType );
				Com -> ReceiveStatusPacket ();

				Log -> Log ( Logger :: LOG_DEBUG, "Clearing module status...\n" );

				Com -> ModuleClearStatus ( Message -> Data );
				Com -> ReceiveStatusPacket ();

				Log -> Log ( Logger :: LOG_DEBUG, "Stopping module motor...\n" );

				Com -> ModuleStopMotor ( Message -> Data, false, true, true );
				Com -> ReceiveStatusPacket ();
				Com -> GetStatus ( & Module -> LastStatus );

				Log -> Log ( Logger :: LOG_DEBUG, "Finished reinitializing module.\n" );

				Module -> LastStatusTime = Timer :: GetPPCTimestamp ();

				ResponseMessage = new ServerMessage ();

				ResponseMessage -> Command = PICSERVO_REINIT_MESSAGE;
				ResponseMessage -> Data = 0;

				msgQSend ( ReceiveMessageQueue, reinterpret_cast <char *> ( & ResponseMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT );

				delete Message;

				break;

			case PICSERVO_GETLIMIT_MESSAGE:

				semTake ( ModuleSemaphore, WAIT_FOREVER );

				Module = Modules [ Message -> Data ];

				Com -> SetStatusType ( Module -> StatusType );
				Com -> ModuleGetStatus ( Message -> Data );
				Com -> ReceiveStatusPacket ();
				Com -> ModuleGetStatus ( Message -> Data );
				Com -> ReceiveStatusPacket ();
				Com -> GetStatus ( & Module -> LastStatus );

				Module -> LastStatusTime = Timer :: GetPPCTimestamp ();

				ResponseMessage = new ServerMessage ();

				ResponseMessage -> Command = PICSERVO_GETLIMIT_MESSAGE;
				ResponseMessage -> Data = ( ( Module -> LastStatus.StandardFlags & 0x20 ) ? 0x01 : 0x00 ) | ( ( Module -> LastStatus.StandardFlags & 0x40 ) ? 0x02 : 0x00 );

				msgQSend ( ReceiveMessageQueue, reinterpret_cast <char *> ( & ResponseMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT );

				semGive ( ModuleSemaphore );

				delete Message;

				break;

			case PICSERVO_GETMOVEDONE_MESSAGE:

				semTake ( ModuleSemaphore, WAIT_FOREVER );

				Module = Modules [ Message -> Data ];

				Com -> SetStatusType ( Module -> StatusType );
				Com -> ModuleGetStatus ( Message -> Data );
				Com -> ReceiveStatusPacket ();
				Com -> GetStatus ( & Module -> LastStatus );
				
				Module -> LastStatusTime = Timer :: GetPPCTimestamp ();

				ResponseMessage = new ServerMessage ();

				ResponseMessage -> Command = PICSERVO_GETLIMIT_MESSAGE;
				ResponseMessage -> Data = Module -> LastStatus.StandardFlags & 0x01;

				msgQSend ( ReceiveMessageQueue, reinterpret_cast <char *> ( & ResponseMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT );

				semGive ( ModuleSemaphore );

				delete Message;

				break;

			case PICSERVO_SETLIMITING_MESSAGE:

				semTake ( ModuleSemaphore, WAIT_FOREVER );

				Module = Modules [ Message -> Data ];

				Com -> SetStatusType ( Module -> StatusType );
				Com -> ModuleIOControl ( ( ( Message -> Data ) & 0xFF ), ( ( ( Message -> Data ) & 0x100 ) != 0 ), false );
				Com -> ReceiveStatusPacket ();
				Com -> GetStatus ( & Module -> LastStatus );

				Module -> LastStatusTime = Timer :: GetPPCTimestamp ();

				semGive ( ModuleSemaphore );

				delete Message;

				break;

			default:

				printf ( "Unhandled command!\n" );

				if ( Message != NULL )
					delete Message;

				break;

			}

		}
		else
			printf ( "Runloop Error!\n" );

	}

};

int PICServoController :: _StartServerTask ( PICServoController * This )
{

	This -> RunLoop ();

	return 0;

};
