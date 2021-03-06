#ifndef SHS_2605_ROBOT_H
#define SHS_2605_ROBOT_H

#include "WPILib.h"

#include "Logging/Logger.h"

#include "CANJagServer/AsynchCANJaguar.h"
#include "CANJagServer/CANJaguarServer.h"

#include "PIC-Servo/PICServo.h"
#include "PIC-Servo/PICServoController.h"
#include "PIC-Servo/PICServoCom.h"
#include "PIC-Servo/AnalogCANJaguarPipeServer.h"

#include "SubSystems/MecanumDrive.h"
#include "SubSystems/ShooterBelts.h"
#include "SubSystems/CollectorArms.h"
#include "SubSystems/ShooterWinch.h"
#include "SubSystems/ArmConfig.h"
#include "SubSystems/WinchConfig.h"

#include "Filters/ExponentialFilter.h"
#include "Filters/DeadbandFilter.h"

#include "Sensors/PIDOutputSensor.h"
#include "Sensors/IRDistanceSensor.h"

#include "Decorations/SHSAnimations.h"

#include "Controls/NumericStepper.h"

#include "Util/Delegate.h"

#include "Behaviors/BehaviorController.h"

#include "TeleopDriveBehavior.h"
#include "EmergenceyArmsBehavior.h"
#include "BallPickupBehavior.h"
#include "AutonomousBehavior.h"

#include "Config/ConfigFile.h"

#include "Sensors/IRDistanceConfig.h"

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

#define ENCODER_CODES_PER_REVOLUTION_360 3075
#define ENCODER_CODES_PER_REVOLUTION_480 3416

#define ROTATION_P 1.0000
#define ROTATION_I 1.0010
#define ROTATION_D 0.0050
#define ROTATION_F 0.0050

#define VISION_PRIORITY 102

#define BELT_P 1.0000
#define BELT_I 0.2500
#define BELT_D 0.2900

#define BELT_ENCODER_CODES_PER_REVOLUTION 2500

// MODIFIED FOR OPEN LOOP
#define BELT_SPEED_SCALE 2500

#define ARM_P 0.0000
#define ARM_I 0.0000
#define ARM_D 0.0000

#define ARM_ENCODER_CODES_PER_REVOLUTION 1080

#define ARM_SPEED_SCALE 100

#define WENCH_SPEED_SCALE 100

#define ROBOT_LOGLEVEL Logger :: LOG_DEBUG

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
	void InitDecorations ();
	void LoadConfiguration ();
	
	void PeriodicCommon ();

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

	void HolyFuck ();

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
	uint32_t DisbCount;

	bool LastSaveButtonState;

	// Robot configuration file

	ConfigFile * RobotConfig;

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

	CANJagConfigInfo WheelConfig360;
	CANJagConfigInfo WheelConfig480;

	ExponentialFilter * DriveFilter;
	DeadbandFilter * StickFilter;

	ClassDelegate <Robot, void> * OnShiftDelegate;
	NumericStepper * GearStepper;
	double GearRPM;

	// Input

	Joystick * StrafeStick;
	Joystick * RotateStick;
	Joystick * CancelStick;

	// Tasks

	Task * AutonomousTask;
	Task * TeleopTask;
	Task * VisionTask;

	// Vision

	AxisCamera * TargetingCamera;
	bool TargetFound;

	// Shooter

	CANJagConfigInfo MasterBeltConfig;
	CANJagConfigInfo SlaveBeltConfig;

	AsynchCANJaguar * BeltL;
	AsynchCANJaguar * BeltR;
	AsynchCANJaguar * BeltLSlave;
	AsynchCANJaguar * BeltRSlave;

	ShooterBelts * Belts;

	// Behaviors

	BehaviorController * Behaviors;

	TeleopDriveBehavior * TeleopDrive;
	EmergenceyArmsBehavior * EmergenceyArms;
	BallPickupBehavior * BallPickup;
	AutonomousBehavior * Autonomous;

	char * TELEOP_DRIVE_BEHAVIOR;
	char * EMERGENCEY_ARMS_BEHAVIOR;
	char * BALL_PICKUP_BEHAVIOR;
	char * AUTONOMOUS_BEHAVIOR;

	// Ball stuff

	AnalogChannel * DistanceSensorAnalog;
	IRDistanceSensor * BallSensor;

	IRDistanceConfig * BallSensorConfig;

	DigitalInput * BallLimit;

	// Arms

	CANJagConfigInfo ArmServoConfig;
	CANJagConfigInfo ArmFreeConfig;

	AsynchCANJaguar * ArmL;
	AsynchCANJaguar * ArmR;

	CollectorArms * Arms;

	ArmConfig * ArmsConfig;

	// Winch

	CANJagConfigInfo WinchServoConfig;
	CANJagConfigInfo WinchFreeConfig;

	AsynchCANJaguar * WinchM;

	ShooterWinch * Winch;

	WinchConfig * WinchConf;

	// TEST STUFF

	uint8_t TestPeriodMode;

	Logger * Log;

	double LowestVoltage;

	double WinchInit;

};

#endif
