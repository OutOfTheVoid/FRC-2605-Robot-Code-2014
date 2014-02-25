#include "MecanumDrive.h"
#include <math.h>

/*
* Copyright (C) 2014 Liam Taylor
* WheelFRC Team Sehome Semonsters 2605
*/

/**
* Constructor
*
* @param WheelFL Front-left wheel speed controller
* @param WheelFR Front-right wheel speed controller
* @param WheelRL Rear-left wheel speed controller
* @param WheelRR Rear-right wheel speed controller
*/
MecanumDrive :: MecanumDrive ( SpeedController * WheelFL, SpeedController * WheelFR, SpeedController * WheelRL, SpeedController * WheelRR )
{
	
	MotorFL.Motor = WheelFL;
	MotorFR.Motor = WheelFR;
	MotorRL.Motor = WheelRL;
	MotorRR.Motor = WheelRR;
	
	MotorFL.Inverted = false;
	MotorFR.Inverted = false;
	MotorRL.Inverted = false;
	MotorRR.Inverted = false;
	
	TX = 0;
	TY = 0;
	TR = 0;
	
	PrescaleT = 1;
	PrescaleR = 1;
	
	Scale = 1;
	
	SineInverted = false;

	Enabled = false;
	
};

MecanumDrive :: ~MecanumDrive () {};

void MecanumDrive :: SetMotors ( SpeedController * WheelFL, SpeedController * WheelFR, SpeedController * WheelRL, SpeedController * WheelRR )
{
	
	if ( Enabled )
		return;

	MotorFL.Motor = WheelFL;
	MotorFR.Motor = WheelFR;
	MotorRL.Motor = WheelRL;
	MotorRR.Motor = WheelRR;
	
};

void MecanumDrive :: SetInverted ( bool WheelFL, bool WheelFR, bool WheelRL, bool WheelRR )
{
	
	if ( Enabled )
		return;

	MotorFL.Inverted = WheelFL;
	MotorFR.Inverted = WheelFR;
	MotorRL.Inverted = WheelRL;
	MotorRR.Inverted = WheelRR;
	
};

bool MecanumDrive :: Enable ()
{
	
	if ( MotorFL.Motor == NULL || MotorFR.Motor == NULL || MotorRL.Motor == NULL || MotorRR.Motor == NULL )
		return false;

	Enabled = true;
	return true;
	
};

void MecanumDrive :: Disable ()
{
	
	Enabled = false;
	
	TX = 0;
	TY = 0;
	TR = 0;
	
	PushTransform ();
	
};

bool MecanumDrive :: GetEnabled ()
{
	
	return Enabled;
	
};

void MecanumDrive :: PushTransform ()
{
	
	double ForceAngle;
	double ForceMagnitude;
	double SinCalc;
	double CosCalc;
	
	if ( ! Enabled )
	{
		
		MotorFL.Motor -> Set ( 0 );
		MotorFR.Motor -> Set ( 0 );
		MotorRL.Motor -> Set ( 0 );
		MotorRR.Motor -> Set ( 0 );
		
		return;
	
	}
		
	ForceMagnitude = sqrt ( TX * TX + TY * TY );
	
	ForceAngle = atan2 ( TX, TY );
	ForceAngle += PI_Div_4;
	
	SinCalc = sin ( ForceAngle ) * ForceMagnitude;
	CosCalc = cos ( ForceAngle ) * ForceMagnitude;

	MotorFL.Motor -> Set ( ( ( SineInverted ? CosCalc : SinCalc ) + TR ) * ( MotorFL.Inverted ? - Scale : Scale ) );
	MotorFR.Motor -> Set ( ( ( SineInverted ? SinCalc : CosCalc ) - TR ) * ( MotorFR.Inverted ? - Scale : Scale ) );
	MotorRL.Motor -> Set ( ( ( SineInverted ? SinCalc : CosCalc ) + TR ) * ( MotorRL.Inverted ? - Scale : Scale ) );
	MotorRR.Motor -> Set ( ( ( SineInverted ? CosCalc : SinCalc ) - TR ) * ( MotorRR.Inverted ? - Scale : Scale ) );
	
};

void MecanumDrive :: DebugValues ()
{
	
	double ForceAngle;
	double ForceMagnitude;
	double SinCalc;
	double CosCalc;
	
	double FL, FR, RL, RR;
	
	ForceMagnitude = sqrt ( TX * TX + TY * TY );
	
	ForceAngle = atan2 ( TX, TY );
	ForceAngle += PI_Div_4;
	
	SinCalc = sin ( ForceAngle ) * ForceMagnitude;
	CosCalc = cos ( ForceAngle ) * ForceMagnitude;
	
	FL = ( ( SineInverted ? CosCalc : SinCalc ) + TR ) * ( MotorFL.Inverted ? - Scale : Scale );
	FR = ( ( SineInverted ? SinCalc : CosCalc ) - TR ) * ( MotorFR.Inverted ? - Scale : Scale );
	RL = ( ( SineInverted ? SinCalc : CosCalc ) + TR ) * ( MotorRL.Inverted ? - Scale : Scale );
	RR = ( ( SineInverted ? CosCalc : SinCalc ) - TR ) * ( MotorRR.Inverted ? - Scale : Scale );
	
	printf ( "[ Mecanum Drive Debug ]\n%s\nInput X: %4.4f\nInput Y: %4.4f\nInput R: %4.4f\n[%+4.4f]---[%+4.4f]\n   |         |   \n   |         |   \n   |         |   \n   |         |   \n   |         |   \n   |         |   \n[%+4.4f]---[%+4.4f]\n", ( Enabled ? "Enabled." : "Disabled." ), TX, TY, TR, FL, FR, RL, RR );
	
};

void MecanumDrive :: SetMotorScale ( double s )
{
	
	Scale = s;
	
};

double MecanumDrive :: GetMotorScale ()
{
	
	return Scale;
	
};

void MecanumDrive :: SetPreScale ( double Translation, double Rotation )
{

	PrescaleT = Translation;
	PrescaleR = Rotation;
	
};

double MecanumDrive :: GetPreScaleRotation ()
{
	
	return PrescaleR;
	
};

double MecanumDrive :: GetPreScaleTranslation ()
{
	
	return PrescaleT;
	
};

void MecanumDrive :: SetTranslation ( double X, double Y )
{
	
	TX = X * SQRT_2 * PrescaleT;
	TY = Y * PrescaleT;
	
};

void MecanumDrive :: SetRotation ( double R )
{
	
	TR = R * PrescaleR;
	
};

void MecanumDrive :: SetSineInversion ( bool SineInverted = false )
{
	
	this -> SineInverted = SineInverted;
	
};
