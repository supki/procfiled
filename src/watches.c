#include <fnmatch.h>
#include <errno.h>
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

static int inotify_instance = 0;
static char * config_name = NULL;
static int config_watch_instance = 0;
static config_record_t * config_head = NULL;

static void add_watches( void )
{
	sleep( 1 ); /* avoid too smart editor problem */
	config_t * config = open_config( config_name );
	config_head = read_config( config );
	close_config( config );
	config_watch_instance = inotify_add_watch( inotify_instance, config_name, IN_MODIFY );
	for_each( config_record_t, config_head, config_record )
	{
		config_record->watch_instance = inotify_add_watch( inotify_instance, config_record->source_path, IN_CREATE | IN_MODIFY );
		syslog( LOG_INFO, "+ %d: %s %s from %s to %s", config_record->watch_instance, config_record->name, config_record->pattern, config_record->source_path, config_record->destination_path );
	}
}

static void remove_watches( void )
{
	for_each( config_record_t, config_head, config_record )
	{
		inotify_rm_watch( inotify_instance, config_record->watch_instance );
		syslog( LOG_INFO, "- %d: %s %s from %s to %s", config_record->watch_instance, config_record->name, config_record->pattern, config_record->source_path, config_record->destination_path );
	}
	destroy_config( config_head );
	inotify_rm_watch( inotify_instance, config_watch_instance );
}

void init_watches( char * config_file )
{
	inotify_instance = inotify_init( );
	config_name = config_file;
	add_watches( );
}

int get_inotify_events( char * events )
{
	return read( inotify_instance, (void *)events, EVENT_BUF_LEN );
}

void parse_inotify_event( struct inotify_event * event )
{
	if ( event->wd == config_watch_instance )
	{
		remove_watches( );
		add_watches( );
		return;
	}

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
