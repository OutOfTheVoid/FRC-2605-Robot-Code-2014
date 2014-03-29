#ifndef SHS_2605_ARM_CONFIG
#define SHS_2605_ARM_CONFIG

#include "src/Config/ConfigSection.h"

#include "CollectorArms.h"

class ArmConfig : public ConfigSection
{
public:

	ArmConfig ( CollectorArms * Arms );
	~ArmConfig ();

	void LoadArmZeros ();
	void SetArmZeros ();

private:

	CollectorArms * Arms;

};

#endif
