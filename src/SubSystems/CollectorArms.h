#ifndef SHS_2605_COLLECTOR_ARMS_H
#define SHS_2605_COLLECTOR_ARMS_H

#include "src/CANJagServer/AsynchCANJaguar.h"

#include "src/Logging/Logger.h"

#include <cmath>

class CollectorArms
{
public:

	CollectorArms ( AsynchCANJaguar * ArmL, AsynchCANJaguar * ArmR );
	~CollectorArms ();

	bool Enable ();
	void Disable ();
	bool GetEnabled ();

	void SetPreScale ( double PreScale );
	void SetInverted ( bool L, bool R );

	void SetZeros ();

	void DrivePositions ( double L, double R );

	bool ArmPositionsWithin ( double Threshold, double L, double R );

	double GetPositionLeft ();
	double GetPositionRight ();

	void Stop ();

private:

	AsynchCANJaguar * ArmL, * ArmR;

	double PreScale;

	double ZeroL, ZeroR;

	bool Enabled;

	bool InvertedL, InvertedR;

	Logger * Log;

};

#endif
