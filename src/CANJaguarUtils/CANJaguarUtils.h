#ifndef SHS_2605_CANJAGUAR_UTILS_H
#define SHS_2605_CANJAGUAR_UTILS_H

/*
* Copyright (C) 2014 Liam Taylor
* FRC Team Sehome Semonsters 2605
*/

#include "WPILib.h"

typedef struct CANJagConfigInfo
{
	
	CANJagConfigInfo ();
	
	CANJaguar :: ControlMode Mode;
	CANJaguar :: PositionReference PosRef;
	CANJaguar :: SpeedReference SpeedRef;
	CANJaguar :: NeutralMode NeutralAction;
	CANJaguar :: LimitMode Limiting;
	
	double LowPosLimit;
	double HighPosLimit;
	
	double MaxVoltage;
	
	double P;
	double I;
	double D;
	
	UINT16 EncoderLinesPerRev;
	UINT16 PotentiometerTurnsPerRev;
	
	bool Safety;
	
} CANJagConfigInfo;

typedef int32_t CAN_ID; 

void ConfigCANJaguar ( CANJaguar * Jag, CANJagConfigInfo Conf );
void CheckCANJaguar ( CANJaguar * Jag, CANJagConfigInfo Conf );

#endif
