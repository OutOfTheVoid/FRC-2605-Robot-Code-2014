#include "ShooterBelts.h"

ShooterBelts :: ShooterBelts ( SpeedController * RollerBL, SpeedController * RollerBR )
{

	RollerL.Motor = RollerBL;
	RollerR.Motor = RollerBR;
	RollerL.Inverted = false;
	RollerR.Inverted = false;

	Scale = 1;

	Prescale = 1;

	Enabled = false;

};

ShooterBelts :: ~ShooterBelts ()
{

	Disable ();

};

void ShooterBelts :: SetMotors ( SpeedController * RollerBL, SpeedController * RollerBR )
{

	if ( Enabled )
		return;

	RollerL.Motor = RollerBL;
	RollerR.Motor = RollerBR;
	RollerL.Inverted = false;
	RollerR.Inverted = false;

};

void ShooterBelts :: SetInverted ( bool BL, bool BR )
{

	if ( Enabled )
		return;

	RollerL.Inverted = BL;
	RollerR.Inverted = BR;

};

void ShooterBelts :: SetPreScale ( double Prescale )
{

	this -> Prescale = Prescale;

};

void ShooterBelts :: SetMotorScale ( double Scale )
{

	this ->  Scale = Scale;

};

void ShooterBelts :: SetSpeed ( double Speed )
{

	if ( ! Enabled )
		return;

	S = Speed * Prescale;

};

bool ShooterBelts :: Enable ()
{

	if ( RollerL.Motor == NULL || RollerR.Motor == NULL )
		return false;

	S = 0;

	Enabled = true;

	return true;

};

void ShooterBelts :: PushSpeeds ()
{

	if ( ! Enabled )
	{

		/*RollerL.Motor -> Set ( 0 );
		RollerR.Motor -> Set ( 0 );*/

		RollerL.Motor -> Disable ();
		RollerR.Motor -> Disable ();

		return;

	}

	RollerL.Motor -> Set ( ( RollerL.Inverted ? S : - S ) * Scale );
	RollerR.Motor -> Set ( ( RollerR.Inverted ? S : - S ) * Scale );

};

void ShooterBelts :: Disable ()
{

	Enabled = false;

	PushSpeeds ();

};
	
bool ShooterBelts :: GetEnabled ()
{

	return Enabled;

};
