#include "CollectorArms.h"

CollectorArms :: CollectorArms ( PICServo * ML, PICServo * MR )
{

	this -> ML = ML;
	this -> MR = MR;

	LInverted = false;
	RInverted = false;

	PreScale = 1;

	Enabled = false;

	FreePWM = 1.0;

	Log = Logger :: GetInstance ();

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

	ML -> SetControlMode ( PICServo :: kPWM );
	MR -> SetControlMode ( PICServo :: kPWM );

	ML -> Set ( 0 );
	ML -> Set ( 0 );

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

void CollectorArms :: SetZeros ()
{

	ML -> ResetPosition ();
	MR -> ResetPosition ();

};

void CollectorArms :: SetFreeDrivePower ( double Power )
{

	if ( Enabled )
		return;

	this -> FreePWM = Power;

};

bool CollectorArms :: DrivePositions ( double L, double R )
{

	ML -> SetControlMode ( PICServo :: kPosition );
	MR -> SetControlMode ( PICServo :: kPosition );

	ML -> Set ( L );
	MR -> Set ( R );

	return ML -> GetMoveDone () && MR-> GetMoveDone ();

};

bool CollectorArms :: ArmPositionsWithin ( double Threshold, double L, double R )
{

	double LD = fabs ( L - ML -> GetPosition () );
	double RD = fabs ( R - MR -> GetPosition () );

	Log -> Log ( Logger :: LOG_DEBUG, "Left diff: %f, Right diff: %f, Reached: %s\n", LD, RD, ( ( LD < Threshold ) && ( RD < Threshold ) ) ? "True" : "False" );

	return ( ( LD < Threshold ) && ( RD < Threshold ) );

};

void CollectorArms :: DrivePWM ( double Value )
{

	ML -> SetControlMode ( PICServo :: kPWM );
	MR -> SetControlMode ( PICServo :: kPWM );

	MR -> Set ( Value );
	ML -> Set ( Value );

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
