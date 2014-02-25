#include "ShooterWench.h"

ShooterWench :: ShooterWench ( SpeedController * WenchMotor )
{

	Enabled = false;

	Direction = true;
	Running = false;
	Inverted = false;

	Scale = 1;

	Motor = WenchMotor;

};

ShooterWench :: ~ShooterWench ()
{
};

void ShooterWench :: SetMotor ( SpeedController * WenchMotor )
{

	if ( Enabled )
		return;

	Motor = WenchMotor;

};

void ShooterWench :: SetInverted ( bool Inverted )
{

	if ( Enabled )
		return;

	this -> Inverted = Inverted;

};

void ShooterWench :: SetMotorScale ( double Scale )
{

	if ( Enabled )
		return;

	this -> Scale = Scale;

};

bool ShooterWench :: Enable ()
{

	if ( Motor == NULL )
		return false;

	Enabled = true;

	return true;

};

void ShooterWench :: Disable ()
{

	Enabled = false;

	PushSpeed ();

};

bool ShooterWench :: GetEnabled ()
{

	return Enabled;

};

void ShooterWench :: Open ()
{

	Direction = true;
	Running = true;

};

void ShooterWench :: Close ()
{

	Direction = false;
	Running = true;

};

void ShooterWench :: Stop ()
{

	Running = false;

};

void ShooterWench :: PushSpeed ()
{

	if ( ( ! Enabled ) || ( ! Running ) )
	{

		Motor -> Set ( 0 );
		return;

	}

	Motor -> Set ( ( Direction != Inverted ) ? Scale : - Scale );

};
