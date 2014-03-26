#include "CANJaguarUtils.h"

/*
* Copyright (C) 2014 Liam Taylor
* FRC Team Sehome Semonsters 2605
*/

CANJagConfigInfo :: CANJagConfigInfo ()
{
	
	Mode = CANJaguar :: kVoltage;
	NeutralAction = CANJaguar :: kNeutralMode_Coast;

	SpeedRef = CANJaguar :: kSpeedRef_None;
	PosRef = CANJaguar :: kPosRef_None;

	Limiting = CANJaguar :: kLimitMode_SwitchInputsOnly;
	
	MaxVoltage = 13;
	Safety = false;
	
	FaultTime = 1.0;

	VRamp = 190.476190476;

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
		if ( Conf.Limiting = CANJaguar :: kLimitMode_SoftPositionLimits )
			Jag -> ConfigSoftPositionLimits ( Conf.HighPosLimit, Conf.LowPosLimit );
		else if ( Conf.Limiting == CANJaguar :: kLimitMode_SwitchInputsOnly )
			Jag -> DisableSoftPositionLimits ();
		break;
		
	case CANJaguar :: kSpeed:
		Jag -> ChangeControlMode ( CANJaguar :: kSpeed );
		Jag -> ConfigEncoderCodesPerRev ( Conf.EncoderLinesPerRev );
		Jag -> SetPID ( Conf.P, Conf.I, Conf.D );
		break;
		
	default:
		Jag -> ChangeControlMode ( CANJaguar :: kVoltage );
		break;
		
	}

	Jag -> SetPositionReference ( Conf.PosRef );
	Jag -> SetSpeedReference ( Conf.SpeedRef );

	if ( Conf.PosRef == CANJaguar :: kPosRef_QuadEncoder )
		Jag -> ConfigEncoderCodesPerRev ( Conf.EncoderLinesPerRev );
	else if ( Conf.PosRef == CANJaguar :: kPosRef_Potentiometer )
		Jag -> ConfigPotentiometerTurns ( Conf.PotentiometerTurns );

	Jag -> ConfigFaultTime ( Conf.FaultTime );
	Jag -> ConfigMaxOutputVoltage ( Conf.MaxVoltage );
	Jag -> ConfigNeutralMode ( Conf.NeutralAction );
	Jag -> SetSafetyEnabled ( Conf.Safety );
	Jag -> SetVoltageRampRate ( Conf.VRamp );
	Jag -> EnableControl ();
	
};

void CheckCANJaguar ( CANJaguar * Jag, CANJagConfigInfo Conf )
{
	
	if ( Jag -> GetControlMode () != Conf.Mode || Jag -> GetPowerCycled () )
	{

		ConfigCANJaguar ( Jag, Conf );

	}
	
};
