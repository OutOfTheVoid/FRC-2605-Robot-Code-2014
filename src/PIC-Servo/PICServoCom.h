#ifndef SHS_2605_PIC_SERVO_SERVER_H
#define SHS_2605_PIC_SERVO_SERVER_H

#include "WPILib.h"

#define PICSERVO_BAUD_RATE_INITIAL 19200

#define	PICSERVO_COMMAND_RESET_POS	  0x00	//Reset encoder counter to 0 (0 bytes)
#define	PICSERVO_COMMAND_SET_ADDR	  0x01	//Set address and group address (2 bytes)
#define	PICSERVO_COMMAND_DEF_STAT	  0x02	//Define status items to return (1 byte)
#define	PICSERVO_COMMAND_READ_STAT	  0x03	//Read value of current status items
#define	PICSERVO_COMMAND_LOAD_TRAJ  	  0x04	//Load trahectory date (1 - 14 bytes)
#define PICSERVO_COMMAND_START_MOVE	  0x05	//Start pre-loaded trajectory (0 bytes)
#define PICSERVO_COMMAND_SET_GAIN	  0x06  //Set servo gain and control parameters (13 or 14)
#define	PICSERVO_COMMAND_STOP_MOTOR 	  0x07	//Stop motor (1 byte)
#define	PICSERVO_COMMAND_IO_CTRL		  0x08	//Define bit directions and set output (1 byte)
#define PICSERVO_COMMAND_SET_HOMING	  0x09  //Define homing mode (1 byte)
#define	PICSERVO_COMMAND_SET_BAUD	  0x0A 	//Set the baud rate (1 byte)
#define PICSERVO_COMMAND_CLEAR_BITS	  0x0B  //Save current pos. in home pos. register (0 bytes)
#define PICSERVO_COMMAND_SAVE_AS_HOME  0x0C	//Store the input bytes and timer val (0 bytes)
#define PICSERVO_COMMAND_ADD_PATHPOINT 0x0D  //Adds path points for path mode
#define	PICSERVO_COMMAND_NOP			  0x0E	//No operation - returns prev. defined status (0 bytes)
#define PICSERVO_COMMAND_HARD_RESET	  0x0F	//RESET - no status is returned

#define PICSERVO_RESET_POSITION_DATASIZE_0 0x00
#define PICSERVO_RESET_POSITION_DATASIZE_1 0x10
#define PICSERVO_RESET_POSITION_DATASIZE_5 0x50

#define PICSERVO_SET_ADDR_DATASIZE_2 0x20

#define PICSERVO_DEF_STAT_DATASIZE_1 0x10

#define PICSERVO_SET_GAIN_DATASIZE_15 0xF0

#define PICSERVO_STOP_MOTOR_DATASIZE_1 0x10

#define PICSERVO_IO_CONTROL_DATASIZE_1 0x10

#define PICSERVO_HARD_RESET_DATASIZE_1 0x10

#define PICSERVO_STATUS_TYPE_POSITION 0x01
#define PICSERVO_STATUS_TYPE_CURRENT_SENSE 0x02
#define PICSERVO_STATUS_TYPE_ENCODER_VELOCITY 0x04
#define PICSERVO_STATUS_TYPE_AUXILIARY_STATUS 0x08
#define PICSERVO_STATUS_TYPE_HOME_POSITION 0x10
#define PICSERVO_STATUS_TYPE_DEVICE_TYPE_VERSION 0x20
#define PICSERVO_STATUS_TYPE_SERVO_ERROR 0x40
#define PICSERVO_STATUS_TYPE_PATH_REMAINING 0x80

class PICServoCom
{
public:

	typedef struct PICServoStatus_t
	{

		uint8_t StandardFlags;
		uint32_t Position;
		uint8_t CurrentSense;
		uint16_t EncoderVelocity;
		uint8_t AuxiliaryStatus;
		uint32_t HomePosition;
		uint8_t DeviceType;
		uint8_t DeviceVersion;
		uint16_t PositionError;
		uint8_t PathPointsPending;

	} PICServoStatus_t;

	PICServoCom ();
	~PICServoCom ();

	void ModuleResetPosition ( uint8_t Module, bool Relative = false );
	void ModuleOverwritePosition ( uint8_t Module, int32_t Position = 0 );
	void ModuleSetAddress ( uint8_t Module, uint8_t NewAddress, uint8_t NewGroupAddress );
	void ModuleDefineStatus ( uint8_t Module, uint8_t Type = 0 );
	void ModuleReadStatus ( uint8_t Module, uint8_t Type, PICServoStatus_t * Status );
	void ModuleStopMotor ( uint8_t Module, bool AmplifierEnabled, bool MotorOff = true, bool Abruptly = true );
	void ModuleSetMetrics ( uint8_t Module, uint16_t P, uint16_t I, uint16_t D, uint16_t IntegrationLimit = 32767, uint8_t OutputLimit = 127, int8_t CurrentLimit = 127, uint16_t PositionErrorLimit = 32767, uint8_t ServoRateDevisor = 1, uint8_t AmplifierDeadbandCompensation = 0, uint8_t StepRateMultiplier = 1 );
	void ModuleIOControl ( uint8_t Module, bool LimitSwitches, bool LimitAbruptly, bool ThreePhaseCom, bool AntiphasePWM, bool FastPath, bool StepAndDirection );
	void ModuleClearStatus ( uint8_t Module );
	void ModuleGetStatus ( uint8_t Module );
	void ModuleHardReset ( uint8_t Module );
	void ModuleHardReset ( uint8_t Module, bool SaveConfigInEERROM, bool RestoreAddresses, bool AmplifierEnabled, bool ServoEnabled, bool StepAndDirectionEnabled, bool LimitAndStopEnabled, bool ThreePhaseComEnabled, bool AntiphasePWMEnabled );
	void ModuleLoadTrajectory ( uint8_t Module, int32_t Position, double Velocity, double Acceleration, int16_t PWM, bool LoadPosition, bool LoadVelocity, bool LoadAcceleration, bool LoadPWM, bool EnableServo, bool VelocityProfileMode = true, bool RelativePosition = 0, bool ImmediateMotion = true );

	void ReceiveStatusPacket ();

	void GetStatus ( PICServoStatus_t * Status );

	void SerialTaskLock ();
	void SerialTaskUnlock ();

private:

	void SendMessage ( uint8_t Address, uint8_t Command, uint8_t * Data, uint8_t DataSize );
	void ReceiveMessage ( uint8_t * Buffer, uint8_t Count );

	SerialPort * Port;

	SEM_ID SerialLock;

	uint8_t StatusType;
	uint8_t StatusSize;
	uint8_t * StatusBytes;

};

#endif
