#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <unistd.h>

#include "config.h"
#include "attribute.h"

#define for_each(type, head, item) \
	for(type * item = head; item != NULL; item = item->next)

static int copy( const char * old_name, const char * new_name )
{
	struct stat stat_buf;
	if ( stat( old_name, &stat_buf ) == -1 )
	{
		errno = EINVAL;
		return -1;
	}

	int source = open( old_name, O_RDONLY );
	int target = open( new_name, O_WRONLY | O_CREAT | O_TRUNC, stat_buf.st_mode );
	if ( source == -1 )
	{
		errno = EBADF;
		return -1;
	}
	if ( target == -1 )
	{
		close( source );
		errno = EBADF;
		return -1;
	}

	if ( sendfile( target, source, NULL, stat_buf.st_size ) == -1 )
	{
		close( source );
		close( target );
		errno = EACCES;
		return -1;
	}

	close( source );
	close( target );

	return 0;
}


static int (*set_function_by_name( const char * name ))( const char *, const char * )
{
	if ( !name ) return NULL;
	if ( !strcmp( name, "move" ) ) return rename;
	if ( !strcmp( name, "link" ) ) return symlink;
	if ( !strcmp( name, "copy" ) ) return copy;
	return NULL;
}

static config_record_t * read_next_record( config_t * config )
{
	char * line = NULL;
	size_t size;
	if ( getline( &line, &size, config ) == -1 ) return NULL;
	line[ strlen( line ) - 1 ] = '\0';
	init_attributes_line( line );

	config_record_t * record = malloc( sizeof( config_record_t ) );
	record->name = construct_next_attribute( mode_attribute );
	record->function = set_function_by_name( record->name );
	record->pattern = construct_next_attribute( mode_attribute );
	record->source_path = construct_next_attribute( path_attribute );
	record->destination_path = construct_next_attribute( path_attribute );

	free( line );

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
	config_record = NULL;
}

void destroy_config( config_record_t * config_head )
{
	config_record_t * config_record = config_head;
	while( config_record != NULL )
	{
		config_record_t * next = config_record->next;
		destroy_record( config_record );
		config_record = next;
	}
}

config_record_t * read_config( config_t * config )
{
	if ( !config ) return NULL;
	rewind( config );

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
