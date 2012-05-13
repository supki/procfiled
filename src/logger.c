#include <err.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

#include "logger.h"

#define DAEMON_NAME   "procfiled"

static int syslog_enabled = 0;

static void print_console( const char * format, va_list arguments )
{
	vprintf( format, arguments );
	printf( "\n" );
}

void log_syslog_start( void )
{
	setlogmask( LOG_UPTO( LOG_INFO ) );
	openlog( DAEMON_NAME, LOG_CONS, LOG_USER );
	syslog( LOG_INFO, "starting syslog" );
	syslog_enabled = 1;
}

void log_end( void )
{
	log_info( "aborting" );
}

void log_info( const char * format, ... )
{
	va_list arguments;
	va_start( arguments, format );
	( syslog_enabled ) ?
		vsyslog( LOG_INFO, format, arguments ):
		print_console( format, arguments );
}

void log_notice( const char * format, ... )
{
	va_list arguments;
	va_start( arguments, format );
	( syslog_enabled ) ?
		vsyslog( LOG_NOTICE, format, arguments ):
		print_console( format, arguments );
}

void log_warning( const char * format, ... )
{
	va_list arguments;
	va_start( arguments, format );
	( syslog_enabled ) ?
		vsyslog( LOG_WARNING, format, arguments ):
		vwarn( format, arguments );
}

void log_error_and_exit( const char * format, ... )
{
	va_list arguments;
	va_start( arguments, format );
	( syslog_enabled ) ?
		vsyslog( LOG_ERR, format, arguments ):
		vwarn( format, arguments );
	exit( EXIT_FAILURE );
}
