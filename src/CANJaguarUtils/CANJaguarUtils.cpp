#include "CANJaguarUtils.h"

/*
* Copyright (C) 2014 Liam Taylor
* FRC Team Sehome Semonsters 2605
*/

CANJagConfigInfo :: CANJagConfigInfo ()
{
	
	Mode = CANJaguar :: kVoltage;
	NeutralAction = CANJaguar :: kNeutralMode_Coast;

	Limiting = CANJaguar :: kLimitMode_SwitchInputsOnly;
	
	MaxVoltage = 13;
	Safety = false;
	
	FaultTime = 1.0;

};

void ConfigCANJaguar ( CANJaguar * Jag, CANJagConfigInfo Conf )
{
	
	Jag -> DisableControl ();
	
	switch ( Conf.Mode )
	{
		
	case CANJaguar :: kPercentVbus:
		Jag -> ChangeControlMode ( CANJaguar :: kPercentVbus );
		break;
		
	case CANJaguar :: kVoltage:
		Jag -> ChangeControlMode ( CANJaguar :: kVoltage );
		break;
		
	case CANJaguar :: kCurrent:
		Jag -> ChangeControlMode ( CANJaguar :: kCurrent );
		break;
		
	case CANJaguar :: kPosition:
		Jag -> ChangeControlMode ( CANJaguar :: kPosition );
		Jag -> SetPID ( Conf.P, Conf.I, Conf.D );
		Jag -> SetPositionReference ( Conf.PosRef );
		if ( Conf.PosRef == CANJaguar :: kPosRef_QuadEncoder )
			Jag -> ConfigEncoderCodesPerRev ( Conf.EncoderLinesPerRev );
		else
			Jag -> ConfigPotentiometerTurns ( Conf.PotentiometerTurnsPerRev );
		if ( Conf.Limiting = CANJaguar :: kLimitMode_SoftPositionLimits )
			Jag -> ConfigSoftPositionLimits ( Conf.HighPosLimit, Conf.LowPosLimit );
		else if ( Conf.Limiting == CANJaguar :: kLimitMode_SwitchInputsOnly )
			Jag -> DisableSoftPositionLimits ();
		break;
		
	case CANJaguar :: kSpeed:
		Jag -> ChangeControlMode ( CANJaguar :: kSpeed );
		Jag -> SetSpeedReference ( Conf.SpeedRef );
		Jag -> ConfigEncoderCodesPerRev ( Conf.EncoderLinesPerRev );
		Jag -> SetPID ( Conf.P, Conf.I, Conf.D );
		break;
		
	default:
		Jag -> ChangeControlMode ( CANJaguar :: kVoltage );
		break;
		
	}

	Jag -> ConfigFaultTime ( Conf.FaultTime );
	Jag -> ConfigMaxOutputVoltage ( Conf.MaxVoltage );
	Jag -> ConfigNeutralMode ( Conf.NeutralAction );
	Jag -> SetSafetyEnabled ( Conf.Safety );
	Jag -> EnableControl ();
	
};

void CheckCANJaguar ( CANJaguar * Jag, CANJagConfigInfo Conf )
{
	
	if ( Jag -> GetControlMode () != Conf.Mode )
	{

		printf ( "CAN_JAGUAR RECONFIGURATION\n" );
		ConfigCANJaguar ( Jag, Conf );


	}
	
};
