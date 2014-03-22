#ifndef SHS_2605_CONFIG_SECTION_H
#define SHS_2605_CONFIG_SECTION_H

#include "Jansson/jansson.h"

class ConfigSection
{
public:

	friend class ConfigFile;

	ConfigSection ( const char * Name );
	virtual ~ConfigSection ();
	
protected:

	virtual const char * GetName ();

	virtual json_t * GetValueObject ();
	virtual void SetLoadedValueObject ( json_t * ValueNode );

	virtual bool IsNodeNew ();

	json_t * SectionNode;
	const char * Name;
	bool New;

};

#endif
