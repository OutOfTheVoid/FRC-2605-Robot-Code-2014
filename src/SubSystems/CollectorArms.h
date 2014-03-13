#ifndef SHS_2605_COLLECTOR_ARMS_H
#define SHS_2605_COLLECTOR_ARMS_H

#include "src/PIC-Servo/PICServo.h"

#include "src/Logging/Logger.h"

#include <cmath>

class CollectorArms
{
public:

	CollectorArms ( PICServo * ML, PICServo * MR );
	~CollectorArms ();

	bool Enable ();
	void Disable ();
	bool GetEnabled ();

	void SetPreScale ( double PreScale );
	void SetInverted ( bool L, bool R );

	void SetZeros ();

	bool DrivePositions ( double L, double R );

	void SetFreeDrivePower ( double Power );
	void DrivePWM ( double Value );

	bool ArmPositionsWithin ( double Threshold, double L, double R );

	void Stop ();

	void CalibratePICServoAnalogs ();

	typedef enum 
	{

		ARMS_OFF,
		ARMS_BEGIN_1,
		ARMS_BEGIN_2,
		ARMS_OPEN,
		ARMS_CLOSED,

	} ArmsPreset;

private:

	PICServo * ML;
	PICServo * MR;

	double PreScale;

	double FreePWM;

	bool Enabled;

	bool LInverted, RInverted;

	Logger * Log;

};

#endif
