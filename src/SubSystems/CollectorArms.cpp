#include "CollectorArms.h"

CollectorArms :: CollectorArms ( PICServo * ML, PICServo * MR )
{

	this -> ML = ML;
	this -> MR = MR;

	LInverted = false;
	RInverted = false;

	PreScale = 1;

	Enabled = false;

	LCalibrated = false;
	RCalibrated = false;

	FreePWM = 1.0;

};

CollectorArms :: ~CollectorArms ()
{
};

bool CollectorArms :: Enable ()
{

	if ( ML == NULL || MR == NULL )
		return false;

	Enabled = true;

	ML -> Enable ();
	MR -> Enable ();

	LastDrivePosition = false;

	ML -> SetControlMode ( PICServo :: kPWM );
	MR -> SetControlMode ( PICServo :: kPWM );

	return true;

};

void CollectorArms :: Disable ()
{

	Enabled = false;

	ML -> Disable ();
	MR -> Disable ();

};

bool CollectorArms :: GetEnabled ()
{

	return Enabled;

};

void CollectorArms :: SetInverted ( bool L, bool R )
{

	if ( Enabled )
		return;

	LInverted = L;
	RInverted = R;

};

void CollectorArms :: SetPreScale ( double PreScale )
{

	if ( Enabled )
		return;

	this -> PreScale = PreScale;

};

void CollectorArms :: SetFreeDrivePower ( double Power )
{

	if ( Enabled )
		return;

	this -> FreePWM = Power;

};

void CollectorArms :: DriveAngle ( double Position )
{

	ML -> SetControlMode ( PICServo :: kPosition );
	MR -> SetControlMode ( PICServo :: kPosition );

	LCalibrated = false;
	RCalibrated = false;

	ML -> Set ( Position * PreScale * ( LInverted ? -1 : 1 ) );
	MR -> Set ( Position * PreScale * ( RInverted ? -1 : 1 ) );

};

void CollectorArms :: DrivePWM ( double Value )
{

	ML -> SetControlMode ( PICServo :: kPWM );
	MR -> SetControlMode ( PICServo :: kPWM );

	MR -> Set ( Value );
	ML -> Set ( Value );

};

bool CollectorArms :: DriveToLimitsAndCalibrate ()
{

	ML -> SetControlMode ( PICServo :: kPWM );
	MR -> SetControlMode ( PICServo :: kPWM );

	bool LimitL = ML -> GetLimit2 ();
	bool LimitR = MR -> GetLimit2 ();

	if ( ! LimitL )
	{

		ML -> Set ( FreePWM * ( LInverted ? 0.7 : - 0.7 ) );
	
	}
	else if ( ! LCalibrated )
	{

		printf ( "DRIVEANGLE_LEFT_CALIB\n" );

		ML -> ResetPosition ();
		ML -> Set ( 0 );

		LCalibrated = true;

	}

	if ( ! LimitR )
	{

		MR -> Set ( FreePWM * ( RInverted ? 0.7 : - 0.7 ) );

	}
	else if ( ! RCalibrated )
	{

		printf ( "DRIVEANGLE_RIGHT_CALIB\n" );

		MR -> ResetPosition ();
		MR -> Set ( 0 );

		RCalibrated = true;

	}

	return LimitR && LimitL;

};

void CollectorArms :: Stop ()
{

	
	MR -> SetControlMode ( PICServo :: kPWM );
	ML -> SetControlMode ( PICServo :: kPWM );

	MR -> Set ( 0 );
	ML -> Set ( 0 );

};

void CollectorArms :: CalibratePICServoAnalogs ()
{

	if ( Enabled )
		return;

	ML -> CalibrateAnalog ();
	MR -> CalibrateAnalog ();

};
