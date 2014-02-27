#ifndef SHS_2605_CONFIG_SECTION_H
#define SHS_2605_CONFIG_SECTION_H

class ConfigSection
{
public:

	virtual ~ConfigSection ();

	virtual const char * GetSectionName ();
	virtual const char ** GetRealValueKeys ();
	virtual const char ** GetIntValueKeys ();
	virtual const char ** GetStringValueKeys ();
	virtual const char ** GetBoolValueKeys ();

private:


};

#endif
