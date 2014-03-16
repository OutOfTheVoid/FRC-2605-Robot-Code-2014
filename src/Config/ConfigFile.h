#ifndef SHSConfigFile
#define SHSConfigFile

#include "Jansson/jansson.h"

#include "ConfigSection.h"

#include "src/Logging/Logger.h"

#include <unistd.h>

class ConfigFile
{
public:

	ConfigFile ( const char * FilePath = "/FRC-Config.json" );
	~ConfigFile ();

	bool Init ();

	void AddConfigSection ( ConfigSection * Section );
	void RemoveConfigSection ( ConfigSection * Section );

	bool Write ();

private:

	const char * FilePath;

	json_t * RootNode;

	bool Loaded;

	Logger * Log;

};

#endif
