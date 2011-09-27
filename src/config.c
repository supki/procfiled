#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wordexp.h>

#include "config.h"

#define for_each(type, head, item) \
    for(type * item = head; item != NULL; item = item->next)

config_t * open_config( void )
{
	struct passwd * pw = getpwuid( getuid( ) );
	char * config_file_name = (char *) malloc( strlen( pw->pw_dir ) + strlen( "/.mtdconf" ) + 1 );
	strcpy( config_file_name, pw->pw_dir );
	strcat( config_file_name, "/.mtdconf" );

	config_t * fstream = fopen( config_file_name, "r" );

	free( config_file_name );

	return fstream;
}

static char * read_config_line( config_t * fstream, int is_path )
{
	char * line = (char *) malloc ( BUFSIZ );
	char * response = fgets( line, BUFSIZ, fstream );
	if ( response != line )
	{
		return NULL;
	}
	line[ strlen( line ) - 1 ] = '\0';

	if ( is_path )
	{
		wordexp_t wordexp_buffer;
		wordexp( line, &wordexp_buffer, 0 );
		strcpy( line, wordexp_buffer.we_wordv[0] );
		wordfree( &wordexp_buffer );
	}

	char * attribute = (char *) malloc( strlen( line ) + 1 );
	strcpy( attribute, line );
	free( line );

	return attribute;
}

static config_record_t * read_config_record( config_t * fstream )
{
	const char * command = read_config_line( fstream, 0 );
	const char * pattern = read_config_line( fstream, 0 );
	const char * source_path = read_config_line( fstream, 1 );
	const char * destination_path = read_config_line( fstream, 1 );

	if ( !command || !pattern || !source_path || !destination_path )
	{
		return NULL;
	}

	config_record_t * record = (config_record_t *) malloc( sizeof( config_record_t ) );
	record->command = command;
	record->pattern = pattern;
	record->source_path = source_path;
	record->destination_path = destination_path;

	return record;
}

config_record_t * read_config( config_t * fstream )
{
	if ( !fstream )
	{
		return NULL;
	}

	config_record_t * head, * prev, * next;
	head = read_config_record( fstream );
	if ( head )
	{
		prev = head;
		do
		{
			next = read_config_record( fstream );
			prev->next = next;
			prev = next;
		}
		while ( next != NULL );
	}

	return head;
}

static void print_config_record( config_record_t * record )
{
	if ( record )
	{
		printf( "Do: %s\n", record->command );
		printf( "What: %s\n", record->pattern );
		printf( "From: %s\n", record->source_path );
		printf( "To: %s\n", record->destination_path );
	}
}

void print_config( config_record_t * config_head )
{
	for_each( config_record_t, config_head, config_record )
	{
		print_config_record( config_record );
	}
}
