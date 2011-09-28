/*
 * Move Torrent Daemon
 *
 * Automatically moves files
 * Based on Inotify
 *
 */
#include <fnmatch.h>
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

#define for_each(type, head, item) \
    for(type * item = head; item != NULL; item = item->next)

static int daemon_mode = 1;
static int default_config_mode = 1;
static char * config_file = NULL;

void get_options( int argc, char * argv[] )
{
	for( int i = 1; i < argc; i++ )
	{
		if ( !strcmp( argv[i], "--no-daemon" ) )
		{
			daemon_mode = 0;
		}
		if ( !strcmp( argv[i], "--conf" ) )
		{
			config_file = shell_expand_path( argv[ ++i ] );
			default_config_mode = 0;
		}
	}
}

void daemonize( void )
{
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

void set_default_config_name( void )
{
	config_file = construct_path( shell_expand_path( "~" ), ".mtdconf" );
}

int main( int argc, char * argv[] )
{
	get_options( argc, argv );
	if ( daemon_mode ) daemonize( );
	if ( default_config_mode ) set_default_config_name( );

	int inotify_instance = inotify_init( );

	watch_record_t * watch_head = init_watches( config_file, inotify_instance );
	print_watches( watch_head );

	char buffer[ EVENT_BUF_LEN ];
	while ( 1 )
	{
		int length = read( inotify_instance, (void *)buffer, EVENT_BUF_LEN );
		if ( length < 0 )
		{
			return EXIT_FAILURE;
		}

		int i = 0;
		while ( i < length )
		{
			struct inotify_event * event = (struct inotify_event *) &buffer[ i ];

			for_each( watch_record_t, watch_head, watch_record )
			{
				if ( ( watch_record->wd != event->wd ) || !( event->len ) )
				{
					continue;
				}

				if ( !fnmatch( watch_record->info->pattern, event->name, 0 ) )
				{
					char * old_path = construct_path( watch_record->info->source_path, event->name );
					char * new_path = construct_path( watch_record->info->destination_path, event->name );

					printf( "%s -> %s\n", old_path, new_path );
					rename( old_path, new_path );

					destroy_path( old_path );
					destroy_path( new_path );
				}
			}

			i += EVENT_SIZE + event->len;
		}
	}

	close( inotify_instance );

	return EXIT_SUCCESS;
}
