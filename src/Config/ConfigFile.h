#ifndef SHSConfigFile
#define SHSConfigFile

#include "Jansson/jansson.h"

#include "ConfigSection.h"

class ConfigFile
{
public:

	ConfigFile ( const char * FilePath = "/FRC-Config.json" );
	~ConfigFile ();

	bool Init ();

private:

	const char * FilePath;

	json_t * RootNode;

	bool Loaded;

};

#endif
