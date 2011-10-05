#include <fnmatch.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <syslog.h>
#include <unistd.h>

#include "path.h"
#include "config.h"
#include "watches.h"

#define for_each(type, head, item) \
	for(type * item = head; item != NULL; item = item->next)

#define MOVE_COMMAND "move"
#define LINK_COMMAND "link"

static int inotify_instance = 0;
static config_record_t * config_head = NULL;

void init_watches( char * config_file, int inotify_initialized )
{
	inotify_instance = inotify_initialized;
	config_t * config = open_config( config_file );
	config_head = read_config( config );

	syslog( LOG_INFO, "Config:" );
	for_each( config_record_t, config_head, config_record )
	{
		config_record->watch_instance = inotify_add_watch( inotify_instance, config_record->source_path, IN_CREATE | IN_MODIFY );
		syslog( LOG_INFO, "%d: %s %s from %s to %s", config_record->watch_instance, config_record->name, config_record->pattern, config_record->source_path, config_record->destination_path );
	}
}

int get_inotify_events( char * events )
{
	return read( inotify_instance, (void *)events, EVENT_BUF_LEN );
}

void parse_inotify_event( struct inotify_event * event )
{
	for_each( config_record_t, config_head, config_record )
	{
		if ( ( config_record->watch_instance != event->wd ) || !( event->len ) )
		{
			continue;
		}

		if ( !fnmatch( config_record->pattern, event->name, 0 ) )
		{
			char * old_name = construct_path( config_record->source_path, event->name );
			char * new_name = construct_path( config_record->destination_path, event->name );

			config_record->function( old_name, new_name );
			syslog( LOG_NOTICE, "%s %s to %s", config_record->name, old_name, new_name );

			destroy_path( old_name );
			destroy_path( new_name );
		}
	}
}
