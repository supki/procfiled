#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "attribute.h"
#include "path.h"

#define for_each(type, head, item) \
    for(type * item = head; item != NULL; item = item->next)

config_t * open_config( const char * path )
{
	return fopen( path, "r" );
}

int copy( const char * old_name, const char * new_name )
{
	(void) old_name;
	(void) new_name;
	return 0;
}

static void set_function_by_name( config_record_t * record )
{
	record->function = NULL;
	if ( !strcmp( record->name, "move" ) )
	{
		record->function = rename;
	}
	if ( !strcmp( record->name, "link" ) )
	{
		record->function = symlink;
	}
	if ( !strcmp( record->name, "copy" ) )
	{
		record->function = copy;
	}
}

static config_record_t * read_next_record( config_t * config )
{
	char line[BUFSIZ];
	if ( line != fgets( line, BUFSIZ, config ) ) return NULL;
	line[ strlen( line ) - 1 ] = '\0';
	init_attributes_line( line );

	config_record_t * record = malloc( sizeof( config_record_t ) );
	record->name = construct_next_attribute( mode_attribute );
	set_function_by_name( record );
	record->pattern = construct_next_attribute( mode_attribute );
	record->source_path = construct_next_attribute( path_attribute );
	record->destination_path = construct_next_attribute( path_attribute );

	return record;
}

static int is_not_valid( config_record_t * record )
{
	return ( !record->name ||
		!record->function ||
		!record->pattern ||
		!record->source_path ||
		!record->destination_path );
}

static void destroy_record( config_record_t * config_record )
{
	if ( config_record->name ) free( (void *)config_record->name );
	if ( config_record->pattern ) free( (void *)config_record->pattern );
	if ( config_record->source_path ) free( (void *)config_record->source_path );
	if ( config_record->destination_path ) free( (void *)config_record->destination_path );
	free( config_record );
}

config_record_t * read_config( config_t * config )
{
	if ( !config ) return NULL;

	config_record_t * prev_record = NULL, * record;
	while ( ( record = read_next_record( config ) ) != NULL )
	{
		if ( is_not_valid( record ) )
		{
			destroy_record( record );
			continue;
		}
		record->next = prev_record;
		prev_record = record;
	}

	return prev_record;
}
