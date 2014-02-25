#ifndef SHS_2605_ANALOG_CANJAGUAR_PIPE_SERVER_H
#define SHS_2605_ANALOG_CANJAGUAR_PIPE_SERVER_H

#include "WPILib.h"

#include "src/Util/Vector.h"

#include "src/CANJaguarUtils/CANJaguarUtils.h"

#define ANALOGCANJAGSERVERTASK_PRIORITY 50
#define ANALOGCANJAGSERVERTASK_STACKSIZE 0x20000

#define ANALOGCANJAGSERVER_MESSAGEQUEUE_LENGTH 100

#define LOOP_ITERATION_TIME 0.003

#define JAGCONTROLMODE CANJaguar :: kVoltage

#define JAGSCALE 12

#define INPUT_SCALE 2.5

typedef uint32_t AnalogCANJaguarPipe_t;

class AnalogCANJaguarPipeServer
{
public:

	AnalogCANJaguarPipeServer ();
	~AnalogCANJaguarPipeServer ();

	bool Start ();
	void Stop ();

	bool IsRunning ();

	void DisablePipe ( AnalogCANJaguarPipe_t Pipe );
	void EnablePipe ( AnalogCANJaguarPipe_t Pipe );

	AnalogCANJaguarPipe_t AddPipe ( CAN_ID JaguarID, uint8_t Channel, uint8_t Module );
	void RemovePipe ( AnalogCANJaguarPipe_t Pipe );

	void SetPipeInverted ( AnalogCANJaguarPipe_t Pipe, bool Inverted );
	void SetPipeOffset ( AnalogCANJaguarPipe_t Pipe, double Offset );
	void ZeroPipe ( AnalogCANJaguarPipe_t Pipe );

private:

	void RunLoop ();

	typedef struct AnalogCANJaguarPipe
	{

		CAN_ID JaguarID;
		uint8_t Channel;
		uint8_t Module;

		CANJaguar * Jaguar;
		AnalogChannel * InputChannel;
		
		double Offset;
		bool Inverted;
		bool Enabled;

	} AnalogCANJaguarPipe;

	typedef struct ServerMessage
	{

		uint32_t Command;
		uint32_t Data;

	} ServerMessage;

	enum ServerCommands
	{

		COMMAND_NOP = 0,
		COMMAND_DISABLE_PIPE,
		COMMAND_ENABLE_PIPE,
		COMMAND_ADD_PIPE,
		COMMAND_REMOVE_PIPE,
		COMMAND_SET_PIPE_INVERTED,
		COMMAND_SET_PIPE_OFFSET,
		COMMAND_ZERO_PIPE

	};

	typedef struct AddPipeMessage
	{

		CAN_ID JaguarID;
		uint32_t Channel;
		uint32_t Module;

	} AddPipeMessage;

	typedef struct SetPipeInvertedMessage
	{

		uint32_t Pipe;
		bool Inverted;

	} SetPipeInvertedMessage;

	typedef struct SetPipeOffsetMessage
	{

		uint32_t Pipe;
		double Offset;

	} SetPipeOffsetMessage;

	bool Running;

	Task * ServerTask;

	SEM_ID PipesAccessSemaphore;
	SEM_ID ResponseSemaphore;
	MSG_Q_ID SendMessageQueue;
	MSG_Q_ID ReceiveMessageQueue;


	Vector <AnalogCANJaguarPipe> * Pipes;

	static int _StartServerTask ( AnalogCANJaguarPipeServer * This );

};

#endif
