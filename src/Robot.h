#ifndef SHS_2605_ROBOT_H
#define SHS_2605_ROBOT_H

#include "WPILib.h"

#include "CANJagServer/AsynchCANJaguar.h"
#include "CANJagServer/CANJaguarServer.h"

#include "PIC-Servo/PICServo.h"
#include "PIC-Servo/PICServoController.h"
#include "PIC-Servo/PICServoCom.h"
#include "PIC-Servo/AnalogCANJaguarPipeServer.h"

#include "SubSystems/MecanumDrive.h"
#include "SubSystems/ShooterBelts.h"
#include "SubSystems/ShooterWench.h"

#include "Filters/ExponentialFilter.h"
#include "Filters/DeadbandFilter.h"

#include "Sensors/PIDOutputSensor.h"

#define DRIVE_RESPONSE_CURVE 2.0

#define SPEED_SCALE_GEAR1 80
#define SPEED_SCALE_GEAR2 280
#define SPEED_SCALE_GEAR3 700

#define P_GEAR1 1.0000
#define I_GEAR1 0.0090
#define D_GEAR1 0.0050

#define P_GEAR2 0.9800
#define I_GEAR2 0.0095
#define D_GEAR2 0.0020

#define P_GEAR3 0.9500
#define I_GEAR3 0.0250
#define D_GEAR3 0.0010

#define ENCODER_CODES_PER_REVOLUTION 3075

#define ROTATION_P 1.0000
#define ROTATION_I 1.0010
#define ROTATION_D 0.0050
#define ROTATION_F 0.0050

#define VISION_PRIORITY 102

#define BELT_P 1.2000
#define BELT_I 0.0100
#define BELT_D 0.0010

#define BELT_ENCODER_CODES_PER_REVOLUTION 1024

#define BELT_SPEED_SCALE 2500

#define ARM_P 0.0000
#define ARM_I 0.0000
#define ARM_D 0.0000

#define ARM_ENCODER_CODES_PER_REVOLUTION 1080

#define ARM_SPEED_SCALE 100

#define WENCH_SPEED_SCALE 100

class Robot : public IterativeRobot
{
public:

	Robot ();
	~Robot ();

	void DisabledInit ();
	void DisabledPeriodic ();
	void DisabledEnd ();

	void TeleopInit ();
	void TeleopPeriodic ();
	void TeleopEnd ();

	void AutonomousInit ();
	void AutonomousPeriodic ();
	void AutonomousEnd ();

	void TestInit ();
	void TestPeriodic ();
	void TestEnd ();

	void AutonomousTaskRoutine ();
	void TeleopTaskRoutine ();
	void VisionTaskRoutine ();

	static int AutonomousTaskStub ( Robot * This );
	static int TeleopTaskStub ( Robot * This );
	static int VisionTaskStub ( Robot * This );

	void ShiftVGear ( uint8_t Gear );

	typedef enum
	{

		RobotStartMode = 0,
		DisabledMode,
		AutonomousMode,
		TeleopMode,
		TestMode,

	} RobotMode;

private:

	// Operating mode used to call *End methods
	
	RobotMode Mode;

	// Driver station readout
	
	DriverStationLCD * DsLcd;

	// Jaguar Server

	CANJaguarServer * JagServer;

	// Mecanum Drive

	AsynchCANJaguar * WheelFL;
	AsynchCANJaguar * WheelFR;
	AsynchCANJaguar * WheelRL;
	AsynchCANJaguar * WheelRR;

	MecanumDrive * Drive;

	CANJagConfigInfo WheelConfig;

	Gyro * RotationGyro;
	PIDController * RotationController;
	PIDOutputSensor * RotationValue;

	ExponentialFilter * DriveFilter;
	DeadbandFilter * StickFilter;

	uint8_t Gear;
	double GearRPM;

	bool GearUpPreState;
	bool GearDownPreState;

	// Input

	Joystick * StrafeStick;
	Joystick * RotateStick;
	Joystick * ShootStick;

	// Tasks

	Task * AutonomousTask;
	Task * TeleopTask;
	Task * VisionTask;

	// Vision

	AxisCamera * TargetingCamera;

	bool TargetFound;

	// Shooter

	CANJagConfigInfo BeltConfig;
	CANJagConfigInfo ArmConfig;

	AsynchCANJaguar * BeltL;
	AsynchCANJaguar * BeltR;

	AsynchCANJaguar * ArmL;
	AsynchCANJaguar * ArmR;

	ShooterBelts * Shooter;

	double IntendedArmPosition;

	// Wench

	CANJagConfigInfo WenchConfig;

	ShooterWench * Wench;
	AsynchCANJaguar * WenchM;

	// PIC-Servo

	AnalogCANJaguarPipeServer * PServer;

	PICServoController * PICServoControl;

	PICServo * TestPICServo;

	PICServoCom * SCom;

	// TEST VARIABLES

	bool LastPosSet;

};

START_ROBOT_CLASS ( Robot );

#endif
