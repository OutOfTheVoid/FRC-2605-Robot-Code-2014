#include "PICServoCom.h"

PICServoCom :: PICServoCom ()
{

	Com = new SerialDriver ( 19200 );
	Com -> SetFlowControl ( SerialDriver :: FlowControl_None );
	Com -> SetTimeout ( 5.0 );
	Com -> SetReadBufferSize ( 0x400 );
	Com -> SetWriteBufferSize ( 0x400 );
	Com -> Clear ();

	Com -> FlushReadBuffer ();

	SerialLock = semMCreate ( SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE );

	StatusSize = 2;
	StatusType = 0x00;
	StatusBytes = new uint8_t [ 20 ];

};

PICServoCom :: ~PICServoCom ()
{

	delete Com;

};

void PICServoCom :: ModuleResetPosition ( uint8_t Module, bool Relative )
{

	if ( ! Relative )
	{
		
		uint8_t Data [ 0 ] = {};

		SendMessage ( Module, PICSERVO_COMMAND_RESET_POS, Data, 0 );
	
	}
	else
	{

		uint8_t Data [ 1 ] = { 0x01 };

		SendMessage ( Module, PICSERVO_COMMAND_RESET_POS | PICSERVO_RESET_POSITION_DATASIZE_1, Data, 1 );

	}

};

void PICServoCom :: ModuleOverwritePosition ( uint8_t Module, int32_t Position )
{

	uint32_t * PositionData = reinterpret_cast <uint32_t *> ( & Position );
	uint8_t Data [ 5 ] = { 0x02, * PositionData & 0xFF, ( * PositionData >> 8 ) & 0xFF, ( * PositionData >> 16 ) & 0xFF, ( * PositionData >> 24 ) & 0xFF };

	SendMessage ( Module, PICSERVO_COMMAND_RESET_POS | PICSERVO_RESET_POSITION_DATASIZE_5, Data, 5 );

};

void PICServoCom :: ModuleSetAddress ( uint8_t Module, uint8_t NewAddress, uint8_t NewGroupAddress )
{

	uint8_t Data [ 2 ] = { NewAddress, NewGroupAddress };

	SendMessage ( Module, PICSERVO_COMMAND_SET_ADDR | PICSERVO_SET_ADDR_DATASIZE_2, Data, 2 );

};

void PICServoCom :: ModuleDefineStatus ( uint8_t Module, uint8_t Type )
{

	uint8_t NewStatusSize = 2;
	uint8_t Data [ 1 ] = { Type };

	if ( StatusType & PICSERVO_STATUS_TYPE_POSITION )
		NewStatusSize += 4;

	if ( StatusType & PICSERVO_STATUS_TYPE_CURRENT_SENSE )
		NewStatusSize ++;

	if ( StatusType & PICSERVO_STATUS_TYPE_ENCODER_VELOCITY )
		NewStatusSize += 2;

	if ( StatusType & PICSERVO_STATUS_TYPE_AUXILIARY_STATUS )
		NewStatusSize ++;

	if ( StatusType & PICSERVO_STATUS_TYPE_HOME_POSITION )
		NewStatusSize += 4;

	if ( StatusType & PICSERVO_STATUS_TYPE_DEVICE_TYPE_VERSION )
		NewStatusSize += 2;

	if ( StatusType & PICSERVO_STATUS_TYPE_SERVO_ERROR )
		NewStatusSize += 2;

	if ( StatusType & PICSERVO_STATUS_TYPE_PATH_REMAINING )
		NewStatusSize ++;

	StatusSize = NewStatusSize;
	StatusType = Type;

	SendMessage ( Module, PICSERVO_COMMAND_DEF_STAT | PICSERVO_DEF_STAT_DATASIZE_1, Data, 1 );

};

void PICServoCom :: SetStatusType ( uint8_t Type )
{

	StatusType = Type;

	uint8_t NewStatusSize = 2;

	if ( StatusType & PICSERVO_STATUS_TYPE_POSITION )
		NewStatusSize += 4;

	if ( StatusType & PICSERVO_STATUS_TYPE_CURRENT_SENSE )
		NewStatusSize ++;

	if ( StatusType & PICSERVO_STATUS_TYPE_ENCODER_VELOCITY )
		NewStatusSize += 2;

	if ( StatusType & PICSERVO_STATUS_TYPE_AUXILIARY_STATUS )
		NewStatusSize ++;

	if ( StatusType & PICSERVO_STATUS_TYPE_HOME_POSITION )
		NewStatusSize += 4;

	if ( StatusType & PICSERVO_STATUS_TYPE_DEVICE_TYPE_VERSION )
		NewStatusSize += 2;

	if ( StatusType & PICSERVO_STATUS_TYPE_SERVO_ERROR )
		NewStatusSize += 2;

	if ( StatusType & PICSERVO_STATUS_TYPE_PATH_REMAINING )
		NewStatusSize ++;

	StatusSize = NewStatusSize;

};

void PICServoCom :: ModuleReadStatus ( uint8_t Module, uint8_t Type, PICServoStatus_t * Status )
{

	uint8_t TempStatusSize = 2;
	uint8_t Data [ 1 ] = { Type };

	if ( Type & PICSERVO_STATUS_TYPE_POSITION )
		TempStatusSize += 4;

	if ( Type & PICSERVO_STATUS_TYPE_CURRENT_SENSE )
		TempStatusSize ++;

	if ( Type & PICSERVO_STATUS_TYPE_ENCODER_VELOCITY )
		TempStatusSize += 2;

	if ( Type & PICSERVO_STATUS_TYPE_AUXILIARY_STATUS )
		TempStatusSize ++;

	if ( Type & PICSERVO_STATUS_TYPE_HOME_POSITION )
		TempStatusSize += 4;

	if ( Type & PICSERVO_STATUS_TYPE_DEVICE_TYPE_VERSION )
		TempStatusSize += 2;

	if ( Type & PICSERVO_STATUS_TYPE_SERVO_ERROR )
		TempStatusSize += 2;

	if ( Type & PICSERVO_STATUS_TYPE_PATH_REMAINING )
		TempStatusSize ++;

	uint8_t TempStatusBytes [ TempStatusSize ];

	SendMessage ( Module, PICSERVO_COMMAND_DEF_STAT | PICSERVO_DEF_STAT_DATASIZE_1, Data, 1 );
	ReceiveMessage ( TempStatusBytes, TempStatusSize );

	uint8_t Counter = 1;

	Status -> StandardFlags = StatusBytes [ 0 ];

	if ( Type & PICSERVO_STATUS_TYPE_POSITION )
	{

		Status -> Position = static_cast <int32_t> ( TempStatusBytes [ 1 ] );
		Status -> Position |= static_cast <int32_t> ( TempStatusBytes [ 2 ] ) << 8;
		Status -> Position |= static_cast <int32_t> ( TempStatusBytes [ 3 ] ) << 16;
		Status -> Position |= static_cast <int32_t> ( TempStatusBytes [ 4 ] ) << 24;

		Counter += 4;

	}

	if ( Type & PICSERVO_STATUS_TYPE_CURRENT_SENSE )
	{

		Status -> CurrentSense = TempStatusBytes [ Counter ];

		Counter ++;

	}

	if ( Type & PICSERVO_STATUS_TYPE_ENCODER_VELOCITY )
	{

		Status -> EncoderVelocity = TempStatusBytes [ Counter ];
		Status -> EncoderVelocity |= static_cast <uint16_t> ( TempStatusBytes [ Counter + 1 ] ) << 8;

		Counter += 2;

	}

	if ( Type & PICSERVO_STATUS_TYPE_AUXILIARY_STATUS )
	{

		Status -> AuxiliaryStatus = TempStatusBytes [ Counter ];

		Counter ++;

	}

	if ( Type & PICSERVO_STATUS_TYPE_HOME_POSITION )
	{

		Status -> HomePosition = static_cast <int32_t> ( TempStatusBytes [ Counter ] );
		Status -> HomePosition |= static_cast <int32_t> ( TempStatusBytes [ Counter + 1 ] ) << 8;
		Status -> HomePosition |= static_cast <int32_t> ( TempStatusBytes [ Counter + 2 ] ) << 16;
		Status -> HomePosition |= static_cast <int32_t> ( TempStatusBytes [ Counter + 3 ] ) << 24;

		Counter += 4;

	}

	if ( Type & PICSERVO_STATUS_TYPE_DEVICE_TYPE_VERSION )
	{

		Status -> DeviceType = TempStatusBytes [ Counter ];
		Status -> DeviceVersion = static_cast <uint16_t> ( TempStatusBytes [ Counter + 1 ] ) << 8;

		Counter += 2;

	}

	if ( Type & PICSERVO_STATUS_TYPE_SERVO_ERROR )
	{

		Status -> PositionError = TempStatusBytes [ Counter ];
		Status -> PositionError |= static_cast <uint16_t> ( TempStatusBytes [ Counter + 1 ] ) << 8;

		Counter += 2;

	}

	if ( Type & PICSERVO_STATUS_TYPE_PATH_REMAINING )
	{

		Status -> PathPointsPending = TempStatusBytes [ Counter ];

		Counter ++;

	}

};

void PICServoCom :: ModuleSetMetrics ( uint8_t Module, uint16_t P, uint16_t I, uint16_t D, uint16_t IntegrationLimit, uint8_t OutputLimit, int8_t CurrentLimit, uint16_t PositionErrorLimit, uint8_t ServoRateDevisor, uint8_t AmplifierDeadbandCompensation, uint8_t StepRateMultiplier )
{

	uint8_t CurrentV = CurrentLimit > 0 ? ( 1 + CurrentLimit * 2 ) : ( CurrentLimit * - 2 );

	uint8_t Data [ 15 ] = { P & 0xFF, P >> 8, I & 0xFF, I >> 8, D & 0xFF, D >> 8, IntegrationLimit & 0xFF, IntegrationLimit >> 8, OutputLimit, CurrentV, PositionErrorLimit & 0xFF, PositionErrorLimit >> 8, ServoRateDevisor, AmplifierDeadbandCompensation, StepRateMultiplier };

	SendMessage ( Module, PICSERVO_COMMAND_SET_GAIN | PICSERVO_SET_GAIN_DATASIZE_15, Data, 15 );

};

void PICServoCom :: ModuleStopMotor ( uint8_t Module, bool AmplifierEnabled, bool MotorOff, bool Abruptly )
{

	uint8_t Value = AmplifierEnabled ? 0x01 : 0x00;
	Value |= MotorOff ? 0x02 : 0x00;
	Value |= Abruptly ? 0x04 : 0x08;

	uint8_t Data [ 1 ] = { Value };

	SendMessage ( Module, PICSERVO_COMMAND_STOP_MOTOR | PICSERVO_STOP_MOTOR_DATASIZE_1, Data, 1 );

};

void PICServoCom :: ModuleIOControl ( uint8_t Module, bool LimitSwitches = false, bool LimitAbruptly = true, bool ThreePhaseCom = false, bool AntiphasePWM = true, bool FastPath = false, bool StepAndDirection = false )
{

	uint8_t Value = LimitSwitches ? 0x04 : 0x00;

	if ( LimitSwitches )
		Value |= LimitAbruptly ? 0x08 : 0x00;

	Value |= ThreePhaseCom ? 0x10 : 0x00;
	Value |= AntiphasePWM ? 0x20 : 0x00;
	Value |= FastPath ? 0x40 : 0x00;
	Value |= StepAndDirection ? 0x80 : 0x00;

	uint8_t Data [ 1 ] = { Value };

	SendMessage ( Module, PICSERVO_COMMAND_IO_CTRL | PICSERVO_IO_CONTROL_DATASIZE_1, Data, 1 );

};

void PICServoCom :: ModuleClearStatus ( uint8_t Module )
{

	SendMessage ( Module, PICSERVO_COMMAND_CLEAR_BITS, NULL, 0 );

};

void PICServoCom :: ModuleGetStatus ( uint8_t Module )
{

	SendMessage ( Module, PICSERVO_COMMAND_NOP, NULL, 0 );

};

void PICServoCom :: ModuleHardReset ( uint8_t Module )
{

	SendMessage ( Module, PICSERVO_COMMAND_HARD_RESET, NULL, 0 );

};

void PICServoCom :: ModuleHardReset ( uint8_t Module, bool SaveConfigInEERROM, bool RestoreAddresses, bool AmplifierEnabled, bool ServoEnabled, bool StepAndDirectionEnabled, bool LimitAndStopEnabled, bool ThreePhaseComEnabled, bool AntiphasePWMEnabled )
{

	uint8_t Value = SaveConfigInEERROM ? 0x01 : 0x00;
	Value |= RestoreAddresses ? 0x02 : 0x00;
	Value |= AmplifierEnabled ? 0x04 : 0x00;
	Value |= ServoEnabled ? 0x08 : 0x00;
	Value |= StepAndDirectionEnabled ? 0x10 : 0x00;
	Value |= LimitAndStopEnabled ? 0x20 : 0x00;
	Value |= ThreePhaseComEnabled ? 0x40 : 0x00;
	Value |= AntiphasePWMEnabled ? 0x80 : 0x00;

	uint8_t Data [ 1 ] = { Value };

	SendMessage ( Module, PICSERVO_COMMAND_HARD_RESET | PICSERVO_HARD_RESET_DATASIZE_1, Data, 1 );

};

void PICServoCom :: ModuleLoadTrajectory ( uint8_t Module, int32_t Position, double Velocity, double Acceleration, int16_t PWM, bool LoadPosition, bool LoadVelocity, bool LoadAcceleration, bool LoadPWM, bool EnableServo, bool VelocityProfileMode, bool RelativePosition, bool ImmediateMotion )
{

	uint8_t DataSize = 1;

	if ( LoadPosition )
		DataSize += 4;

	if ( LoadVelocity )
		DataSize += 4;

	if ( LoadAcceleration )
		DataSize += 4;

	if ( LoadPWM )
		DataSize += 1;

	uint8_t Data [ DataSize ];

	uint8_t ControlByte = LoadPosition ? 0x01 : 0x00;
	ControlByte |= LoadVelocity ? 0x02 : 0x00;
	ControlByte |= LoadAcceleration ? 0x04 : 0x00;
	ControlByte |= LoadPWM ? 0x08 : 0x00;
	ControlByte |= EnableServo ? 0x10 : 0x00;
	ControlByte |= VelocityProfileMode ? 0x20 : 0x00;
	ControlByte |= VelocityProfileMode ? ( PWM >= 0 ? 0x00 : 0x40 ) : ( RelativePosition ? 0x40 : 0x00 );
	ControlByte |= ImmediateMotion ? 0x80 : 0x00;

	Data [ 0 ] = ControlByte;

	uint8_t Counter = 1;

	uint32_t * PositionData = reinterpret_cast <uint32_t *> ( & Position );

	if ( LoadPosition )
	{

		Data [ 1 ] = ( * PositionData ) & 0xFF;
		Data [ 2 ] = ( ( * PositionData ) >> 8 ) & 0xFF;
		Data [ 3 ] = ( ( * PositionData ) >> 16 ) & 0xFF;
		Data [ 4 ] = ( ( * PositionData ) >> 24 ) & 0xFF;

		Counter += 4;

	}

	if ( LoadVelocity )
	{

		if ( Velocity < 0 )
			Velocity = - Velocity;

		uint16_t VelHigh = static_cast <uint16_t> ( Velocity );
		uint16_t VelLow = static_cast <uint16_t> ( Velocity * ( 1 << 16 ) );

		Data [ Counter ] = VelLow & 0xFF;
		Data [ Counter + 1 ] = ( VelLow >> 8 ) & 0xFF;
		Data [ Counter + 2 ] = VelHigh & 0xFF;
		Data [ Counter + 3 ] = ( VelHigh >> 8 ) & 0xFF;

		Counter += 4;

	}

	if ( LoadAcceleration )
	{

		if ( Acceleration < 0 )
			Acceleration = - Acceleration;

		uint16_t AccHigh = static_cast <uint16_t> ( Acceleration );
		uint16_t AccLow = static_cast <uint16_t> ( Acceleration * ( 1 << 16 ) );

		Data [ Counter ] = AccLow & 0xFF;
		Data [ Counter + 1 ] = ( AccLow >> 8 ) & 0xFF;
		Data [ Counter + 2 ] = AccHigh & 0xFF;
		Data [ Counter + 3 ] = ( AccHigh >> 8 ) & 0xFF;

		Counter += 4;

	}

	if ( LoadPWM )
	{

		if ( PWM > 0xFF )
			PWM = 0xFF;

		Data [ Counter ] = PWM;

	}

};

void PICServoCom :: ReceiveStatusPacket ()
{

	uint8_t TBuff [ StatusSize ];

	if ( ReceiveMessage ( TBuff, StatusSize ) )
		memcpy ( reinterpret_cast <void *> ( StatusBytes ), reinterpret_cast <void *> ( TBuff ), StatusSize );

};

void PICServoCom :: GetStatus ( PICServoStatus_t * Status )
{

	uint8_t Counter = 1;

	Status -> StandardFlags = StatusBytes [ 0 ];

	uint8_t CheckSum = 0;

	printf ( "Status: message\n[" );

	for ( uint32_t i = 0; i < StatusSize; i ++ )
	{

		CheckSum += StatusBytes [ i ];
		printf ( " 0x%x", StatusBytes [ i ] );

	}

	printf ( " ]\n" );

	if ( StatusType & PICSERVO_STATUS_TYPE_POSITION )
	{

		Status -> Position = static_cast <int32_t> ( StatusBytes [ 1 ] );
		Status -> Position |= static_cast <int32_t> ( StatusBytes [ 2 ] ) << 8;
		Status -> Position |= static_cast <int32_t> ( StatusBytes [ 3 ] ) << 16;
		Status -> Position |= static_cast <int32_t> ( StatusBytes [ 4 ] ) << 24;

		Counter += 4;

	}

	if ( StatusType & PICSERVO_STATUS_TYPE_CURRENT_SENSE )
	{

		Status -> CurrentSense = StatusBytes [ Counter ];

		Counter ++;

	}

	if ( StatusType & PICSERVO_STATUS_TYPE_ENCODER_VELOCITY )
	{

		Status -> EncoderVelocity = StatusBytes [ Counter ];
		Status -> EncoderVelocity |= static_cast <uint16_t> ( StatusBytes [ Counter + 1 ] ) << 8;

		Counter += 2;

	}

	if ( StatusType & PICSERVO_STATUS_TYPE_AUXILIARY_STATUS )
	{

		Status -> AuxiliaryStatus = StatusBytes [ Counter ];

		Counter ++;

	}

	if ( StatusType & PICSERVO_STATUS_TYPE_HOME_POSITION )
	{

		Status -> HomePosition = static_cast <int32_t> ( StatusBytes [ Counter ] );
		Status -> HomePosition |= static_cast <int32_t> ( StatusBytes [ Counter + 1 ] ) << 8;
		Status -> HomePosition |= static_cast <int32_t> ( StatusBytes [ Counter + 2 ] ) << 16;
		Status -> HomePosition |= static_cast <int32_t> ( StatusBytes [ Counter + 3 ] ) << 24;

		Counter += 4;

	}

	if ( StatusType & PICSERVO_STATUS_TYPE_DEVICE_TYPE_VERSION )
	{

		Status -> DeviceType = StatusBytes [ Counter ];
		Status -> DeviceVersion = StatusBytes [ Counter + 1 ];

		Counter += 2;

	}

	if ( StatusType & PICSERVO_STATUS_TYPE_SERVO_ERROR )
	{

		Status -> PositionError = StatusBytes [ Counter ];
		Status -> PositionError |= static_cast <uint16_t> ( StatusBytes [ Counter + 1 ] ) << 8;

		Counter += 2;

	}

	if ( StatusType & PICSERVO_STATUS_TYPE_PATH_REMAINING )
	{

		Status -> PathPointsPending = StatusBytes [ Counter ];

		Counter ++;

	}

	if ( StatusBytes [ Counter ] != CheckSum )
		printf ( "Checksum Error!\n" );

};

void PICServoCom :: SerialTaskLock ()
{

	semTake ( SerialLock, WAIT_FOREVER );

};

void PICServoCom :: SerialTaskUnlock ()
{

	semGive ( SerialLock );

};

void PICServoCom :: SendMessage ( uint8_t Address, uint8_t Command, uint8_t * Data, uint8_t DataSize )
{

	uint8_t CheckSum = Address + Command;
	uint8_t DataBuffer [ 4 + DataSize ];

	DataBuffer [ 0 ] = 0xAA;
	DataBuffer [ 1 ] = Address;
	DataBuffer [ 2 ] = Command;

	for ( uint8_t i = 0; i < DataSize; i ++ )
	{

		DataBuffer [ 3 + i ] = Data [ i ];
		CheckSum += Data [ i ];
	
	}

	DataBuffer [ 3 + DataSize ] = CheckSum;

	Com -> Write ( DataBuffer, 4 + DataSize );
	Com -> FlushWriteBuffer ();
	Com -> FlushDeviceWrite ();

};

bool PICServoCom :: ReceiveMessage ( uint8_t * Buffer, uint32_t Count )
{

	if ( Com -> Read ( Buffer, Count ) == Count )
	{

		Com -> FlushReadBuffer ();
		Com -> FlushDeviceRead ();

		return true;

	}

	
	Com -> FlushReadBuffer ();
	Com -> FlushDeviceRead ();

	return false;

};
