#include <err.h>
#include <fnmatch.h>
#include <errno.h>
#include <libnotify/notify.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>

#include "config.h"
#include "logger.h"
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
	config_t * config = fopen( config_name, "r" );
	if ( config == NULL )
	{
		log_error_and_exit( "Cannot open config file %s", config_name );
	}
	config_head = read_config( config );
	fclose( config );
	config_watch_instance = inotify_add_watch( inotify_instance, config_name, IN_MODIFY );
	for_each( config_record_t, config_head, config_record )
	{
		config_record->watch_instance = inotify_add_watch( inotify_instance, config_record->src, IN_CREATE | IN_MODIFY | IN_MOVED_TO );
		log_info( "+ %d: %s %s from %s to %s", config_record->watch_instance, config_record->name, config_record->pattern, config_record->src, config_record->dst );
	}
}

static void remove_watches( void )
{
	for_each( config_record_t, config_head, config_record )
	{
		inotify_rm_watch( inotify_instance, config_record->watch_instance );
		log_info( "- %d: %s %s from %s to %s", config_record->watch_instance, config_record->name, config_record->pattern, config_record->src, config_record->dst );
	}
	destroy_config( config_head );
	inotify_rm_watch( inotify_instance, config_watch_instance );
}

void init_watches( char * config_file )
{
	notify_init( "procfiled" );
	inotify_instance = inotify_init( );
	config_name = config_file;
	add_watches( );
}

int get_inotify_events( char * events )
{
	return read( inotify_instance, (void *)events, EVENT_BUF_LEN );
}

static char * construct_path( const char * dir_name, const char * file_name )
{
	char * path = malloc( strlen( dir_name ) + strlen( file_name ) + 2 ); 
	strcpy( path, dir_name);
	strcat( path, "/" );
	strcat( path, file_name );

	return path;
}

#define MAX_NOTIFICATION_BODY_LENGTH 80
static void notify_send ( NotifyUrgency urgency, const char * title, const char * fmt, ... )
{
	va_list args;
	va_start( args, fmt );

	char body[MAX_NOTIFICATION_BODY_LENGTH];
	vsnprintf( body, MAX_NOTIFICATION_BODY_LENGTH-1, fmt, args );

	NotifyNotification *example;
	example = notify_notification_new( title, body, NULL );
	notify_notification_set_urgency( example, urgency );
	notify_notification_show( example, NULL );
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
			char * old_name = construct_path( config_record->src, event->name );
			char * new_name = construct_path( config_record->dst, event->name );

			if ( config_record->function( old_name, new_name ) == -1 )
			{
				log_warning( "daemon failed to %s: %s", config_record->name, strerror( errno ) );
				notify_send( NOTIFY_URGENCY_CRITICAL, "daemon failed to %s: %s", config_record->name, strerror( errno ) );
			}
			log_notice( "%s %s to %s", config_record->name, old_name, new_name );
			notify_send( NOTIFY_URGENCY_NORMAL, config_record->name, "%s\n→\n%s", old_name, new_name );

			free( old_name );
			free( new_name );
		}
	}
}
