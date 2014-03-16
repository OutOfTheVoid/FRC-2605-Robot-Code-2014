#include "IRDistanceConfig.h"

IRDistanceConfig :: IRDistanceConfig ( IRDistanceSensor * Sensor )
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

};

void IRDistanceConfig :: SetSensorCalibration ()
{

	json_real_t * CalibInLow = json_real ( Sensor -> InAttenuator -> GetRangeIn ().Low );
	json_real_t * CalibInHigh = json_real ( Sensor -> InAttenuator -> GetRangeIn ().High );

	json_real_t * CalibOutLow = json_real ( Sensor -> InAttenuator -> GetRangeOut ().Low );
	json_real_t * CalibOutHigh = json_real ( Sensor -> OutAttenuator -> GetRangeOut ().High );

	json_ojbect_set ( SectionNode, "Calib_In_Low", CalibInLow );
	json_ojbect_set ( SectionNode, "Calib_In_High", CalibInLow );
	json_ojbect_set ( SectionNode, "Calib_Out_Low", CalibInLow );
	json_ojbect_set ( SectionNode, "Calib_Out_High", CalibInLow );

};
