#include "Logger.h"

Logger * Logger :: Instance = NULL;

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
		Log ( LOG_ERROR, "Couldn't create print synch semaphore!\n" );

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

		if ( Level == LOG_ERROR )
			printf ( "* ERROR: " );

		if ( Level == LOG_WARNING )
			printf ( "* WARNING: " );

		va_list ArgList;
		va_start ( ArgList, Format );
		vprintf ( Format, ArgList );
		va_end ( ArgList );

		if ( Level == LOG_ERROR )
		{

			semGive ( PrintSynchSemaphore );
			throw "(Logged Error)\n";
		
		}

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
