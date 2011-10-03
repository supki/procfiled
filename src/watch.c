#include <fnmatch.h>
#include <stdlib.h>
#include <sys/inotify.h>

#include "path.h"
#include "config.h"
#include "watch.h"

#define for_each(type, head, item) \
    for(type * item = head; item != NULL; item = item->next)

typedef struct watch_record_s
{
	int wd;
	config_record_t * info;
	struct watch_record_s * next;
} watch_record_t;

static watch_record_t * watch_head = NULL;

void init_watches( char * config_file, int inotify_instance )
{
	config_t * config = open_config( config_file );
	config_record_t * config_head = read_config( config );

	watch_record_t * prev_watch_record = NULL, * watch_record = NULL;
	for_each( config_record_t, config_head, config_record )
	{
		watch_record = ( watch_record_t * ) malloc( sizeof( watch_record_t ) );
		watch_record->wd = inotify_add_watch( inotify_instance, config_record->source_path, IN_CREATE | IN_ACCESS | IN_ATTRIB | IN_OPEN );
		watch_record->info = config_record;
		watch_record->next = prev_watch_record;

		prev_watch_record = watch_record;
	}

	watch_head = watch_record;
}

void print_watches( void )
{
	for_each( watch_record_t, watch_head, watch_record )
	{
		printf( "%d: %s\n", watch_record->wd, watch_record->info->source_path );
	}
}

void parse_inotify_event( struct inotify_event * event )
{
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
}
