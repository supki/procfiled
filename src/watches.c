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
		if ( !strcmp( config_record->command, MOVE_COMMAND ) ||
			!strcmp( config_record->command, LINK_COMMAND ) )
		{
			config_record->watch_instance = inotify_add_watch( inotify_instance, config_record->source_path, IN_CREATE );
		}

		syslog( LOG_INFO, "%d: %s %s from %s to %s", config_record->watch_instance, config_record->command, config_record->pattern, config_record->source_path, config_record->destination_path );
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
			if ( !strcmp( config_record->command, MOVE_COMMAND ) )
			{
				char * old_path = construct_path( config_record->source_path, event->name );
				char * new_path = construct_path( config_record->destination_path, event->name );

				rename( old_path, new_path );
				syslog( LOG_NOTICE, "moving %s to %s", old_path, new_path );

				destroy_path( old_path );
				destroy_path( new_path );
			}
			if ( !strcmp( config_record->command, LINK_COMMAND ) )
			{
				char * file_path = construct_path( config_record->source_path, event->name );
				char * link_path = construct_path( config_record->destination_path, event->name );

				symlink( file_path, link_path );
				syslog( LOG_NOTICE, "linking %s to %s", link_path, file_path );

				destroy_path( file_path );
				destroy_path( link_path );
			}
		}
	}
}
