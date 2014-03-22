#ifndef SHS_2605_LOGGER_H
#define SHS_2605_LOGGER_H

#include <stdio.h>
#include <stdarg.h>
#include <semlib.h>

class Logger
{
public:

	typedef enum
	{

		LOG_FATAL_ERROR = 0,
		LOG_ERROR = 1,
		LOG_PROBLEM = 2,
		LOG_WARNING = 3,
		LOG_EVENT = 4,
		LOG_DEBUG = 5,
		LOG_DEBUG2 = 6,
		LOG_DEBUG3 = 7,

	} LogLevel;

	static Logger * GetInstance ();

	void SetPrintLevel ( LogLevel Level );
	
	void Log ( LogLevel Level, const char * Format, ... );
	void ForcedLog ( const char * Format, ... );

private:

	Logger ();
	~Logger ();

	static Logger * Instance;

	LogLevel PrintLevel;

	SEM_ID PrintSynchSemaphore;

};

void SetLogLevel ( int l );

#endif
