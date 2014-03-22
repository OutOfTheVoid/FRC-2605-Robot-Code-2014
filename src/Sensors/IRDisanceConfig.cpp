#include "IRDistanceConfig.h"

IRDistanceConfig :: IRDistanceConfig ( IRDistanceSensor * Sensor ):
	ConfigSection ( "IR" )
{

	this -> Sensor = Sensor;

};

IRDistanceConfig :: ~IRDistanceConfig ()
{



};

void IRDistanceConfig :: LoadSensorCalibration ()
{

	json_t * CalibInLow = json_object_get ( SectionNode, "Calib_In_Low" );
	json_t * CalibInHigh = json_object_get ( SectionNode, "Calib_In_High" );

	json_t * CalibOutLow = json_object_get ( SectionNode, "Calib_Out_Low" );
	json_t * CalibOutHigh = json_object_get ( SectionNode, "Calib_Out_High" );

	Sensor -> InAttenuator -> SetRangeIn ( range_t ( json_real_value ( CalibInLow ), json_real_value ( CalibInHigh ) ) );
	Sensor -> OutAttenuator -> SetRangeOut ( range_t ( json_real_value ( CalibOutLow ), json_real_value ( CalibOutHigh ) ) );

};

void IRDistanceConfig :: SetSensorCalibration ()
{

	json_t * CalibInLow = json_real ( Sensor -> InAttenuator -> GetRangeIn ().Low );
	json_t * CalibInHigh = json_real ( Sensor -> InAttenuator -> GetRangeIn ().High );

	json_t * CalibOutLow = json_real ( Sensor -> InAttenuator -> GetRangeOut ().Low );
	json_t * CalibOutHigh = json_real ( Sensor -> OutAttenuator -> GetRangeOut ().High );

	json_object_set ( SectionNode, "Calib_In_Low", CalibInLow );
	json_object_set ( SectionNode, "Calib_In_High", CalibInHigh );
	json_object_set ( SectionNode, "Calib_Out_Low", CalibOutLow );
	json_object_set ( SectionNode, "Calib_Out_High", CalibOutHigh );

	json_decref ( CalibInLow );
	json_decref ( CalibInHigh );
	json_decref ( CalibOutLow );
	json_decref ( CalibOutHigh );

};
