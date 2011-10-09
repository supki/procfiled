#include <fcntl.h>
#include <getopt.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>

#include "daemonize.h"
#include "logger.h"
#include "path.h"

#define MAX_PID_LENGTH 5

static int daemon_mode = 1;
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
		log_error_and_exit( "Cannot open PID file" );
	}
	char line[ MAX_PID_LENGTH + 1 ];
	if ( read( fd, &line, sizeof( line ) ) == -1 )
	{
		log_error_and_exit( "Cannot read from PID file" );
	}
	close( fd );

	int pid;
	sscanf( line, "%d", &pid );
	if ( kill( pid, SIGTERM ) < 0 )
	{
		log_error_and_exit( "Cannot kill PID process" );
	}

	exit( EXIT_SUCCESS );
}

void get_options( int argc, char * argv[] )
{
	struct option long_options[] = {
	{"version",    no_argument,       0,  1 },
	{"kill",       no_argument,       0,  2 },
	{"conf",       required_argument, 0,  3 },
	{"foreground", no_argument,       0, 'f'},
	{0,            0,                 0,  0 }
	};
	while ( 1 )
	{
		int c = getopt_long( argc, argv, "0123:f", long_options, NULL );
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

		case 'f':
			daemon_mode = 0;
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
		log_error_and_exit( "Cannot open PID file" );
	}
	destroy_path( pid_file );
	if ( flock( fd, LOCK_EX | LOCK_NB ) )
	{
		log_error_and_exit( "PID file locking is failed (another mtd instance is running?)." );
	}
	if ( write( fd, &pid, length + 1 ) == -1 )
	{
		log_error_and_exit( "Cannot write to PID file" );
	}
}


void daemonize( void )
{
	if ( !daemon_mode )
	{
		log_warning( "foreground mode is enabled" );
		return;
	}

	signal(SIGHUP, signal_ignore_handler);
	signal(SIGTERM, signal_term_handler);
	signal(SIGINT, signal_ignore_handler);
	signal(SIGQUIT, signal_ignore_handler);

	pid_t pid = fork( );
	if ( pid < 0 )
	{
		log_error_and_exit( "Cannot daemonize" );
	}
	if ( pid > 0 ) exit( EXIT_SUCCESS );

	umask( 0 );

	log_syslog_start( );
	atexit( log_end );

	if ( setsid( ) < 0 )
	{
		log_error_and_exit( "Cannot set daemon session leader!" );
	}
	if ( ( chdir( "/" ) ) < 0 )
	{
		log_error_and_exit( "Cannot change working directory to /" );
	}

	close( STDIN_FILENO );
	close( STDOUT_FILENO );
	close( STDERR_FILENO );

	save_pid( );
}

char * get_config_name( void )
{
	if ( default_config )
	{
		config_file = construct_path( expand_path( "~" ), ".mtdconf" );
	}

	return config_file;
}
