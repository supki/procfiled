#include <stdlib.h>
#include <string.h>
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

static char * extract_attribute( char * line, unsigned int length, int need_expansion )
{
	if ( need_expansion )
	{
		return expand_path_length( line, length );
	}
	else
	{
		char * attribute = malloc( length + 1 );
		strncpy( attribute, line, length );
		attribute[ length ] = '\0';
		return attribute;
	}
}

static char * read_config_attribute( char * line, int need_expansion )
{
	unsigned int start_position = eat_whitespaces( line, current_position );
	current_position = eat_not_whitespaces( line, start_position );

	return extract_attribute( line + start_position, current_position - start_position, need_expansion );
}

static config_record_t * read_config_record( config_t * config )
{
	char line[BUFSIZ];
	if ( line != fgets( line, BUFSIZ, config ) ) return NULL;
	line[ strlen( line ) - 1 ] = '\0';

	config_record_t * record = (config_record_t *) malloc( sizeof( config_record_t ) );
	read_config_attribute_refresh( );
	record->command = read_config_attribute( line, 0 );
	record->pattern = read_config_attribute( line, 0 );
	record->source_path = read_config_attribute( line, 1 );
	record->destination_path = read_config_attribute( line, 1 );

	return record;
}

config_record_t * read_config( config_t * config )
{
	if ( !config )
	{
		return NULL;
	}

	config_record_t * config_head, * config_prev, * config_next;
	config_head = read_config_record( config );
	if ( config_head )
	{
		config_prev = config_head;
		do
		{
			config_next = read_config_record( config );
			config_prev->next = config_next;
			config_prev = config_next;
		}
		while ( config_next != NULL );
	}

	return config_head;
}

static void print_config_record( config_record_t * record )
{
	if ( record )
	{
		printf( "%s %s:\n%s -> %s\n",
			record->command, record->pattern, record->source_path, record->destination_path );
	}
}

void print_config( config_record_t * config_head )
{
	for_each( config_record_t, config_head, config_record )
	{
		print_config_record( config_record );
	}
}
