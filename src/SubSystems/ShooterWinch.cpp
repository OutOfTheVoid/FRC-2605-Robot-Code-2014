#include "ShooterWinch.h"

ShooterWinch :: ShooterWinch ( AsynchCANJaguar * Motor, CANJagConfigInfo ServoConfig, CANJagConfigInfo OpenConfig )
{

	M = Motor;

	Inverted = false;
	Enabled = false;

	Zero = 0.0;

	this -> ServoConfig = ServoConfig;
	this -> OpenConfig = OpenConfig;

	PreScale = 1.0;

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

void ShooterWinch :: SetPreScale ( double PreScale )
{

	if ( Enabled )
		return;

	this -> PreScale = PreScale;

};

void ShooterWinch :: SetZero ()
{

	Zero = M -> GetPosition ();

};

bool ShooterWinch :: Enable ()
{

	if ( M == NULL )
		return false;

	M -> Configure ( ServoConfig );
	M -> Enable ();

	Enabled = true;

	return true;

};

void ShooterWinch :: Disable ()
{

	Enabled = false;

	M -> Disable ();
	M -> Configure ( OpenConfig );
	M -> Set ( 0.0 );

};

bool ShooterWinch :: GetEnabled ()
{

	return Enabled;

};

bool ShooterWinch :: WithinAngle ( double Threshold, double Angle )
{

	double D = fabs ( Angle - ( M -> GetPosition () - Zero ) / PreScale );

	return D < Threshold;

};

void ShooterWinch :: DriveAngle ( double Angle )
{

	M -> Set ( ( Angle * ( Inverted ? - PreScale : PreScale ) ) + Zero );

};

double ShooterWinch :: GetAngle ()
{

	return ( M -> GetPosition () - Zero ) / ( Inverted ? - PreScale : PreScale );

};

void ShooterWinch :: Stop ()
{

	M -> Set ( M -> GetPosition () );

};
