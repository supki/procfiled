#include <stdlib.h>
#include <sys/inotify.h>

#include "watch.h"

watch_record_t * init_watches( int inotify_instance, config_record_t * config_head )
{
	watch_record_t * watch_head = NULL, * prev_watch_record = NULL, * watch_record;
	config_record_t * prev_config_record, * config_record = config_head;
	do
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

		prev_config_record = config_record;
		config_record = config_record->next;
	}
	while ( prev_config_record->next );

	return watch_head;
}

void print_watches( watch_record_t * watch_head )
{
	watch_record_t * watch_record, * prev_watch_record = NULL;
	watch_record = watch_head;
	do
	{
		printf( "%d: %s\n", watch_record->wd, watch_record->info->source_path );

		prev_watch_record = watch_record;
		watch_record = watch_record->next;
	}
	while ( prev_watch_record->next );
}
