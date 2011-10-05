#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>

#include "daemonize.h"
#include "path.h"

#define DAEMON_NAME   "mtd"

#define MAX_PID_LENGTH 5

static int default_config = 1;
static char * config_file = NULL;

static void print_version( void )
{
	printf( "mtd (MTD: Move Torrents Daemon) 0.2.0\n\nCopyright (C) 2011 Matvey Aksenov <matvey.aksenov@gmail.com>\nThis is free software; see the source for copying conditions.  There is NO\nwarranty; not even MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n" );
	exit( EXIT_SUCCESS );
}

static void kill_daemon( void )
{
	char * pid_file = construct_path( expand_path( "~" ), ".mtdpid" );
	int fd = open( pid_file, O_RDONLY );
	if ( fd < 0 ) exit( EXIT_FAILURE );
	char line[ MAX_PID_LENGTH + 1 ];
	read( fd, &line, sizeof( line ) );
	close( fd );

	int pid;
	sscanf( line, "%d", &pid );
	if ( kill( pid, SIGTERM ) < 0 ) exit( EXIT_FAILURE );
	exit( EXIT_SUCCESS );
}

void get_options( int argc, char * argv[] )
{
	for( int i = 1; i < argc; i++ )
	{
		if ( !strcmp( argv[i], "--version" ) )
		{
			print_version( );
		}
		if ( !strcmp( argv[i], "--kill" ) )
		{
			kill_daemon( );
		}
		if ( !strcmp( argv[i], "--conf" ) )
		{
			config_file = expand_path( argv[ ++i ] );
			default_config = 0;
		}
	}
}

static void signal_term_handler( int signal )
{
	exit( EXIT_SUCCESS );
	(void) signal;
}

static void signal_ignore_handler( int signal )
{
	(void) signal;
}

static void save_pid( void )
{
	char pid[ MAX_PID_LENGTH + 1 ];
	int length = snprintf( pid, sizeof( pid ), "%d", getpid( ) );
	pid[length] = '\n';

	char * pid_file = construct_path( expand_path( "~" ), ".mtdpid" );
	int fd = open( pid_file, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR );
	destroy_path( pid_file );
	if ( flock( fd, LOCK_EX | LOCK_NB ) )
	{
		syslog( LOG_INFO, "PID file locking is failed, possibly another mtd instance is runniing. Aborting.." );
		exit( EXIT_FAILURE );
	}
	write( fd, &pid, length + 1 );

}
static void log_init( void )
{
	syslog( LOG_INFO, "Daemon starting.." );
}

static void log_exit( void )
{
	syslog( LOG_INFO, "Daemon exiting.." );
}

void daemonize( void )
{
	signal(SIGHUP, signal_ignore_handler);
	signal(SIGTERM, signal_term_handler);
	signal(SIGINT, signal_ignore_handler);
	signal(SIGQUIT, signal_ignore_handler);

	pid_t pid = fork( );
	if ( pid < 0 ) exit( EXIT_FAILURE );
	if ( pid > 0 ) exit( EXIT_SUCCESS );

	umask( 0 );

	log_init( );
	setlogmask( LOG_UPTO( LOG_INFO ) );
	openlog( DAEMON_NAME, LOG_CONS, LOG_USER );

	if ( setsid( ) < 0 ) exit( EXIT_FAILURE );
	if ( ( chdir( "/" ) ) < 0 ) exit( EXIT_FAILURE );

	close( STDIN_FILENO );
	close( STDOUT_FILENO );
	close( STDERR_FILENO );

	save_pid( );
	atexit( log_exit );
}

char * get_config_name( void )
{
	if ( default_config )
	{
		config_file = construct_path( expand_path( "~" ), ".mtdconf" );
	}

	return config_file;
}
