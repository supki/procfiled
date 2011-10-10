#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wordexp.h>

#include "attribute.h"

#define SYMBOL_SPACE ' '
#define SYMBOL_TAB   '	'

static unsigned int current_position = 0;
static char * attributes_line = NULL;

void init_attributes_line( char * line )
{
	attributes_line = line;
	current_position = 0;
}

static int eat_whitespaces( unsigned int position )
{
	while( ( attributes_line[ position ] == SYMBOL_SPACE ||
		attributes_line[ position ] == SYMBOL_TAB ) &&
		position < strlen( attributes_line ) )
	{
		position++;
	}

	return position;
}

static int eat_not_whitespaces( unsigned int position )
{
	while( attributes_line[ position ] != SYMBOL_SPACE &&
		attributes_line[ position ] != SYMBOL_TAB &&
		position < strlen( attributes_line ) )
	{
		position++;
	}

	return position;
}

char * mode_attribute( char * line, unsigned int length )
{
	return strndup( line, length );
}

char * path_attribute( char * line, unsigned int length )
{
	(void) length;
	wordexp_t wordexp_buffer;
	wordexp( line, &wordexp_buffer, 0 );
	char * expanded = strdup( wordexp_buffer.we_wordv[0] );
	wordfree( &wordexp_buffer );

	return expanded;
}

char * construct_next_attribute( char * (*get_attribute)(char *, unsigned int ) )
{
	unsigned int start_position = eat_whitespaces( current_position );
	if ( start_position == strlen( attributes_line ) ) return NULL;
	current_position = eat_not_whitespaces( start_position );

	return get_attribute( attributes_line + start_position, current_position - start_position );
}
