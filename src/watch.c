#include <stdlib.h>
#include <sys/inotify.h>

#include "watch.h"

#define for_each(type, head, item) \
    for(type * item = head; item != NULL; item = item->next)

watch_record_t * init_watches( int inotify_instance, config_record_t * config_head )
{
	watch_record_t * watch_head = NULL, * prev_watch_record = NULL, * watch_record;
	for_each( config_record_t, config_head, config_record )
	{
		watch_record = ( watch_record_t * ) malloc( sizeof( watch_record_t ) );
		watch_record->wd = inotify_add_watch( inotify_instance, config_record->source_path, IN_CREATE );
		watch_record->info = config_record;
		if ( prev_watch_record )
		{
			prev_watch_record->next = watch_record;
		}
		else
		{
			prev_watch_record = watch_record;
		}
		if ( !watch_head )
		{
			watch_head = watch_record;
		}
	}

	return watch_head;
}

void print_watches( watch_record_t * watch_head )
{
	for_each( watch_record_t, watch_head, watch_record )
	{
		printf( "%d: %s\n", watch_record->wd, watch_record->info->source_path );
	}
}
