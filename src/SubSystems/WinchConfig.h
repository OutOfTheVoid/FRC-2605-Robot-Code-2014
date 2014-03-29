#ifndef SHS_2605_WINCH_CONFIG
#define SHS_2605_WINCH_CONFIG

#include "src/Config/ConfigSection.h"

#include "ShooterWinch.h"

class WinchConfig : public ConfigSection
{
public:

	WinchConfig ( ShooterWinch * Winch );
	~WinchConfig ();

	void LoadZero ();
	void SetZero ();

private:

	ShooterWinch * Winch;

};

#endif
