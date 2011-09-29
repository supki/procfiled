#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wordexp.h>

#include "path.h"

char * construct_path( const char * dir_name, const char * file_name )
{
	char * path = (char *) malloc( strlen( dir_name ) + strlen( file_name ) + 2 ); 
	strcpy( path, dir_name);
	strcat( path, "/" );
	strcat( path, file_name );

	return path;
}

void destroy_path( char * path )
{
	free( path );
}

char * expand_path( char * path )
{
	char * expanded = (char *) malloc ( BUFSIZ );

	wordexp_t wordexp_buffer;
	wordexp( path, &wordexp_buffer, 0 );
	strcpy( expanded, wordexp_buffer.we_wordv[0] );
	wordfree( &wordexp_buffer );

	return expanded;
}

char * expand_path_length( char * line, unsigned int length )
{
	char piece[ length + 1 ];
	strncpy( piece, line, length );
	piece[ length ] = '\0';

	return expand_path( piece );
}