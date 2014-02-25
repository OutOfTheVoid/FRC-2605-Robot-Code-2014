#include "ShooterBelts.h"

ShooterBelts :: ShooterBelts ( SpeedController * RollerBL, SpeedController * RollerBR, SpeedController * ArmSL, SpeedController * ArmSR )
{

	RollerL.Motor = RollerBL;
	RollerR.Motor = RollerBR;
	RollerL.Inverted = false;
	RollerR.Inverted = false;

	ArmL.Motor = ArmSL;
	ArmR.Motor = ArmSR;
	ArmL.Inverted = false;
	ArmR.Inverted = false;

	ScaleR = 1;
	ScaleA = 1;

	PrescaleR = 1;
	PrescaleA = 1;

	Enabled = false;

};

ShooterBelts :: ~ShooterBelts ()
{

	Disable ();

};

void ShooterBelts :: SetMotors ( SpeedController * RollerBL, SpeedController * RollerBR, SpeedController * ArmSL, SpeedController * ArmSR )
{

	if ( Enabled )
		return;

	RollerL.Motor = RollerBL;
	RollerR.Motor = RollerBR;
	RollerL.Inverted = false;
	RollerR.Inverted = false;

	ArmL.Motor = ArmSL;
	ArmR.Motor = ArmSR;
	ArmL.Inverted = false;
	ArmR.Inverted = false;

};

void ShooterBelts :: SetInverted ( bool BL, bool BR, bool SL, bool SR )
{

	if ( Enabled )
		return;

	RollerL.Inverted = BL;
	RollerR.Inverted = BR;
	ArmL.Inverted = SL;
	ArmR.Inverted = SR;

};

void ShooterBelts :: SetBeltPreScale ( double Prescale )
{

	PrescaleR = Prescale;

};

void ShooterBelts :: SetArmPreScale ( double Prescale )
{

	PrescaleA = Prescale;

};

void ShooterBelts :: SetBeltMotorScale ( double Scale )
{

	ScaleR = Scale;

};

void ShooterBelts :: SetArmMotorScale ( double Scale )
{

	ScaleA = Scale;

};

void ShooterBelts :: SetArmPosition ( double Position )
{

	if ( ! Enabled )
		return;

	A = Position * PrescaleA;

};

void ShooterBelts :: SetBeltSpeed ( double Speed )
{

	if ( ! Enabled )
		return;

	S = Speed * PrescaleR;

};

bool ShooterBelts :: Enable ()
{

	if ( RollerL.Motor == NULL || RollerR.Motor == NULL || ArmL.Motor == NULL || ArmR.Motor == NULL )
		return false;

	S = 0;
	A = 0;

	Enabled = true;

	return true;

};

void ShooterBelts :: PushSpeeds ()
{

	if ( ! Enabled )
	{

		RollerL.Motor -> Set ( 0 );
		RollerR.Motor -> Set ( 0 );

		ArmL.Motor -> Set ( 0 );
		ArmR.Motor -> Set ( 0 );

		return;

	}

	RollerL.Motor -> Set ( ( RollerL.Inverted ? S : - S ) * ScaleR );
	RollerR.Motor -> Set ( ( RollerR.Inverted ? S : - S ) * ScaleR );

	ArmL.Motor -> Set ( ( ArmL.Inverted ? A : - A ) * ScaleA );
	ArmR.Motor -> Set ( ( ArmR.Inverted ? A : - A ) * ScaleA );

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
