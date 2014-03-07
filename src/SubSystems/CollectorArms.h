#ifndef SHS_2605_COLLECTOR_ARMS_H
#define SHS_2605_COLLECTOR_ARMS_H

#include "src/PIC-Servo/PICServo.h"

class CollectorArms
{
public:

	CollectorArms ( PICServo * ML, PICServo * MR );
	~CollectorArms ();

	bool Enable ();
	void Disable ();
	bool GetEnabled ();

	void SetInverted ( bool L, bool R );

	void SetPreScale ( double PreScale );

	void SetFreeDrivePower ( double Power );

	void DrivePWM ( double Value );
	void DriveAngle ( double Position );
	bool DriveToLimitsAndCalibrate ();

	void Stop ();

	void CalibratePICServoAnalogs ();

private:

	PICServo * ML;
	PICServo * MR;

	double PreScale;

	double FreePWM;

	bool Enabled;

	bool LastDrivePosition;

	bool LInverted, RInverted;

	bool LCalibrated, RCalibrated;

};

#endif
