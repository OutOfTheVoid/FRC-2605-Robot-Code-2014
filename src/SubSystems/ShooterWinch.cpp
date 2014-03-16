#include "ShooterWinch.h"

ShooterWinch :: ShooterWinch ( PICServo * Motor )
{

	M = Motor;

	Inverted = false;
	Enabled = false;

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

void ShooterWinch :: SetZero ()
{

	M -> ResetPosition ();

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

bool ShooterWinch :: WithinAngle ( double Threshold, double Angle )
{

	double D = fabs ( Angle - M -> GetPosition () );

	return D < Threshold;

};

bool ShooterWinch :: DriveAngle ( double Angle )
{

	M -> SetControlMode ( PICServo :: kPosition );
	M -> Set ( Angle * ( Inverted ? -1 : 1 ) );

	return M -> GetMoveDone ();

};

double ShooterWinch :: GetAngle ()
{

	return M -> GetPosition ();

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
