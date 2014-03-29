#include "WinchConfig.h"

WinchConfig :: WinchConfig ( ShooterWinch * Winch ):
	ConfigSection ( "Winch" )
{

	this -> Winch = Winch;

};

WinchConfig :: ~WinchConfig ()
{
};

void WinchConfig :: SetZero ()
{

	json_t * Zero = json_real ( Winch -> Zero );

	json_object_set ( SectionNode, "Zero", Zero );

};

void WinchConfig :: LoadZero ()
{

	json_t * Zero = json_object_get ( SectionNode, "Zero" );

	Winch -> Zero = json_real_value ( Zero );

};
