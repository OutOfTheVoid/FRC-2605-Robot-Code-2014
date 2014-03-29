#ifndef SHS_2605_CONFIG_FILE
#define SHS_2605_CONFIG_FILE

#include "Jansson/jansson.h"

#include "ConfigSection.h"

#include "src/Logging/Logger.h"

#include <unistd.h>

class ConfigFile
{
public:

	ConfigFile ( const char * FilePath = "/RobotConfig/FRC2605-Config.json" );
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
