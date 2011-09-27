#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "daemonize.h"

#define DAEMON_NAME   "mtd"

void signal_handler( int signal )
{
	(void) signal;
}

void daemonize( void )
{
	signal(SIGHUP, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGINT, signal_handler);
	signal(SIGQUIT, signal_handler);

	pid_t pid = fork( );
	if ( pid < 0 )
	{
		exit( EXIT_FAILURE );
	}
	if ( pid > 0 )
	{
		exit( EXIT_SUCCESS );
	}

	umask( 0 );

	pid_t sid = setsid( );
	if ( sid < 0 )
	{
		exit( EXIT_FAILURE );
	}

	if ( ( chdir( "/" ) ) < 0 )
	{
		exit( EXIT_FAILURE );
	}

	close( STDIN_FILENO );
	close( STDOUT_FILENO );
	close( STDERR_FILENO );
}
