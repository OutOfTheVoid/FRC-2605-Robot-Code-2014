#include "Logger.h"

Logger * Logger :: GetInstance ()
{

	if ( Instance == NULL )
		Instance = new Logger ();

	return Instance;

};

Logger :: Logger ()
{

	PrintLevel = LOG_DEBUG;

	PrintSynchSemaphore = semMCreate ( SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE );

	if ( PrintSynchSemaphore == NULL )
		throw "Logger Error: Couldn't create print synch semaphore.";

};

Logger :: ~Logger ()
{



};

void Logger :: SetPrintLevel ( LogLevel Level )
{

	PrintLevel = Level;

};

void Logger :: Log ( LogLevel Level, const char * Format, ... )
{

	semTake ( PrintSynchSemaphore, WAIT_FOREVER );

	if ( Level <= PrintLevel )
	{

		va_list ArgList;
		va_start ( ArgList, Format );
		vprintf ( Format, ArgList );
		va_end ( ArgList );

	}

	semGive ( PrintSynchSemaphore );

};

void Logger :: ForcedLog ( const char * Format, ... )
{

	semTake ( PrintSynchSemaphore, WAIT_FOREVER );

	va_list ArgList;
	va_start ( ArgList, Format );
	vprintf ( Format, ArgList );
	va_end ( ArgList );

	semGive ( PrintSynchSemaphore );

};
