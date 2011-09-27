/*
 * Move Torrent Daemon
 *
 * Automatically moves files
 * Based on Inotify
 *
 */
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

int main( )
{
	/* daemonize( ); */

	config_t * config = open_config( );
	config_record_t * config_head = read_config( config );
	print_config( config_head );

	int inotify_instance = inotify_init( );

	watch_record_t * watch_head = init_watches( inotify_instance, config_head );
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
				if ( ( watch_record->wd != event->wd ) ||
					!( event->mask & IN_CREATE ) ||
					( event->mask & IN_ISDIR ) ||
					!( event->len ) )
				{
					continue;
				}

				if ( !strcmp( event->name + strlen( event->name ) - strlen( watch_record->info->file_mask ), watch_record->info->file_mask ) )
				{
					char old_path[256], new_path[256];
					strcpy( old_path, watch_record->info->source_path );
					strcat( old_path, event->name );
					strcpy( new_path, watch_record->info->destination_path );
					strcat( new_path, event->name );

					if ( rename( old_path, new_path ) == -1 )
					{
						return EXIT_FAILURE;
					}
				}
			}

			i += EVENT_SIZE + event->len;
		}
	}

	close( inotify_instance );

	return EXIT_SUCCESS;
}
