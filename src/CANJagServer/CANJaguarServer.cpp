#include "CANJaguarServer.h"

/*
* Copyright (C) 2014 Liam Taylor
* FRC Team Sehome Semonsters 2605
*/

/*
* Note: I realize this code is a bit allocation-heavy. It was the quickest way i though of to pass a constant-length message through VxWorks's MessageQueues.
* I may update it to use a copied structure in the future, or have some sort of FIFO/Ringbuffer allocation system. I'm not sure whether it's more overhead
* to pass a large message through the Queue or just pass a pointer and deal with allocation/deallocation, but one nice thing is that i don't have to worry
* about alignment or unions with this approach. Anyway, it works for now, and i'm getting oh so tired of dealing with c++.
*/

/**
* Constructor
*
* @param DoBrownOutCheck Whether or not to peridoically check if any Jaguars on the List have browned-out.
* @param BrownOutCheckInterval How much time should pass between each successive brown-out check. (Set this higher if brown-outs aren't a common problem for you.)
* @param CANBusUpdateInterval Minimum time in between CANBus Updates. ( Set this higher if you're canbus is complaining, as can be a common problem with serial-CAN.)
* @param CommandTimeout How many system ticks to lock a command waiting on the message queue to have space.
* @param ParseTimeout How many system ticks to lock the message loop while no messages are queued before moving on to Brown-out detection or re-trying.
*/
CANJaguarServer :: CANJaguarServer ( bool DoBrownOutCheck, double BrownOutCheckInterval, double CANBusUpdateInterval, uint32_t CommandTimeout, uint32_t ParseTimeout )
{

	JagCheckInterval = BrownOutCheckInterval;
	CANUpdateInterval = CANBusUpdateInterval;
	CheckJags = DoBrownOutCheck;
	ParseWait = ParseTimeout;
	CommandWait = CommandTimeout;

	Running = false;

	// Server task. _StartServerTask calls this -> RunLoop.
	ServerTask = new Task ( "2605_CANJaguarServer_Task", (FUNCPTR) & _StartServerTask, CANJAGSERVER_PRIORITY, CANJAGSERVER_STACKSIZE );

	// Array for Server Jaguar Information structures.
	Jags = new Vector <ServerCanJagInfo> (); 

	Log = Logger :: GetInstance ();

};

/**
* Destructor
*/
CANJaguarServer :: ~CANJaguarServer ()
{

	Stop ();

	delete ServerTask;
	delete Jags;

};

/**
* Set the message loop receive timout.
*
* @param ParseTimeout How many system ticks to lock the message loop while no messages are queued before moving on to Brown-out detection or re-trying.
*/
void CANJaguarServer :: SetParseMessageTimeout ( uint32_t ParseTimeout )
{

	ParseWait = ParseTimeout;

};

/**
* Set the command send timeout.
*
* CommandTimeout How many system ticks to lock a command waiting on the message queue to have space.
*/
void CANJaguarServer :: SetCommandMessageTimeout ( uint32_t CommandTimeout )
{

	CommandWait = CommandTimeout;

};

/**
* Enable or disable brown-Out betection.
*
* @param DoBrownOutCheck Whether or not to enable brown-out checking.
*/
void CANJaguarServer :: SetBrownOutCheckEnabled ( bool DoBrownOutCheck )
{

	CheckJags = DoBrownOutCheck;

};

/**
* Set the time interval between server messages to check for brown-outs.
*
* @param Interval Interval time in seconds.
*/
void CANJaguarServer :: SetJagCheckInterval ( double Interval )
{

	// Possible race condition ignored, due to only being used for conditional comparison.
	JagCheckInterval = Interval;

};

/**
* Set the minimum time interval allowed between CAN-BUS Updates. Useful if you need to limit CAN-bandwidth. (For example if you're using the serial-can bridge.)
*
* @param Interval Interval time in seconds.
*/
void CANJaguarServer :: SetCANBusUpdateInterval ( double Interval )
{

	// Possible race condition ignored, due to only being used for conditional comparison.
	CANUpdateInterval = Interval;

};

/**
* Start the server. 
*
* You need to call this before anything that passes messages.
*/
bool CANJaguarServer :: Start ()
{

	Log -> Log ( Logger :: LOG_DEBUG, "CANJaguarServer STARTING...\n" );

	// Message Send Queue - A cross-thread command queue to direct the server thread.
	MessageSendQueue = msgQCreate ( CANJAGSERVER_MESSAGEQUEUE_LENGTH, sizeof ( CANJagServerMessage * ), MSG_Q_FIFO );

	// Handle error
	if ( MessageSendQueue == NULL )
		return false;

	// Message Response Queue - A cross-thread response queue to get the server thread's responses.
	MessageReceiveQueue = msgQCreate ( 10, sizeof ( CANJagServerMessage * ), MSG_Q_FIFO );

	// Handle Error
	if ( MessageReceiveQueue == NULL )
	{

		msgQDelete ( MessageSendQueue );
		MessageSendQueue = NULL;

	}

	// Response Semaphore - Mutex primitive used to ensure that commands which require an immediate response do not encounter a semi-race condition on the response.
	ResponseSemaphore = semMCreate ( SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE );

	// Handle error
	if ( ResponseSemaphore == NULL )
	{

		msgQDelete ( MessageSendQueue );
		msgQDelete ( MessageReceiveQueue );

		MessageSendQueue = NULL;
		MessageReceiveQueue = NULL;

		return false;

	}

	// Start Task, Handle error.
	if ( ! ServerTask -> Start ( (uint32_t) this ) )
	{

		msgQDelete ( MessageSendQueue );
		msgQDelete ( MessageReceiveQueue );
		semDelete ( ResponseSemaphore );

		MessageSendQueue = NULL;
		MessageReceiveQueue = NULL;
		ResponseSemaphore = NULL;

		return false;

	}

	Log -> Log ( Logger :: LOG_DEBUG, "CANJaguarServer STARTED!\n" );

	// Success!
	Running = true;
	SendError = false;

	return true;

};

/**
* Stop the server. 
*
* (Invalidates all jaguars. Do not allow a wrapper class to persist beyond this call.)
*/
void CANJaguarServer :: Stop ()
{

	Log -> Log ( Logger :: LOG_DEBUG, "CANJaguarServer STOPPING...\n" );

	// Make sure no command call is waiting on a response. Protects the commanding thread from a deadlock.
	semTake ( ResponseSemaphore, WAIT_FOREVER );

	ServerTask -> Stop ();

	semGive ( ResponseSemaphore );

	// Are there still messages in the queue?
	bool DeQueueSuccessful = true;

	// Get rid of them.
	while  ( DeQueueSuccessful )
	{

		// Message pointer.
		CANJagServerMessage * Message = NULL;

		// Receive pointer to message.
		DeQueueSuccessful = ( msgQReceive ( MessageSendQueue, reinterpret_cast <char *> ( & Message ), sizeof ( CANJagServerMessage * ), 0 ) != ERROR );

		if ( Message != NULL && DeQueueSuccessful )
		{

			// Correctly de-allocate messages.
			switch ( Message -> Command )
			{

				case SEND_MESSAGE_NOP:
				case SEND_MESSAGE_JAG_DISABLE:
				case SEND_MESSAGE_JAG_REMOVE:

					delete Message;

					break;

				case SEND_MESSAGE_JAG_ENABLE:

					{

						EnableCANJagMessage * EJMessage = reinterpret_cast <EnableCANJagMessage *> ( Message -> Data );

						if ( EJMessage != NULL )
							delete EJMessage;

					}

				case SEND_MESSAGE_JAG_SET:

					{
						
						SetCANJagMessage * SJMessage = reinterpret_cast <SetCANJagMessage *> ( Message -> Data );

						if ( SJMessage != NULL )
							delete SJMessage;

					}

					delete Message;
					break;

				case SEND_MESSAGE_JAG_ADD:

					{

						AddCANJagMessage * AJMessage = reinterpret_cast <AddCANJagMessage *> ( Message -> Data );

						if ( AJMessage != NULL )
							delete AJMessage;
					
					}
						
					delete Message;
					break;

				case SEND_MESSAGE_JAG_CONFIG:
					
					{

						ConfigCANJagMessage * CJMessage = reinterpret_cast <ConfigCANJagMessage *> ( Message -> Data );

						if ( CJMessage != NULL )
							delete CJMessage;

					}

					delete Message;
					break;

				default:

					delete Message;
					break;

			}


		}

	}

	// Destroy queues and semaphore

	msgQDelete ( MessageSendQueue );
	msgQDelete ( MessageReceiveQueue );
	semDelete ( ResponseSemaphore );

	MessageSendQueue = NULL;
	MessageReceiveQueue = NULL;
	ResponseSemaphore = NULL;

	Running = false;

	Log -> Log ( Logger :: LOG_DEBUG, "CANJaguarServer STOPPED!\n" );

};

/**
* Determine whether an error occured during the last send.
*/
bool CANJaguarServer :: CheckSendError ()
{

	return SendError;

}

/**
* Clear the SendError flag.
*/
void CANJaguarServer :: ClearSendError ()
{

	SendError = false;

};

/** 
* Disable a jaguar.
*
* @param ID Controller ID on the CAN-Bus.
*/
void CANJaguarServer :: DisableJag ( CAN_ID ID )
{

	CANJagServerMessage * Message = new CANJagServerMessage ();
	
	Message -> Command = SEND_MESSAGE_JAG_DISABLE;
	Message -> Data = static_cast <uint32_t> ( ID );

	SendError = ( msgQSend ( MessageSendQueue, reinterpret_cast <char *> ( & Message ), sizeof ( CANJagServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT ) == ERROR );

};

/**
* Enable a jaguar.
*
* @param ID Controller ID on the CAN-Bus.
*/
void CANJaguarServer :: EnableJag ( CAN_ID ID, double EncoderInitialPosition )
{

	CANJagServerMessage * Message = new CANJagServerMessage ();

	EnableCANJagMessage * EJMessage = new EnableCANJagMessage ();

	EJMessage -> EncoderInitialPosition = EncoderInitialPosition;
	EJMessage -> ID = ID;
	
	Message -> Command = SEND_MESSAGE_JAG_ENABLE;
	Message -> Data = reinterpret_cast <uint32_t> ( EJMessage );

	SendError = ( msgQSend ( MessageSendQueue, reinterpret_cast <char *> (  & Message ), sizeof ( CANJagServerMessage * ), CommandWait, MSG_PRI_NORMAL ) == ERROR );

};

/**
* Calls Set() on a Jaguar. 
*
* @param ID Controller ID on the CAN-Bus.
* @param Speed What speed to set the controller to.
* @param SyncGroup The SyncGroup to add this Set () to.
*/
void CANJaguarServer :: SetJag ( CAN_ID ID, float Speed, uint8_t SyncGroup )
{

	CANJagServerMessage * Message = new CANJagServerMessage ();

	SetCANJagMessage * SJMessage = new SetCANJagMessage ();

	SJMessage -> ID = ID;
	SJMessage -> Speed = Speed;
	SJMessage -> SyncGroup = SyncGroup;

	Message -> Command = SEND_MESSAGE_JAG_SET;
	Message -> Data = reinterpret_cast <uint32_t> ( SJMessage );

	SendError = ( msgQSend ( MessageSendQueue, reinterpret_cast <char *> ( & Message ), sizeof ( CANJagServerMessage * ), CommandWait, MSG_PRI_NORMAL ) == ERROR );

};

/**
* Adds a Jaguar to the Server's list.
*
* @param ID Controller ID on the CAN-Bus.
* @param Speed What speed to set the controller to.
* @param SyncGroup The SyncGroup to add this Set () to.
*/
void CANJaguarServer :: AddJag ( CAN_ID ID, CANJagConfigInfo Configuration )
{

	CANJagServerMessage * Message = new CANJagServerMessage ();
	
	AddCANJagMessage * AJMessage = new AddCANJagMessage ();

	AJMessage -> ID = ID;
	AJMessage -> Config = Configuration;

	Message -> Command = SEND_MESSAGE_JAG_ADD;
	Message -> Data = reinterpret_cast <uint32_t> ( AJMessage );

	SendError = ( msgQSend ( MessageSendQueue, reinterpret_cast <char *> ( & Message ), sizeof ( CANJagServerMessage * ), CommandWait, MSG_PRI_NORMAL ) == ERROR );

};

/**
* Configures a Jaguar.
*
* @param ID Controller ID on the CAN-Bus.
* @param Configuration Configuration Information.
*/
void CANJaguarServer :: ConfigJag ( CAN_ID ID, CANJagConfigInfo Configuration )
{

	CANJagServerMessage * Message = new CANJagServerMessage ();
	
	ConfigCANJagMessage * CJMessage = new AddCANJagMessage ();

	CJMessage -> ID = ID;
	CJMessage -> Config = Configuration;

	Message -> Command = SEND_MESSAGE_JAG_CONFIG;
	Message -> Data = reinterpret_cast <uint32_t> ( CJMessage );

	SendError = ( msgQSend ( MessageSendQueue, reinterpret_cast <char *> ( & Message ), sizeof ( CANJagServerMessage * ), CommandWait, MSG_PRI_NORMAL ) == ERROR );

};

//WARNING: I'm not absolutely sure i've fixed this one.
/**
* Gets the speed value of a Jaguar.
*
* This function locks if another Get* method is being called concurrently.
*
* @param ID Controller ID on the CAN-Bus.
*/
float CANJaguarServer :: GetJag ( CAN_ID ID )
{

	// Due to needing a response from the server thread, we must aquire the ResponseSemaphore so it doesn't preumpt a currently operating JAG_GET*
	semTake ( ResponseSemaphore, WAIT_FOREVER );

	CANJagServerMessage * SendMessage = new CANJagServerMessage ();
	
	SendMessage -> Command = SEND_MESSAGE_JAG_GET;
	SendMessage -> Data = static_cast <uint32_t> ( ID );

	// In order to return quickly, due to the locking nature of Get* functions, we preumpt any pending commands.
	SendError = ( msgQSend ( MessageSendQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( CANJagServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT ) == ERROR );

	if ( SendError )
	{

		semGive ( ResponseSemaphore );
		return 0;
	
	}

	volatile CANJagServerMessage * ReceiveMessage;

	// Receive message. ( It's imperative that we receive the result due to the preumptive nature of the call. )
	if ( msgQReceive ( MessageReceiveQueue, reinterpret_cast <char *> ( & ReceiveMessage ), sizeof ( CANJagServerMessage * ), WAIT_FOREVER ) == ERROR )
	{

		semGive ( ResponseSemaphore );
		return 0;
	
	}

	// We have our response, no longer necessary to lock the ResponseSemaphore
	semGive ( ResponseSemaphore );

	if ( ReceiveMessage == NULL )
		return 0;

	// Parse and check response message. Not sure how i'm going to handle this if something inconsistent ends up happening. Could be a response-locked semaphore, but then there's no point in using messaging.
	GetCANJagMessage * GMessage = (GetCANJagMessage *) ReceiveMessage -> Data;

	if ( GMessage != NULL )
	{

		// Acutally a response from JAG_GET?
		if ( ReceiveMessage -> Command == SEND_MESSAGE_JAG_GET )
		{

			// Proper Jaguar message?
			if ( GMessage -> ID == ID )
			{

				double val = GMessage -> Value;

				delete GMessage;
				delete ReceiveMessage;

				return val;

			}
			
			delete GMessage;
			delete ReceiveMessage;

			return 0;

		}

	}
	
	delete ReceiveMessage;
	return 0;

};

/**
* Get the jaguar's position
*
* @param ID The CAN id of the Jaguar.
**/

float CANJaguarServer :: GetJagPosition ( CAN_ID ID )
{

	// Due to needing a response from the server thread, we must aquire the ResponseSemaphore so it doesn't preumpt a currently operating JAG_GET*
	semTake ( ResponseSemaphore, WAIT_FOREVER );

	CANJagServerMessage * SendMessage = new CANJagServerMessage ();
	
	SendMessage -> Command = SEND_MESSAGE_JAG_GET_POSITION;
	SendMessage -> Data = static_cast <uint32_t> ( ID );

	// In order to return quickly, due to the locking nature of Get* functions, we preumpt any pending commands.
	SendError = ( msgQSend ( MessageSendQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( CANJagServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT ) == ERROR );

	if ( SendError )
	{

		semGive ( ResponseSemaphore );
		return 0;
	
	}

	volatile CANJagServerMessage * ReceiveMessage;

	// Receive message. ( It's imperative that we receive the result due to the preumptive nature of the call. )
	if ( msgQReceive ( MessageReceiveQueue, reinterpret_cast <char *> ( & ReceiveMessage ), sizeof ( CANJagServerMessage * ), WAIT_FOREVER ) == ERROR )
	{

		semGive ( ResponseSemaphore );
		return 0;
	
	}

	// We have our response, no longer necessary to lock the ResponseSemaphore
	semGive ( ResponseSemaphore );

	if ( ReceiveMessage == NULL )
		return 0;

	// Parse and check response message. Not sure how i'm going to handle this if something inconsistent ends up happening. Could be a response-locked semaphore, but then there's no point in using messaging.
	GetCANJagPositionMessage * GMessage = (GetCANJagPositionMessage *) ReceiveMessage -> Data;

	if ( GMessage != NULL )
	{

		// Acutally a response from JAG_GET?
		if ( ReceiveMessage -> Command == SEND_MESSAGE_JAG_GET_POSITION )
		{

			// Proper Jaguar message?
			if ( GMessage -> ID == ID )
			{

				double val = GMessage -> Value;

				delete GMessage;
				delete ReceiveMessage;

				return val;

			}
			
			delete GMessage;
			delete ReceiveMessage;

			return 0;

		}

	}
	
	delete ReceiveMessage;
	return 0;

};

/**
* Get the Jaguar's bus voltage.
*
* @param ID The CAN id of the Jaguar.
*/
float CANJaguarServer :: GetJagBusVoltage ( CAN_ID ID )
{

	// Due to needing a response from the server thread, we must aquire the ResponseSemaphore so it doesn't preumpt a currently operating JAG_GET
	semTake ( ResponseSemaphore, WAIT_FOREVER );

	CANJagServerMessage * SendMessage = new CANJagServerMessage ();

	SendMessage -> Command = SEND_MESSAGE_JAG_GET_BUS_VOLTAGE;
	SendMessage -> Data = static_cast <uint32_t> ( ID );

	// In order to return quickly, due to the locking nature of Get* functions, we preumpt any pending commands.
	SendError = ( msgQSend ( MessageSendQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( CANJagServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT ) == ERROR );

	if ( SendError )
	{

		semGive ( ResponseSemaphore );
		return 0;
	
	}

	volatile CANJagServerMessage * ReceiveMessage;

	// Receive message. ( It's imperative that we receive the result due to the preumptive nature of the call. )
	if ( msgQReceive ( MessageReceiveQueue, reinterpret_cast <char *> ( & ReceiveMessage ), sizeof ( CANJagServerMessage * ), WAIT_FOREVER ) == ERROR )
	{

		semGive ( ResponseSemaphore );
		return 0;
	
	}

	// We have our response, no longer necessary to lock the ResponseSemaphore
	semGive ( ResponseSemaphore );

	if ( ReceiveMessage == NULL )
		return 0;

	// Parse and check response message. Not sure how i'm going to handle this if something inconsistent ends up happening. Could be a response-locked semaphore, but then there's no point in using messaging.
	GetCANJagBusVoltageMessage * GBVMessage = reinterpret_cast <GetCANJagBusVoltageMessage *> ( ReceiveMessage -> Data );

	if ( GBVMessage != NULL )
	{

		// Acutally a response from JAG_GET_BUS_VOLTAGE?
		if ( ReceiveMessage -> Command == SEND_MESSAGE_JAG_GET_BUS_VOLTAGE )
		{

			// Proper Jaguar message?
			if ( GBVMessage -> ID == ID )
			{

				float val = GBVMessage -> Value;

				delete GBVMessage;
				delete ReceiveMessage;

				return val;

			}
			
			delete GBVMessage;
			delete ReceiveMessage;

			return 0;

		}

		delete GBVMessage;

	}

	delete ReceiveMessage;
	return 0;

};

/**
* Get the output voltage of a Jaguar.
*
* @param The CAN id of the Jaguar.
*/
float CANJaguarServer :: GetJagOutputVoltage ( CAN_ID ID )
{

	// Due to needing a response from the server thread, we must aquire the ResponseSemaphore so it doesn't preumpt a currently operating JAG_GET
	semTake ( ResponseSemaphore, WAIT_FOREVER );

	CANJagServerMessage * SendMessage = new CANJagServerMessage ();

	SendMessage -> Command = SEND_MESSAGE_JAG_GET_OUTPUT_VOLTAGE;
	SendMessage -> Data = static_cast <uint32_t> ( ID );

	// In order to return quickly, due to the locking nature of Get* functions, we preumpt any pending commands.
	SendError = ( msgQSend ( MessageSendQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( CANJagServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT ) == ERROR );

	if ( SendError )
	{

		semGive ( ResponseSemaphore );
		return 0;
	
	}

	volatile CANJagServerMessage * ReceiveMessage;

	// Receive message. ( It's imperative that we receive the result due to the preumptive nature of the call. )
	if ( msgQReceive ( MessageReceiveQueue, reinterpret_cast <char *> ( & ReceiveMessage ), sizeof ( CANJagServerMessage * ), WAIT_FOREVER ) == ERROR )
	{

		semGive ( ResponseSemaphore );
		return 0;
	
	}

	// We have our response, no longer necessary to lock the ResponseSemaphore
	semGive ( ResponseSemaphore );

	if ( ReceiveMessage == NULL )
		return 0;

	// Parse and check response message. Not sure how i'm going to handle this if something inconsistent ends up happening. Could be a response-locked semaphore, but then there's no point in using messaging.
	GetCANJagOutputVoltageMessage * GOVMessage = reinterpret_cast <GetCANJagOutputVoltageMessage *> ( ReceiveMessage -> Data );

	if ( GOVMessage != NULL )
	{

		// Acutally a response from JAG_GET_BUS_VOLTAGE?
		if ( ReceiveMessage -> Command == SEND_MESSAGE_JAG_GET_OUTPUT_VOLTAGE )
		{

			// Proper Jaguar message?
			if ( GOVMessage -> ID == ID )
			{

				float val = GOVMessage -> Value;

				delete GOVMessage;
				delete ReceiveMessage;

				return val;

			}

		}

		delete GOVMessage;

	}

	delete ReceiveMessage;
	return 0;

};

float CANJaguarServer :: GetJagOutputCurrent ( CAN_ID ID )
{

	// Due to needing a response from the server thread, we must aquire the ResponseSemaphore so it doesn't preumpt a currently operating JAG_GET
	semTake ( ResponseSemaphore, WAIT_FOREVER );

	CANJagServerMessage * SendMessage = new CANJagServerMessage ();

	SendMessage -> Command = SEND_MESSAGE_JAG_GET_OUTPUT_CURRENT;
	SendMessage -> Data = static_cast <uint32_t> ( ID );

	// In order to return quickly, due to the locking nature of Get* functions, we preumpt any pending commands.
	SendError = ( msgQSend ( MessageSendQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( CANJagServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT ) == ERROR );

	if ( SendError )
	{

		semGive ( ResponseSemaphore );
		return 0;
	
	}

	volatile CANJagServerMessage * ReceiveMessage;

	// Receive message. ( It's imperative that we receive the result due to the preumptive nature of the call. )
	if ( msgQReceive ( MessageReceiveQueue, reinterpret_cast <char *> ( & ReceiveMessage ), sizeof ( CANJagServerMessage * ), WAIT_FOREVER ) == ERROR )
	{

		semGive ( ResponseSemaphore );
		return 0;
	
	}

	// We have our response, no longer necessary to lock the ResponseSemaphore
	semGive ( ResponseSemaphore );

	if ( ReceiveMessage == NULL )
		return 0;

	// Parse and check response message. Not sure how i'm going to handle this if something inconsistent ends up happening. Could be a response-locked semaphore, but then there's no point in using messaging.
	GetCANJagOutputCurrentMessage * GOCMessage = reinterpret_cast <GetCANJagOutputCurrentMessage *> ( ReceiveMessage -> Data );

	if ( GOCMessage != NULL )
	{

		// Acutally a response from JAG_GET_BUS_CURRENT?
		if ( ReceiveMessage -> Command == SEND_MESSAGE_JAG_GET_OUTPUT_CURRENT )
		{

			// Proper Jaguar message?
			if ( GOCMessage -> ID == ID )
			{

				float val = GOCMessage -> Value;

				delete GOCMessage;
				delete ReceiveMessage;

				return val;

			}

		}

		delete GOCMessage;

	}

	delete ReceiveMessage;
	return 0;

};

/**
* Updates the SyncGroup of Jaguars.
*
* @param SyncGroup The mask of sync groups to update pending Set () operations on.
*/
void CANJaguarServer :: UpdateJagSyncGroup ( uint8_t SyncGroup )
{

	CANJagServerMessage * Message = new CANJagServerMessage ();
	
	Message -> Command = SEND_MESSAGE_JAG_UPDATE_SYNC_GROUP;
	Message -> Data = static_cast <uint32_t> ( SyncGroup );

	SendError = ( msgQSend ( MessageSendQueue, reinterpret_cast <char *> ( & Message ), sizeof ( CANJagServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT ) == ERROR );

};

/**
* Removes a Jaguar from the Server.
*
* @param ID Controller ID on the CAN-Bus.
*/
void CANJaguarServer :: RemoveJag ( CAN_ID ID )
{

	CANJagServerMessage * Message = new CANJagServerMessage ();
	
	Message -> Command = SEND_MESSAGE_JAG_REMOVE;
	Message -> Data = static_cast <uint32_t> ( ID );

	SendError = ( msgQSend ( MessageSendQueue, reinterpret_cast <char *> ( & Message ), sizeof ( CANJagServerMessage * ), CommandWait, MSG_PRI_NORMAL ) == ERROR );

};

void CANJaguarServer :: RunLoop ()
{

	Log -> Log ( Logger :: LOG_DEBUG, "CANJaguarServer RUNLOOP ENTERED!\n" );

	if ( MessageSendQueue == NULL )
		return;

	if ( Jags == NULL )
		return;

	uint32_t JagLoopCounter = 0;
	CANJagServerMessage * Message;

	// Possibly used case independant variables.
	CAN_ID ID;
	bool Conflict = false;
	CANJagServerMessage * SendMessage;

	Log -> Log ( Logger :: LOG_DEBUG, "CANJaguarServer TIME_MARK\n" );

	double PreJagCheckTime = Timer :: GetPPCTimestamp () - JagCheckInterval;
	double PreCANCheckTime = Timer :: GetPPCTimestamp () - CANUpdateInterval;

	Log -> Log ( Logger :: LOG_DEBUG, "CANJaguarServer ENTERING MESSAGE LOOP\n" );

	while ( true )
	{

		Log -> Log ( Logger :: LOG_DEBUG3, "CANJaguarServer MSG_GET\n" );

		if ( msgQReceive ( MessageSendQueue, reinterpret_cast <char *> ( & Message ), sizeof ( CANJagServerMessage * ), ParseWait ) != ERROR )
		{

			Log -> Log ( Logger :: LOG_DEBUG3, "CANJaguarServer MESSAGE RECEIVED:\n" );

			// CAN-Bus Update speed protection.
			if ( CANUpdateInterval != 0 )
			{

				double CurrentCANUpdateTime = Timer :: GetPPCTimestamp ();
				double CANUpdateDelta = PreCANCheckTime - CurrentCANUpdateTime;

				if ( CANUpdateDelta < CANUpdateInterval )
				{

					Wait ( CANUpdateDelta - CANUpdateInterval );
					PreCANCheckTime = Timer :: GetPPCTimestamp ();

				}

			}

			if ( Message != NULL )
			{

				switch ( Message -> Command )
				{

					// No-Op
					case SEND_MESSAGE_NOP:

						delete Message;

						break;

					// Disable Jaguar
					case SEND_MESSAGE_JAG_DISABLE:

						// Which Jaguar?
						ID = static_cast <CAN_ID> ( Message -> Data );

						// Find it and disable it.
						for ( uint32_t i = 0; i < Jags -> GetLength (); i ++ )
						{

							ServerCANJagInfo JagInfo = ( * Jags ) [ i ];

							if ( JagInfo.ID == ID )
							{

								JagInfo.Jag -> Set ( 0 );
								JagInfo.Jag -> DisableControl ();

								break;

							}	

						}

						delete Message;

						break;

					// Enable Jaguar
					case SEND_MESSAGE_JAG_ENABLE:

						// Which Jaguar?
						EnableCANJagMessage * EJMessage = reinterpret_cast <EnableCANJagMessage *> ( Message -> Data );

						if ( EJMessage == NULL )
						{

							delete Message;
							break;

						}

						// Find it and enable it.
						for ( uint32_t i = 0; i < Jags -> GetLength (); i ++ )
						{

							ServerCANJagInfo JagInfo = ( * Jags ) [ i ];

							if ( JagInfo.ID == EJMessage -> ID )
							{

								JagInfo.Jag -> EnableControl ( EJMessage -> EncoderInitialPosition );

								break;

							}

						}

						delete EJMessage;
						delete Message;

						break;

				// Set Jaguar
					case SEND_MESSAGE_JAG_SET:

						// Retreive JAG_SET message.
						SetCANJagMessage * SJMessage = reinterpret_cast <SetCANJagMessage *> ( Message -> Data );

						// Garbage data protection.
						if ( SJMessage == NULL )
						{

							delete Message;
							break;

						}

						// Find appropriate Jaguar and set it.
						for ( uint32_t i = 0; i < Jags -> GetLength (); i ++ )
						{

							ServerCANJagInfo JagInfo = ( * Jags ) [ i ];

							if ( JagInfo.ID == SJMessage -> ID )
							{

								JagInfo.Jag -> Set ( SJMessage -> Speed, SJMessage -> SyncGroup );

								break;

							}

						}

						delete SJMessage;
						delete Message;

						break;

					// Add Jaguar
					case SEND_MESSAGE_JAG_ADD:

						Log -> Log ( Logger :: LOG_DEBUG3, "CANJaguarServer ADDING JAGUAR:\n" );

						// Retreive ADD_JAG Message.
						AddCANJagMessage * AJMessage = reinterpret_cast <AddCANJagMessage *> ( Message -> Data );

						// Garbage protection.
						if ( AJMessage == NULL )
						{

							delete Message;
							break;

						}

						Conflict = false;

						// Does a Jaguar with the requested CAN_ID exist?
						for ( uint32_t i = 0; i < Jags -> GetLength (); i ++ )
						{

							if ( ( * Jags ) [ i ].ID == AJMessage -> ID )
							{

								// Clean up.
								delete AJMessage;
								delete Message;

								Conflict = true;

								Log -> Log ( Logger :: LOG_DEBUG3, "CANJaguarServer ADD_JAG_CONFLICT! (ID: %i)\n", AJMessage -> ID );

								break;

							}

						}

						// Do not create conflicting CANJaguar.
						if ( Conflict )
							break;

						{

							ServerCANJagInfo NewJag;

							NewJag.ID = AJMessage -> ID;
							NewJag.Jag = new CANJaguar ( AJMessage -> ID );
							NewJag.Info = AJMessage -> Config;

							ConfigCANJaguar ( NewJag.Jag, NewJag.Info );

							Jags -> Push ( NewJag );

							Log -> Log ( Logger :: LOG_DEBUG3, "CANJaguarServer ADDED! (ID: %i)\n", AJMessage -> ID );

						}

						delete AJMessage;
						delete Message;

						break;

					// Config Jaguar
					case SEND_MESSAGE_JAG_CONFIG:

						Log -> Log ( Logger :: LOG_DEBUG2, "CANJaguarServer JAG_CONFIG\n" );

						// Retreive JAG_CONFIG Message.
						ConfigCANJagMessage * CJMessage = reinterpret_cast <ConfigCANJagMessage *> ( Message -> Data );

						// Garbage protection.
						if ( CJMessage == NULL )
						{

							Log -> Log ( Logger :: LOG_WARNING, "CANJaguarServer CJMESSAGE NULL\n" );

							delete Message;
							break;

						}

						Log -> Log ( Logger :: LOG_DEBUG2, "CANJaguarServer CONF ID: %i\n", CJMessage -> ID );

						// Find the appropriate Jaguar and configure it.
						for ( uint32_t i = 0; i < Jags -> GetLength (); i ++ )
						{

							Log -> Log ( Logger :: LOG_DEBUG2, "CANJaguarServer TEST #%i, ID: %i\n", i, CJMessage -> ID );

							ServerCANJagInfo JagInfo = ( * Jags ) [ i ];

							if ( JagInfo.ID == CJMessage -> ID )
							{

								Log -> Log ( Logger :: LOG_DEBUG2, "CANJaguarServer CONFIGURING... (ID: %i)\n", CJMessage -> ID );

								ConfigCANJaguar ( JagInfo.Jag, CJMessage -> Config );

								( * Jags ) [ i ].Info = CJMessage -> Config;

								break;

							}

						}

						delete Message;
						delete CJMessage;

						break;

					// Get Jaguar Speed
					case SEND_MESSAGE_JAG_GET:

						// Retreive JAG_GET CAN_ID.
						ID = static_cast <CAN_ID> ( Message -> Data );

						// Find appropriate Jaguar.
						for ( uint32_t i = 0; i < Jags -> GetLength (); i ++ )
						{

							ServerCANJagInfo JagInfo = ( * Jags ) [ i ];

							if ( JagInfo.ID == ID )
							{

								// Respond with CANJaguar :: Get ();
								SendMessage = new CANJagServerMessage ();
								volatile GetCANJagMessage * JVMessage = new GetCANJagMessage ();

								JVMessage -> ID = JagInfo.ID;
								JVMessage -> Value = JagInfo.Jag -> Get ();

								SendMessage -> Command = SEND_MESSAGE_JAG_GET;
								SendMessage -> Data = reinterpret_cast <uint32_t> ( JVMessage );

								msgQSend ( MessageReceiveQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( CANJagServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT );

							}

						}

						delete Message;

						break;

					// Remove Jaguar
					case SEND_MESSAGE_JAG_REMOVE:

						// Which Jaguar?
						ID = static_cast <uint32_t> ( Message -> Data );

						// Find it and remove it.
						for ( uint32_t i = 0; i < Jags -> GetLength (); i ++ )
						{

							ServerCANJagInfo JagInfo = ( * Jags ) [ i ];

							if ( JagInfo.ID == ID )
							{

								Jags -> Remove ( i, 1 );

								JagInfo.Jag -> DisableControl ();
								delete JagInfo.Jag;

								delete Message;
								break;

							}

						}

						delete Message;

						break;

					// CANJaguar :: UpdateSyncGroup (). (I'm not sure this actually needs to run in the same thread context as the appropriate jags, but this is easier than testing it.)
					case SEND_MESSAGE_JAG_UPDATE_SYNC_GROUP:

						uint8_t Group = static_cast <uint8_t> ( Message -> Data );
						CANJaguar :: UpdateSyncGroup ( Group );

						delete Message;

						break;

					case SEND_MESSAGE_JAG_GET_BUS_VOLTAGE:

						// Retreive JAG_GET_BUS_VOLTAGE CAN_ID.
						ID = static_cast <CAN_ID> ( Message -> Data );

						// Find appropriate Jaguar.
						for ( uint32_t i = 0; i < Jags -> GetLength (); i ++ )
						{

							ServerCANJagInfo JagInfo = ( * Jags ) [ i ];

							if ( JagInfo.ID == ID )
							{

								// Respond with CANJaguar :: GetBusVoltage ()
								SendMessage = new CANJagServerMessage ();

								GetCANJagBusVoltageMessage * GJBVMessage = new GetCANJagBusVoltageMessage ();

								GJBVMessage -> Value = JagInfo.Jag -> GetBusVoltage ();
								GJBVMessage -> ID = ID;

								SendMessage -> Command = SEND_MESSAGE_JAG_GET_BUS_VOLTAGE;
								SendMessage -> Data = reinterpret_cast <uint32_t> ( GJBVMessage );

								msgQSend ( MessageReceiveQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( CANJagServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT );

								break;

							}

						}

						delete Message;

						break;

					case SEND_MESSAGE_JAG_GET_OUTPUT_VOLTAGE:

						ID = static_cast <CAN_ID> ( Message -> Data );

						for ( uint32_t i = 0; i < Jags -> GetLength (); i ++ )
						{

							ServerCANJagInfo JagInfo = ( * Jags ) [ i ];

							if ( JagInfo.ID == ID )
							{

								SendMessage = new CANJagServerMessage ();

								GetCANJagOutputVoltageMessage * GJOVMessage = new GetCANJagOutputVoltageMessage ();

								GJOVMessage -> Value = JagInfo.Jag -> GetOutputVoltage ();
								GJOVMessage -> ID = ID;

								SendMessage -> Command = SEND_MESSAGE_JAG_GET_OUTPUT_VOLTAGE;
								SendMessage -> Data = reinterpret_cast <uint32_t> ( GJOVMessage );

								msgQSend ( MessageReceiveQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( CANJagServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT );

								break;

							}

						}

						delete Message;

						break;

					case SEND_MESSAGE_JAG_GET_OUTPUT_CURRENT:

						ID = static_cast <CAN_ID> ( Message -> Data );

						for ( uint32_t i = 0; i < Jags -> GetLength (); i ++ )
						{

							ServerCANJagInfo JagInfo = ( * Jags ) [ i ];

							if ( JagInfo.ID == ID )
							{

								SendMessage = new CANJagServerMessage ();

								GetCANJagOutputCurrentMessage * GJOCMessage = new GetCANJagOutputCurrentMessage ();

								GJOCMessage -> Value = JagInfo.Jag -> GetOutputCurrent ();
								GJOCMessage -> ID = ID;

								SendMessage -> Command = SEND_MESSAGE_JAG_GET_OUTPUT_CURRENT;
								SendMessage -> Data = reinterpret_cast <uint32_t> ( GJOCMessage );

								msgQSend ( MessageReceiveQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( CANJagServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT );

								break;

							}

						}

						delete Message;

						break;

					case SEND_MESSAGE_JAG_GET_POSITION:

						// Retreive JAG_GET CAN_ID.
						ID = static_cast <CAN_ID> ( Message -> Data );

						// Find appropriate Jaguar.
						for ( uint32_t i = 0; i < Jags -> GetLength (); i ++ )
						{

							ServerCANJagInfo JagInfo = ( * Jags ) [ i ];

							if ( JagInfo.ID == ID )
							{

								// Respond with CANJaguar :: GetPosition ();
								SendMessage = new CANJagServerMessage ();
								volatile GetCANJagPositionMessage * JVMessage = new GetCANJagPositionMessage ();

								JVMessage -> ID = JagInfo.ID;
								JVMessage -> Value = JagInfo.Jag -> GetPosition ();

								SendMessage -> Command = SEND_MESSAGE_JAG_GET_POSITION;
								SendMessage -> Data = reinterpret_cast <uint32_t> ( JVMessage );

								msgQSend ( MessageReceiveQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( CANJagServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT );

							}

						}

						delete Message;

						break;

					default:

						delete Message;

						break;

				}

			}
			else
			{

				Log -> Log ( Logger :: LOG_WARNING, "CANJaguarServer NULL MESSAGE\n" );

			}

		}
		else if ( errno != S_objLib_OBJ_TIMEOUT )
			Log -> Log ( Logger :: LOG_DEBUG, "CANJaguarServer MESSAGE RECEIVE FAILURE\n" );

		// Get time passed since last brown-out check.
		double CheckTime = Timer :: GetPPCTimestamp ();
		double CheckTimeDelta = CheckTime - PreJagCheckTime;

		// Has the required time passed to check a jaguar? If so, check it.
		if ( CheckJags && ( JagCheckInterval <= CheckTimeDelta ) )
		{

			PreJagCheckTime = CheckTime;

			// No Jags currently.
			if ( JagLoopCounter >= Jags -> GetLength () )
				JagLoopCounter = 0;

			if ( Jags -> GetLength () != 0 )
				CheckCANJaguar ( ( * Jags ) [ JagLoopCounter ].Jag, ( * Jags ) [ JagLoopCounter ].Info );

			JagLoopCounter ++;

		}

	}

};	

// Server entry point.
void CANJaguarServer :: _StartServerTask ( CANJaguarServer * Server )
{

	Server -> RunLoop ();

};
