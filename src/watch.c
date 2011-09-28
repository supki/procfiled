#include <stdlib.h>
#include <sys/inotify.h>

#include "watch.h"

#define for_each(type, head, item) \
    for(type * item = head; item != NULL; item = item->next)

watch_record_t * init_watches( char * config_file, int inotify_instance )
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

	return watch_record;
}

void print_watches( watch_record_t * watch_head )
{
	for_each( watch_record_t, watch_head, watch_record )
	{
		printf( "%d: %s\n", watch_record->wd, watch_record->info->source_path );
	}
}
