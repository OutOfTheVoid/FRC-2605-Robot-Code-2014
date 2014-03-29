#include "ConfigSection.h"

ConfigSection :: ConfigSection ( const char * Name )
{

	this -> Name = Name;
	this -> SectionNode = json_object ();

};

ConfigSection :: ~ConfigSection ()
{

	if ( SectionNode != NULL )
		json_decref ( SectionNode );

};

const char * ConfigSection :: GetName ()
{

	return this -> Name;

};

json_t * ConfigSection :: GetValueObject ()
{

	return SectionNode;

}

void ConfigSection :: SetLoadedValueObject ( json_t * ValueNode )
{

	if ( ValueNode == NULL )
	{

		New = true;
		SectionNode = json_object ();

	}
	else
	{

		New = false;

		json_incref ( ValueNode );
		SectionNode = ValueNode;
	
	}

};

bool ConfigSection :: IsNodeNew ()
{

	return New;

};
