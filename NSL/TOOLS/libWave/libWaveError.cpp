#pragma warning( disable : 4786 )

#include <stdarg.h>
#include <vector>
#include <string>
#include "libWave.h"

static std::vector<std::string> reportHeaderStack;
static std::vector<int>			reportVerboseLevelStack;
static FILE*					reportFile = stderr;

static struct WAVEReportInitClass
{ 
	WAVEReportInitClass() 
	{ 
		reportHeaderStack.push_back("libWave");
		reportVerboseLevelStack.push_back(0);
	}
} WAVEReportInitInstance;

int WAVE_ReportHeaderPush( const char *fmt, ... )
{
	char      buff[1024];
	va_list   args;
	va_start( args, fmt );
	vsprintf( buff, fmt, args );
	reportHeaderStack.push_back( buff );
	return 0;
}

int WAVE_ReportHeaderPop( void )
{
	reportHeaderStack.pop_back();
	return 0;
}

int WAVE_ReportVerboseLevelPush( int newLevel )
{
	reportVerboseLevelStack.push_back( newLevel );
	return 0;
}

int WAVE_ReportVerboseLevelPop( void )
{
	int level = reportVerboseLevelStack.back();
	reportVerboseLevelStack.pop_back();
	return level;
}

WAVE_CReportHeader::WAVE_CReportHeader( const char *fmt, ... )
{
	char      buff[1024];
	va_list   args;
	va_start( args, fmt );
	vsprintf( buff, fmt, args );
	reportHeaderStack.push_back( buff );
}

WAVE_CReportHeader::~WAVE_CReportHeader( void )
{
	reportHeaderStack.pop_back();
}

WAVE_CReportVerboseLevel::WAVE_CReportVerboseLevel( int level )
{
	WAVE_ReportVerboseLevelPush( level );
}

WAVE_CReportVerboseLevel::~WAVE_CReportVerboseLevel( void )
{
	WAVE_ReportVerboseLevelPop();
}


static void report( int thisLevel, char *str )
{
	int level = reportVerboseLevelStack.back();
	if( level >= thisLevel )
	{
		if( level >= WAVE_REPORT_VERBOSE_LEVEL_SHOW_HEADERS )
		{
			std::vector<std::string>::iterator i;
			int s = reportHeaderStack.size();
			// Print all without the last one - .back();
			for( i = reportHeaderStack.begin(); i != reportHeaderStack.end(); ++i )
				fprintf( reportFile, " %s", i->c_str() );
			fprintf( reportFile, ": %s\n", str );
		}
		else
		{
			const char *hdr;
			hdr = reportHeaderStack.back().c_str();
			fprintf( reportFile, "%s: %s\n", hdr, str );
		}
	}
}

int WAVE_ReportError( const char *fmt, ... )
{
	char      buff[1024];
	va_list   args;
	va_start( args, fmt );
	vsprintf( buff, fmt, args );
	report( WAVE_REPORT_VERBOSE_LEVEL_SHOW_ERRORS, buff );
	return 0;
}

int WAVE_ReportWarning( const char *fmt, ... )
{
	char      buff[1024];
	va_list   args;
	va_start( args, fmt );
	vsprintf( buff, fmt, args );
	report( WAVE_REPORT_VERBOSE_LEVEL_SHOW_WARNINGS, buff );
	return 0;
}

int WAVE_ReportInternal( const char *fmt, ... )
{
	char      buff[1024];
	va_list   args;
	va_start( args, fmt );
	vsprintf( buff, fmt, args );
	report( WAVE_REPORT_VERBOSE_LEVEL_SHOW_INTERNALS, buff );
	return 0;
}

int WAVE_ReportMessage( const char *fmt, ... )
{
	char      buff[1024];
	va_list   args;
	va_start( args, fmt );
	vsprintf( buff, fmt, args );
	report( WAVE_REPORT_VERBOSE_LEVEL_SHOW_MESSAGES, buff );
	return 0;
}

int WAVE_ReportFatal( const char *fmt, ... )
{
	char      buff[1024];
	va_list   args;
	va_start( args, fmt );
	vsprintf( buff, fmt, args );
	report( WAVE_REPORT_VERBOSE_LEVEL_SHOW_FATALS, buff );
	return 0;
}
