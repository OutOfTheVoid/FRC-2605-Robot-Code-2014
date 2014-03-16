#include "ConfigFile.h"

ConfigFile :: ConfigFile ( const char * FilePath )
{

	Log = Logger :: GetInstance ();

	this -> FilePath = FilePath;
	this -> Loaded = false;

};

bool ConfigFile :: Init ()
{

	if ( Loaded )
		return true;

	json_error_t JSONError;

	RootNode = json_load_file ( FilePath, JSON_REJECT_DUPLICATES, & JSONError );

	if ( RootNode == NULL )
	{

		Log -> Log ( Logger :: LOG_WARNING, "CONFIG_FILE: JSON Error: %s\n", JSONError.text );
		return false;

	}

	Loaded = true;

	return true;

};

void ConfigFile :: AddConfigSection ( ConfigSection * Section )
{

	json_t * SectionNode = json_object_get ( RootNode, Section -> GetName () );

	if ( SectionNode != NULL )
	{

		Section -> SetLoadedValueObject ( SectionNode );
		return;

	}

	json_object_set ( RootNode, Section -> GetName (), Section -> GetValueObject () );

};

// TODO
void ConfigFile :: RemoveConfigSection ( ConfigSection * Section )
{

	json_object_del ( RootNode, Section -> GetName () );

};

// TODO
bool ConfigFile :: Write ()
{

	if ( json_dump_file ( RootNode, FilePath, JSON_INDENT(4) | JSON_SORT_KEYS ) == - 1 )
	{

		Log -> Log ( Logger :: LOG_WARNING, "JSON Write failed!\n" );
		return false;
	
	}

	return true;

};

ConfigFile :: ~ConfigFile ()
{

	json_decref ( RootNode );

};
