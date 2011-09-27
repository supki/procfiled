/*
 * Move Torrent Daemon
 *
 * Automatically moves files
 * Based on Inotify
 *
 */
#include <glob.h>
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
				if ( ( watch_record->wd != event->wd ) || !( event->len ) )
				{
					continue;
				}

				glob_t glob_record;
				char * pattern = (char *) malloc( strlen( watch_record->info->source_path ) + strlen( "/" ) + strlen( watch_record->info->pattern ) + 1 );
				strcpy( pattern, watch_record->info->source_path );
				strcat( pattern, "/" );
				strcat( pattern, watch_record->info->pattern );
				if ( glob( pattern, GLOB_MARK | GLOB_NOSORT, NULL, &glob_record ) < 0 )
				{
					return EXIT_FAILURE;
				}

				for ( unsigned int j = 0; j < glob_record.gl_pathc; j++ )
				{
					char new_path[256];
					strcpy( new_path, watch_record->info->destination_path );
					strcat( new_path, "/" );
					strcat( new_path, event->name );
					printf( "%s %s\n", glob_record.gl_pathv[j], new_path );

					if ( rename( glob_record.gl_pathv[j], new_path ) == -1 )
					{
						return EXIT_FAILURE;
					}
				}

				globfree( &glob_record );
			}

			i += EVENT_SIZE + event->len;
		}
	}

	close( inotify_instance );

	return EXIT_SUCCESS;
}
