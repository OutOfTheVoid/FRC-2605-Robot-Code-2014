#include "ArmConfig.h"

ArmConfig :: ArmConfig ( CollectorArms * Arms ):
	ConfigSection ( "ShooterArms" )
{

	this -> Arms = Arms;

};

ArmConfig :: ~ArmConfig ()
{
};

void ArmConfig :: SetArmZeros ()
{

	json_t * ZeroLeft = json_real ( Arms -> ZeroL );
	json_t * ZeroRight = json_real ( Arms -> ZeroR );

	json_object_set ( SectionNode, "Zero_Left", ZeroLeft );
	json_object_set ( SectionNode, "Zero_Right", ZeroRight );

};

void ArmConfig :: LoadArmZeros ()
{

	json_t * ZeroLeft = json_object_get ( SectionNode, "Left_Zero" );
	json_t * ZeroRight = json_object_get ( SectionNode, "Right_Zero" );

	Arms -> ZeroL = json_real_value ( ZeroLeft );
	Arms -> ZeroR = json_real_value ( ZeroRight );

};
