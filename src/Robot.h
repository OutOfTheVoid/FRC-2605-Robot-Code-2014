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
#include "SubSystems/ShooterWinch.h"

#include "Filters/ExponentialFilter.h"
#include "Filters/DeadbandFilter.h"

#include "Sensors/PIDOutputSensor.h"
#include "Sensors/IRDistanceSensor.h"

#include "Decorations/SHSAnimations.h"

#include "Controls/NumericStepper.h"

#include "Util/Delegate.h"

#include "Behaviors/BehaviorController.h"

#include "BallPickupBehavior.h"
#include "TeleopDriveBehavior.h"

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

#define BELT_P 3.2000
#define BELT_I 0.0200
#define BELT_D 0.3000

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

	void InitControls ();
	void InitMotors ();
	void InitBehaviors ();
	void InitVision ();
	void InitSensors ();

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

	void OnShift ();
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

	// Operating control variables
	
	RobotMode Mode;

	uint32_t TeleCount;
	uint32_t AutoCount;
	uint32_t TestCount;

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

	ExponentialFilter * DriveFilter;
	DeadbandFilter * StickFilter;

	ClassDelegate <Robot, void> * OnShiftDelegate;
	NumericStepper * GearStepper;
	double GearRPM;

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

	AsynchCANJaguar * BeltL;
	AsynchCANJaguar * BeltR;

	ShooterBelts * Shooter;

	DigitalInput * BallPositionSwitch;

	// Wench

	PICServo * WinchM;

	ShooterWinch * Winch;

	// Arms

	CollectorArms * Arms;

	PICServo * ArmL;
	PICServo * ArmR;

	// PIC-Servo

	PICServoController * PICServoControl;

	// TEST VARIABLES

	bool LastPosSet;

	// LED STUFF

	LEDStrip * LEDS;

	LEDStripAnimator * TestAnimation;

	// Behaviors

	BehaviorController * Behaviors;

	TeleopDriveBehavior * TeleopDrive;
	BallPickupBehavior * BallPickup;

	char * TELEOP_DRIVE_BEHAVIOR;
	char * BALL_PICKUP_BEHAVIOR;

	// Ball stuff

	AnalogChannel * DistanceSensorAnalog;
	IRDistanceSensor * BallSensor;

	// TEST STUFF

	uint8_t TestPeriodMode;

};

START_ROBOT_CLASS ( Robot );

#endif
