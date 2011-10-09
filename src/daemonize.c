#include <err.h>
#include <fcntl.h>
#include <getopt.h>
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
	if ( fd < 0 )
	{
		err( EXIT_FAILURE, "Cannot open PID file" );
	}
	char line[ MAX_PID_LENGTH + 1 ];
	if ( read( fd, &line, sizeof( line ) ) == -1 )
	{
		err( EXIT_FAILURE, "Cannot read from PID file" );
	}
	close( fd );

	int pid;
	sscanf( line, "%d", &pid );
	if ( kill( pid, SIGTERM ) < 0 )
	{
		err( EXIT_FAILURE, "Cannot kill PID process" );
	}
	exit( EXIT_SUCCESS );
}

void get_options( int argc, char * argv[] )
{
	struct option long_options[] = {
	{"version",    no_argument,       0,  1 },
	{"kill",       no_argument,       0,  2 },
	{"conf",       required_argument, 0,  3 },
	{0,            0,                 0,  0 }
	};
	while ( 1 )
	{
		int c = getopt_long( argc, argv, "0123:", long_options, NULL );
		if ( c == -1 )
		{
			break;
		}

		switch ( c )
		{
		case 1:
			print_version( );
			break;

		case 2:
			kill_daemon( );
			break;

		case 3:
			config_file = expand_path( optarg );
			default_config = 0;
			break;

		case '?':
			break;
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
	if ( fd < 0 )
	{
		syslog( LOG_ERR, "ERROR: Cannot open PID file!" );
		exit( EXIT_FAILURE );
	}
	destroy_path( pid_file );
	if ( flock( fd, LOCK_EX | LOCK_NB ) )
	{
		syslog( LOG_ERR, "PID file locking is failed (another mtd instance is running?)." );
		exit( EXIT_FAILURE );
	}
	if ( write( fd, &pid, length + 1 ) == -1 )
	{
		syslog( LOG_ERR, "ERROR: Cannot write to PID file!" );
		exit( EXIT_FAILURE );
	}

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
	if ( pid < 0 )
	{
		err( EXIT_FAILURE, "Cannot daemonize" );
	}
	if ( pid > 0 ) exit( EXIT_SUCCESS );

	umask( 0 );

	log_init( );
	setlogmask( LOG_UPTO( LOG_INFO ) );
	openlog( DAEMON_NAME, LOG_CONS, LOG_USER );

	if ( setsid( ) < 0 )
	{
		syslog( LOG_ERR, "Cannot set daemon session leader!" );
		exit( EXIT_FAILURE );
	}
	if ( ( chdir( "/" ) ) < 0 )
	{
		syslog( LOG_ERR, "Cannot change working directory to /" );
		exit( EXIT_FAILURE );
	}

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
