#include "CollectorArms.h"

CollectorArms :: CollectorArms ( AsynchCANJaguar * ArmL, AsynchCANJaguar * ArmR )
{

	this -> ArmL = ArmL;
	this -> ArmR = ArmR;

	ZeroL = 0;
	ZeroR = 0;

	InvertedL = false;
	InvertedR = false;

	Enabled = false;

};

CollectorArms :: ~CollectorArms ()
{

	if ( GetEnabled () )
		Disable ();

};

bool CollectorArms :: Enable ()
{

	if ( ArmL == NULL || ArmR == NULL )
		return false;

	Enabled = true;

	ArmL -> Enable ();
	ArmR -> Enable ();

	return true;

};

void CollectorArms :: Disable ()
{

	Enabled = false;

	ArmL -> Disable ();
	ArmR -> Disable ();

};

bool CollectorArms :: GetEnabled ()
{

	return Enabled;

};

void CollectorArms :: SetInverted ( bool L, bool R )
{

	if ( Enabled )
		return;

	InvertedL = L;
	InvertedR = R;

};

void CollectorArms :: SetPreScale ( double PreScale )
{

	if ( Enabled )
		return;

	this -> PreScale = PreScale;

};

void CollectorArms :: SetZeros ()
{

	ZeroL = ArmL -> GetPosition ();
	ZeroR = ArmR -> GetPosition ();

};

void CollectorArms :: DrivePositions ( double L, double R )
{

	if ( ! Enabled )
		return;

	ArmL -> Set ( ZeroL + ( L * ( InvertedL ? - 1.0 : 1.0 ) ) );
	ArmR -> Set ( ZeroR + ( R * ( InvertedR ? - 1.0 : 1.0 ) ) );

};

bool CollectorArms :: ArmPositionsWithin ( double Threshold, double L, double R )
{

	double LD = fabs ( L - ( ArmL -> GetPosition () - ZeroL ) );
	double RD = fabs ( R - ( ArmR -> GetPosition () - ZeroR ) );

	Log -> Log ( Logger :: LOG_DEBUG, "Left diff: %f, Right diff: %f, Reached: %s\n", LD, RD, ( ( LD < Threshold ) && ( RD < Threshold ) ) ? "True" : "False" );

	return ( ( LD < Threshold ) && ( RD < Threshold ) );

};

void CollectorArms :: Stop ()
{

	
	ArmL -> Set ( ArmL -> GetPosition () );
	ArmR -> Set ( ArmR -> GetPosition () );

};
