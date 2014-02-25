#include "AsynchCANJaguar.h"

/*
* Copyright (C) 2014 Liam Taylor
* FRC Team Sehome Semonsters 2605
*/

AsynchCANJaguar :: AsynchCANJaguar ( CANJaguarServer * Server, CAN_ID ID, CANJagConfigInfo Config )
{

	this -> Server = Server;
	this -> ID = ID;

	Server -> AddJag ( ID, Config );

	LastControlMode = Config.Mode;

};

AsynchCANJaguar :: ~AsynchCANJaguar ()
{
	
	Server -> RemoveJag ( ID );

}

void AsynchCANJaguar :: Enable ( double EncoderInitialPosition )
{

	Server -> EnableJag ( ID, EncoderInitialPosition );

};

void AsynchCANJaguar :: Disable ()
{

	Server -> DisableJag ( ID );

};

void AsynchCANJaguar :: Set ( float Speed, uint8_t SyncGroup )
{

	Server -> SetJag ( ID, Speed, SyncGroup );

};

float AsynchCANJaguar :: Get ()
{

	return Server -> GetJag ( ID );

};

float AsynchCANJaguar :: GetPosition ()
{

	return Server -> GetJagPosition ( ID );

};

float AsynchCANJaguar :: GetBusVoltage ()
{

	return Server -> GetJagBusVoltage ( ID );

};

float AsynchCANJaguar :: GetOutputVoltage ()
{

	return Server -> GetJagOutputCurrent ( ID );

};

float AsynchCANJaguar :: GetOutputCurrent ()
{

	return Server -> GetJagOutputCurrent ( ID );

};

void AsynchCANJaguar :: Configure ( CANJagConfigInfo Config )
{

	Server -> ConfigJag ( ID, Config );

	LastControlMode = Config.Mode; 

};

void AsynchCANJaguar :: PIDWrite ( float Speed )
{

	if ( LastControlMode != CANJaguar :: kPercentVbus )
	{

		wpi_setWPIErrorWithContext(IncompatibleMode, "PID only supported in PercentVbus mode");
		
		return;

	}

	Set ( Speed );

};

void AsynchCANJaguar :: UpdateSyncGroup ( CANJaguarServer * Server, uint8_t SyncGroup )
{

	Server -> UpdateJagSyncGroup ( SyncGroup );

};
