#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <wordexp.h>

#include "config.h"
#include "path.h"

#define for_each(type, head, item) \
    for(type * item = head; item != NULL; item = item->next)

#define SYMBOL_SPACE ' '
#define SYMBOL_TAB   '	'

static unsigned int current_position = 0;

config_t * open_config( const char * path )
{
	return fopen( path, "r" );
}

int eat_whitespaces( char * line, unsigned int position )
{
	while( ( line[ position ] == SYMBOL_SPACE ||
		line[ position ] == SYMBOL_TAB ) &&
		position < strlen( line ) )
	{
		position++;
	}

	return position;
}

int eat_not_whitespaces( char * line, unsigned int position )
{
	while( line[ position ] != SYMBOL_SPACE &&
		line[ position ] != SYMBOL_TAB &&
		position < strlen( line ) )
	{
		position++;
	}

	return position;
}

static void read_config_attribute_refresh( void )
{
	current_position = 0;
}

static char * construct_mode_config_argument( char * line, unsigned int length )
{
	char * attribute = malloc( length + 1 );
	strncpy( attribute, line, length );
	attribute[ length ] = '\0';
	return attribute;
}

static char * construct_path_config_argument( char * line, unsigned int length )
{
	char attribute[ length + 1 ];
	strncpy( attribute, line, length );
	attribute[ length ] = '\0';

	return expand_path( attribute );
}

static char * read_config_attribute( char * line, char * (*get_attribute)(char *, unsigned int ) )
{
	unsigned int start_position = eat_whitespaces( line, current_position );
	if ( start_position == strlen( line ) ) return NULL;
	current_position = eat_not_whitespaces( line, start_position );

	return get_attribute( line + start_position, current_position - start_position );
}

static config_record_t * read_config_record( config_t * config )
{
	char line[BUFSIZ];
	if ( line != fgets( line, BUFSIZ, config ) ) return NULL;
	line[ strlen( line ) - 1 ] = '\0';

	config_record_t * record = (config_record_t *) malloc( sizeof( config_record_t ) );
	read_config_attribute_refresh( );
	record->command = read_config_attribute( line, construct_mode_config_argument );
	record->pattern = read_config_attribute( line, construct_mode_config_argument );
	record->source_path = read_config_attribute( line, construct_path_config_argument );
	record->destination_path = read_config_attribute( line, construct_path_config_argument );

	return record;
}

static int is_not_valid_config_record( config_record_t * record )
{
	return ( !record->command || !record->pattern || !record->source_path || !record->destination_path );
}

static void free_config_record( config_record_t * config_record )
{
	if ( config_record->command ) free( (void *)config_record->command );
	if ( config_record->pattern ) free( (void *)config_record->pattern );
	if ( config_record->source_path ) free( (void *)config_record->source_path );
	if ( config_record->destination_path ) free( (void *)config_record->destination_path );
	free( config_record );
}

config_record_t * read_config( config_t * config )
{
	if ( !config ) return NULL;

	config_record_t * prev_config_record = NULL, * config_record;
	while ( ( config_record = read_config_record( config ) ) != NULL )
	{
		if ( is_not_valid_config_record( config_record ) )
		{
			free_config_record( config_record );
			continue;
		}
		config_record->next = prev_config_record;
		prev_config_record = config_record;
	}

	return prev_config_record;
}

void log_config_record( config_record_t * record )
{
	syslog( LOG_INFO, "%s %s from %s to %s",
		record->command, record->pattern, record->source_path, record->destination_path );
}
