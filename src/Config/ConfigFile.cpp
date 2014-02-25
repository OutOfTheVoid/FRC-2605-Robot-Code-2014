#include "ConfigFile.h"

ConfigFile :: ConfigFile ( const char * FilePath )
{

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

		printf("CONFIG_FILE: JSON Error: %s\n", JSONError.text );
		return false;

	}

	Loaded = true;

	return true;

};

ConfigFile :: ~ConfigFile ()
{



};
