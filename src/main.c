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
#include <sys/types.h>
#include <unistd.h>

#include "daemonize.h"
#include "config.h"
#include "watch.h"

#define EVENT_SIZE    ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN ( 1024 * ( EVENT_SIZE + 16 ) )

#define for_each(type, head, item) \
    for(type * item = head; item != NULL; item = item->next)

static int daemon_mode = 1;

void get_options( int argc, char * argv[] )
{
	for( int i = 1; i < argc; i++ )
	{
		if ( !strcmp( argv[i], "--no-daemon" ) )
		{
			daemon_mode = 0;
		}
	}
}

int main( int argc, char * argv[] )
{
	get_options( argc, argv );
	if ( daemon_mode ) daemonize( );

	config_t * config = open_config( );
	config_record_t * config_head = read_config( config );

	int inotify_instance = inotify_init( );

	watch_record_t * watch_head = init_watches( inotify_instance, config_head );

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
					char * old_path = (char *) malloc( strlen( watch_record->info->source_path ) + strlen( "/" ) + strlen( event->name ) + 1 );
					strcpy( old_path, watch_record->info->source_path );
					strcat( old_path, "/" );
					strcat( old_path, event->name );

					char * new_path = (char *) malloc( strlen( watch_record->info->destination_path ) + strlen( "/" ) + strlen( event->name ) + 1 );
					strcpy( new_path, watch_record->info->destination_path );
					strcat( new_path, "/" );
					strcat( new_path, event->name );

					rename( old_path, new_path );

					free( old_path );
					free( new_path );
				}
			}

			i += EVENT_SIZE + event->len;
		}
	}

	close( inotify_instance );

	return EXIT_SUCCESS;
}
