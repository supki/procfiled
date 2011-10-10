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
#include <wordexp.h>

#include "daemonize.h"
#include "logger.h"

#define MAX_PID_LENGTH 5

static int daemon_mode = 1;
static int default_config = 1;
static char * config_file = NULL;
static int default_pidfile = 1;
static char * pid_file = NULL;

static char * expand_path( char * path )
{
	wordexp_t wordexp_buffer;
	wordexp( path, &wordexp_buffer, 0 );
	char * expanded = strdup( wordexp_buffer.we_wordv[0] );
	wordfree( &wordexp_buffer );

	return expanded;
}

static char * get_pidfile_path( void )
{
	if ( default_pidfile )
	{
		pid_file = ( getuid( ) == 0 ) ?
			strdup( "/var/run/mtd.pid" ):
			expand_path( "~/.mtdpid" );
	}

	return pid_file;
}

static void print_version( void )
{
	printf( "mtd (MTD: Move Torrents Daemon) 0.2.0\n\nCopyright (C) 2011 Matvey Aksenov <matvey.aksenov@gmail.com>\nThis is free software; see the source for copying conditions.  There is NO\nwarranty; not even MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n" );
	exit( EXIT_SUCCESS );
}

static void kill_daemon( void )
{
	char * pid_file = get_pidfile_path( );
	int fd = open( pid_file, O_RDONLY );
	if ( fd < 0 )
	{
		log_error_and_exit( "Cannot open PID file %s", pid_file );
	}
	free( pid_file );
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
		log_error_and_exit( "Cannot kill %d process", pid );
	}

	exit( EXIT_SUCCESS );
}

void get_options( int argc, char * argv[] )
{
	struct option long_options[] = {
	{"version",    no_argument,       0,  1 },
	{"kill",       no_argument,       0,  2 },
	{"conf",       required_argument, 0,  3 },
	{"pidfile",    required_argument, 0,  4 },
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
			config_file = strdup( optarg );
			default_config = 0;
			break;

		case 4:
			pid_file = strdup( optarg );
			default_pidfile = 0;
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

	char * pid_file = get_pidfile_path( );
	int fd = open( pid_file, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR );
	if ( fd < 0 )
	{
		log_error_and_exit( "Cannot open PID file %s", pid_file );
	}
	free( pid_file );
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

	signal(SIGTERM, signal_term_handler);
	signal(SIGHUP, signal_ignore_handler);
	signal(SIGINT, signal_ignore_handler);
	signal(SIGQUIT, signal_ignore_handler);

	if ( daemon( 0, 0 ) < 0 )
	{
		log_error_and_exit( "Cannot daemonize" );
	}

	umask( 0 );

	log_syslog_start( );
	atexit( log_end );

	save_pid( );
}

char * get_configfile_path( void )
{
	if ( default_config )
	{
		config_file = ( getuid( ) == 0 ) ?
			strdup( "/etc/mtd.conf" ):
			expand_path( "~/.mtdconf" );
	}

	return config_file;
}
