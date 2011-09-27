#include <pwd.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"

FILE * open_config( void )
{
	struct passwd * pw = getpwuid( getuid( ) );
	char * config_file_name = (char *) malloc( strlen( pw->pw_dir ) + strlen( "/.mtdconf" ) + 1 );
	strcpy( config_file_name, pw->pw_dir );
	strcat( config_file_name, "/.mtdconf" );

	FILE * fstream = fopen( config_file_name, "r" );

	free( config_file_name );

	return fstream;
}

static char * read_config_line( FILE * fstream )
{
	char line[ LINE_MAX ];
	char * response = fgets( line, LINE_MAX, fstream );
	if ( response != line )
	{
		return NULL;
	}

	char * attribute = (char *) malloc( strlen( line ) );
	strcpy( attribute, line );
	attribute[ strlen( line ) - 1 ] = '\0';

	return attribute;
}

static config_record_t * read_config_record( FILE * fstream )
{
	const char * command = read_config_line( fstream );
	const char * file_mask = read_config_line( fstream );
	const char * source_path = read_config_line( fstream );
	const char * destination_path = read_config_line( fstream );

	if ( !command || !file_mask || !source_path || !destination_path )
	{
		return NULL;
	}

	config_record_t * record = (config_record_t *) malloc( sizeof( config_record_t ) );
	record->command = command;
	record->file_mask = file_mask;
	record->source_path = source_path;
	record->destination_path = destination_path;

	return record;
}

config_record_t * read_config( FILE * fstream )
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
		printf( "What: %s\n", record->file_mask );
		printf( "From: %s\n", record->source_path );
		printf( "To: %s\n", record->destination_path );
	}
}

void print_config( config_record_t * record )
{
	if ( !record )
	{
		return;
	}

	print_config_record( record );

	do
	{
		record = record->next;
		print_config_record( record );
	}
	while ( record->next != NULL );
}
