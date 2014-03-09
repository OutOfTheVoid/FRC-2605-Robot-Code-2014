#include "ShooterWinch.h"

ShooterWinch :: ShooterWinch ( PICServo * Motor )
{

	M = Motor;

	PreScale = 1;
	Inverted = false;

	PreviousAllowSlop = false;

};

ShooterWinch :: ~ShooterWinch ()
{
};

void ShooterWinch :: SetInverted ( bool Inverted )
{

	if ( Enabled )
		return;

	this -> Inverted = Inverted;

};

void ShooterWinch :: SetPreScale ( double Scale )
{

	if ( Enabled )
		return;

	this -> PreScale = Scale;

};

bool ShooterWinch :: Enable ()
{

	if ( M == NULL )
		return false;

	M -> Enable ();

	Enabled = true;

	return true;

};

void ShooterWinch :: Disable ()
{

	Enabled = false;

	M -> Disable ();

};

bool ShooterWinch :: GetEnabled ()
{

	return Enabled;

};

bool ShooterWinch :: DriveAngle ( double Angle )
{

	M -> SetControlMode ( PICServo :: kPosition );
	M -> Set ( Angle * PreScale * ( Inverted ? -1 : 1 ) );

	return M -> GetMoveDone ();

};

void ShooterWinch :: DrivePWM ( double Value )
{

	M -> SetControlMode ( PICServo :: kPWM );
	M -> Set ( Value );

};

void ShooterWinch :: Stop ()
{

	M -> SetControlMode ( PICServo :: kPWM );
	M -> Set ( 0 );

};
