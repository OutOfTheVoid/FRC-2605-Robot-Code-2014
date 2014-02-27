#include "AnalogCANJaguarPipeServer.h"

#include <sysLib.h> 

AnalogCANJaguarPipeServer :: AnalogCANJaguarPipeServer ()
{

	Running = false;

	ServerTask = new Task ( "2605_AnalogCANJaguarPipeServer_Task", (FUNCPTR) & _StartServerTask, ANALOGCANJAGSERVERTASK_PRIORITY, ANALOGCANJAGSERVERTASK_STACKSIZE );

	Pipes = new Vector <AnalogCANJaguarPipe> ();

};

AnalogCANJaguarPipeServer :: ~AnalogCANJaguarPipeServer ()
{

	if ( Running )
		Stop ();

};

bool AnalogCANJaguarPipeServer :: Start () 
{

	SendMessageQueue = msgQCreate ( ANALOGCANJAGSERVER_MESSAGEQUEUE_LENGTH, sizeof ( ServerMessage * ), MSG_Q_FIFO );

	if ( SendMessageQueue == NULL )
		return false;

	ReceiveMessageQueue = msgQCreate ( ANALOGCANJAGSERVER_MESSAGEQUEUE_LENGTH, sizeof ( ServerMessage * ), MSG_Q_FIFO );

	if ( ReceiveMessageQueue == NULL )
	{

		msgQDelete ( SendMessageQueue );

		SendMessageQueue = NULL;

		return false;

	}

	PipesAccessSemaphore = semMCreate ( SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE );

	if ( PipesAccessSemaphore == NULL )
	{

		msgQDelete ( SendMessageQueue );
		msgQDelete ( ReceiveMessageQueue );

		ReceiveMessageQueue = NULL;
		SendMessageQueue = NULL;

		return false;

	}

	ResponseSemaphore = semMCreate ( SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE );

	if ( ResponseSemaphore == NULL )
	{

		msgQDelete ( SendMessageQueue );
		msgQDelete ( ReceiveMessageQueue );
		semDelete ( PipesAccessSemaphore );

		SendMessageQueue = NULL;
		ReceiveMessageQueue = NULL;
		PipesAccessSemaphore = NULL;

		return false;

	}

	if ( ! ServerTask -> Start ( reinterpret_cast <uint32_t> ( this ) ) )
	{

		msgQDelete ( SendMessageQueue );
		msgQDelete ( ReceiveMessageQueue );
		semDelete ( PipesAccessSemaphore );
		semDelete ( ResponseSemaphore );

		SendMessageQueue = NULL;
		ReceiveMessageQueue = NULL;
		PipesAccessSemaphore = NULL;
		ResponseSemaphore = NULL;

		return false;

	}

	Running = true;

	return true;

};

void AnalogCANJaguarPipeServer :: Stop ()
{

	if ( ! Running )
		return;

	semTake ( PipesAccessSemaphore, WAIT_FOREVER );
	semTake ( ResponseSemaphore, WAIT_FOREVER );

	ServerTask -> Stop ();

	semGive ( ResponseSemaphore );
	semGive ( PipesAccessSemaphore );

	bool DeQueueSuccessful = true;

	while ( DeQueueSuccessful )
	{

		ServerMessage * Message = NULL;

		DeQueueSuccessful = ( msgQReceive ( SendMessageQueue, reinterpret_cast <char *> ( & Message ), sizeof ( ServerMessage * ), 0 ) != ERROR );

		if ( DeQueueSuccessful && Message != NULL )
		{

			switch ( Message -> Command )
			{



				default:

					delete Message;

					break;

			}

		}

	}

	msgQDelete ( SendMessageQueue );
	msgQDelete ( ReceiveMessageQueue );
	semDelete ( PipesAccessSemaphore );
	semDelete ( ResponseSemaphore );

	SendMessageQueue = NULL;
	ReceiveMessageQueue = NULL;
	PipesAccessSemaphore = NULL;
	ResponseSemaphore = NULL;

	Running = false;

};

bool AnalogCANJaguarPipeServer :: IsRunning ()
{

	return Running;

};

void AnalogCANJaguarPipeServer :: DisablePipe ( AnalogCANJaguarPipe_t Pipe )
{

	ServerMessage * SendMessage = new ServerMessage ();

	SendMessage -> Command = COMMAND_DISABLE_PIPE;
	SendMessage -> Data = static_cast <uint32_t> ( Pipe );

	msgQSend ( SendMessageQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT );

};

void AnalogCANJaguarPipeServer :: EnablePipe ( AnalogCANJaguarPipe_t Pipe )
{

	ServerMessage * SendMessage = new ServerMessage ();

	SendMessage -> Command = COMMAND_ENABLE_PIPE;
	SendMessage -> Data = static_cast <uint32_t> ( Pipe );

	msgQSend ( SendMessageQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_NORMAL );

};

AnalogCANJaguarPipe_t AnalogCANJaguarPipeServer :: AddPipe ( CAN_ID JaguarID, uint8_t Channel, uint8_t Module )
{

	semTake ( ResponseSemaphore, WAIT_FOREVER );

	ServerMessage * SendMessage = new ServerMessage ();

	AddPipeMessage * APMessage = new AddPipeMessage ();

	APMessage -> JaguarID = JaguarID;
	APMessage -> Channel = Channel;
	APMessage -> Module = Module;

	SendMessage -> Command = COMMAND_ADD_PIPE;
	SendMessage -> Data = reinterpret_cast <uint32_t> ( APMessage );

	msgQSend ( SendMessageQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT );

	ServerMessage * ResponseMessage = NULL;
	msgQReceive ( ReceiveMessageQueue, reinterpret_cast <char *> ( & ResponseMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER );

	semGive ( ResponseSemaphore );

	AnalogCANJaguarPipe_t PipeIndex = static_cast <AnalogCANJaguarPipe_t> ( reinterpret_cast <uint32_t> ( ResponseMessage -> Data ) );

	delete ResponseMessage;

	return PipeIndex;

};

void AnalogCANJaguarPipeServer :: RemovePipe ( AnalogCANJaguarPipe_t Pipe )
{

	ServerMessage * SendMessage = new ServerMessage ();

	SendMessage -> Command = COMMAND_REMOVE_PIPE;
	SendMessage -> Data = static_cast <uint32_t> ( Pipe );

	msgQSend ( SendMessageQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT );

};

void AnalogCANJaguarPipeServer :: SetPipeInverted ( AnalogCANJaguarPipe_t Pipe, bool Inverted )
{

	ServerMessage * SendMessage = new ServerMessage ();

	SetPipeInvertedMessage * SIMessage = new SetPipeInvertedMessage ();

	SIMessage -> Pipe = Pipe;
	SIMessage -> Inverted = Inverted;

	SendMessage -> Command = COMMAND_SET_PIPE_INVERTED;
	SendMessage -> Data = reinterpret_cast <uint32_t> ( SIMessage );

	msgQSend ( SendMessageQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_NORMAL );

};

void AnalogCANJaguarPipeServer :: SetPipeOffset ( AnalogCANJaguarPipe_t Pipe, double Offset )
{

	ServerMessage * SendMessage = new ServerMessage ();

	SetPipeOffsetMessage * SOMessage = new SetPipeOffsetMessage ();

	SOMessage -> Pipe = Pipe;
	SOMessage -> Offset = Offset;

	SendMessage -> Command = COMMAND_SET_PIPE_OFFSET;
	SendMessage -> Data = reinterpret_cast <uint32_t> ( SOMessage );

	msgQSend ( SendMessageQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_NORMAL );

};

void AnalogCANJaguarPipeServer :: ZeroPipe ( AnalogCANJaguarPipe_t Pipe )
{

	ServerMessage * SendMessage = new ServerMessage ();

	SendMessage -> Command = COMMAND_ZERO_PIPE;
	SendMessage -> Data = static_cast <uint32_t> ( Pipe );

	msgQSend ( SendMessageQueue, reinterpret_cast <char *> ( & SendMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_NORMAL );

};

void AnalogCANJaguarPipeServer :: RunLoop ()
{

	double sysClkRate = static_cast <double> ( sysClkRateGet () );

	while ( true )
	{

		double LoopInitialTime = Timer :: GetPPCTimestamp ();
		double LoopTimeElapsed = 0.0;

		uint32_t PipeIndex;

		while ( LoopTimeElapsed < LOOP_ITERATION_TIME )
		{

			ServerMessage * Message = NULL;

			if ( msgQReceive ( SendMessageQueue, reinterpret_cast <char *> ( & Message ), sizeof ( ServerMessage * ), static_cast <int> ( ( LOOP_ITERATION_TIME - LoopTimeElapsed ) * sysClkRate ) ) != ERROR )
			{

				switch ( Message -> Command )
				{

				case COMMAND_NOP:

					delete Message;

					break;

				case COMMAND_DISABLE_PIPE:

					semTake ( PipesAccessSemaphore, WAIT_FOREVER );

					PipeIndex = reinterpret_cast <uint32_t> ( Message -> Data );

					if ( PipeIndex + 1 <= Pipes -> GetLength () )
					{

						( * Pipes ) [ PipeIndex ].Enabled = false;
						( * Pipes ) [ PipeIndex ].Jaguar -> DisableControl ();

					}

					semGive ( PipesAccessSemaphore );

					break;

				case COMMAND_ENABLE_PIPE:

					semTake ( PipesAccessSemaphore, WAIT_FOREVER );

					PipeIndex = reinterpret_cast <uint32_t> ( Message -> Data );

					if ( PipeIndex + 1 <= Pipes -> GetLength () )
					{

						( * Pipes ) [ PipeIndex ].Jaguar -> EnableControl ();
						( * Pipes ) [ PipeIndex ].Enabled = true;
					
					}

					semGive ( PipesAccessSemaphore );

					break;

				case COMMAND_ADD_PIPE:

					AddPipeMessage * APMessage = reinterpret_cast <AddPipeMessage *> ( Message -> Data );

					AnalogCANJaguarPipe NewPipe;

					NewPipe.JaguarID = APMessage -> JaguarID;
					NewPipe.Channel = APMessage -> Channel;
					NewPipe.Module = APMessage -> Module;

					NewPipe.Jaguar = new CANJaguar ( static_cast <uint8_t> ( NewPipe.JaguarID ), JAGCONTROLMODE );
					NewPipe.Jaguar -> DisableControl ();

					NewPipe.InputChannel = new AnalogChannel ( NewPipe.Module, NewPipe.Channel );

					NewPipe.Offset = 2.5;
					NewPipe.Inverted = false;
					NewPipe.Enabled = false;

					semTake ( PipesAccessSemaphore, WAIT_FOREVER );

					Pipes -> Push ( NewPipe );

					ServerMessage * ResponseMessage = new ServerMessage ();

					ResponseMessage -> Command = COMMAND_ADD_PIPE;
					ResponseMessage -> Data = static_cast <uint32_t> ( Pipes -> GetLength () - 1 );

					semGive ( PipesAccessSemaphore );

					msgQSend ( ReceiveMessageQueue, reinterpret_cast <char *> ( & ResponseMessage ), sizeof ( ServerMessage * ), WAIT_FOREVER, MSG_PRI_URGENT );

					delete APMessage;
					delete Message;

					break;

				case COMMAND_REMOVE_PIPE:

					PipeIndex = reinterpret_cast <uint32_t> ( Message -> Data );

					if ( PipeIndex + 1 <= Pipes -> GetLength () )
					{

						semTake ( PipesAccessSemaphore, WAIT_FOREVER );

						AnalogCANJaguarPipe PipeToKill = ( * Pipes ) [ PipeIndex ];

						delete PipeToKill.Jaguar;
						delete PipeToKill.InputChannel;

						Pipes -> Remove ( PipeIndex, 1 );

						semGive ( PipesAccessSemaphore );

					}

					delete Message;

					break;

				case COMMAND_SET_PIPE_INVERTED:

					SetPipeInvertedMessage * SIMessage = reinterpret_cast <SetPipeInvertedMessage *> ( Message -> Data );

					semTake ( PipesAccessSemaphore, WAIT_FOREVER );

					PipeIndex = SIMessage -> Pipe;

					if ( PipeIndex + 1 <= Pipes -> GetLength () )
						( * Pipes ) [ PipeIndex ].Inverted = SIMessage -> Inverted;

					semGive ( PipesAccessSemaphore );

					delete SIMessage;
					delete Message;

					break;

				case COMMAND_SET_PIPE_OFFSET:

					SetPipeOffsetMessage * SOMessage = reinterpret_cast <SetPipeOffsetMessage *> ( Message -> Data );

					semTake ( PipesAccessSemaphore, WAIT_FOREVER );

					PipeIndex = SOMessage -> Pipe;

					if ( PipeIndex + 1 <= Pipes -> GetLength () )
						( * Pipes ) [ PipeIndex ].Offset = SOMessage -> Offset;

					semGive ( PipesAccessSemaphore );

					delete SOMessage;
					delete Message;

					break;

				case COMMAND_ZERO_PIPE:

					semTake ( PipesAccessSemaphore, WAIT_FOREVER );

					PipeIndex = static_cast <uint32_t> ( Message -> Data );

					if ( PipeIndex + 1 <= Pipes -> GetLength () )
						( * Pipes ) [ PipeIndex ].Offset = ( * Pipes ) [ PipeIndex ].InputChannel -> GetVoltage ();

					semGive ( PipesAccessSemaphore );

					break;

				default:

					delete Message;

					break;

				}

			}

			Wait ( 0.001 );

			LoopTimeElapsed = Timer :: GetPPCTimestamp () - LoopInitialTime;

		}

		semTake ( PipesAccessSemaphore, WAIT_FOREVER ); 

		for ( uint32_t i = 0; i < Pipes -> GetLength (); i ++ )
		{
			
			double in = ( * Pipes ) [ i ].InputChannel -> GetVoltage ();
			double in_offset = in - ( * Pipes ) [ i ].Offset;
			double in_proportional = in_offset / INPUT_SCALE;
			double in_inverted = in_proportional * ( ( * Pipes ) [ i ].Inverted ? -1 : 1 );
			double out = in_inverted * JAGSCALE;

			if ( ( * Pipes ) [ i ].Enabled )
				( * Pipes ) [ i ].Jaguar -> Set ( out );


		}
	}

};

int AnalogCANJaguarPipeServer :: _StartServerTask ( AnalogCANJaguarPipeServer * This )
{

	This -> RunLoop ();

	return 0;

};
