#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "path.h" 
#include "watch.h"

#define DAEMON_NAME   "mtd"
#define EVENT_SIZE    ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN ( 1024 * ( EVENT_SIZE + 16 ) )

#define MAX_PID_LENGTH 5

static int print_version_mode = 0;
static int kill_daemon_mode = 0;
static int daemon_mode = 1;
static int default_config_mode = 1;
static char * config_file = NULL;

void get_options( int argc, char * argv[] )
{
	for( int i = 1; i < argc; i++ )
	{
		if ( !strcmp( argv[i], "--version" ) )
		{
			print_version_mode = 1;
		}
		if ( !strcmp( argv[i], "--kill" ) )
		{
			kill_daemon_mode = 1;
		}
		if ( !strcmp( argv[i], "--no-daemon" ) )
		{
			daemon_mode = 0;
		}
		if ( !strcmp( argv[i], "--conf" ) )
		{
			config_file = expand_path( argv[ ++i ] );
			default_config_mode = 0;
		}
	}
}

void print_version( void )
{
	printf( "mtd (MTD: Move Torrents Daemon) 0.1.0\n\nCopyright (C) 2011 Matvey Aksenov <matvey.aksenov@gmail.com>\nThis is free software; see the source for copying conditions.  There is NO\nwarranty; not even MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n" );
	exit( EXIT_SUCCESS );
}

void kill_daemon( void )
{
	char * pid_file = construct_path( expand_path( "~" ), ".mtdpid" );
	int fd = open( pid_file, O_RDONLY );
	char line[ MAX_PID_LENGTH + 1 ];
	read( fd, &line, sizeof( line ) );
	close( fd );

	int pid;
	sscanf( line, "%d", &pid );
	if ( kill( pid, SIGKILL ) < 0 ) exit( EXIT_FAILURE );
	if ( unlink( pid_file ) < 0 ) exit( EXIT_FAILURE );
	destroy_path( pid_file );
	exit( EXIT_SUCCESS );
}

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
	if ( pid < 0 ) exit( EXIT_FAILURE );
	if ( pid > 0 ) exit( EXIT_SUCCESS );

	umask( 0 );

	if ( setsid( ) < 0 ) exit( EXIT_FAILURE );
	if ( ( chdir( "/" ) ) < 0 ) exit( EXIT_FAILURE );

	close( STDIN_FILENO );
	close( STDOUT_FILENO );
	close( STDERR_FILENO );
}

void set_default_config_name( void )
{
	config_file = construct_path( expand_path( "~" ), ".mtdconf" );
}

void save_pid( void )
{
	char pid[ MAX_PID_LENGTH + 1 ];
	int length = snprintf( pid, sizeof( pid ), "%d", getpid( ) );
	pid[length] = '\n';

	char * pid_file = construct_path( expand_path( "~" ), ".mtdpid" );
	int fd = open( pid_file, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR );
	destroy_path( pid_file );
	write( fd, &pid, length + 1 );
	close( fd );
}

int main( int argc, char * argv[] )
{
	get_options( argc, argv );
	if ( print_version_mode ) print_version( );
	if ( kill_daemon_mode ) kill_daemon( );
	if ( daemon_mode ) daemonize( );
	if ( default_config_mode ) set_default_config_name( );
	save_pid( );

	int inotify_instance = inotify_init( );
	init_watches( config_file, inotify_instance );

	char buffer[ EVENT_BUF_LEN ];
	while ( 1 )
	{
		int length = read( inotify_instance, (void *)buffer, EVENT_BUF_LEN );
		if ( length < 0 )
		{
			if ( errno == EINTR ) continue;
			return EXIT_FAILURE;
		}

		for ( int i = 0; i < length; i += EVENT_SIZE + ( (struct inotify_event *) &buffer[ i ] )->len )
		{
			struct inotify_event * event = (struct inotify_event *) &buffer[ i ];
			parse_inotify_event( event );
		}
	}

	close( inotify_instance );

	return EXIT_SUCCESS;
}
