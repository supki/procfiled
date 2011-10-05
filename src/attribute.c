#include <stdlib.h>
#include <string.h>

#include "config_attribute.h"
#include "path.h"

#define SYMBOL_SPACE ' '
#define SYMBOL_TAB   '	'

static unsigned int current_position = 0;
static char * attribute_line = NULL;

void init_attribute_line( char * line )
{
	attribute_line = line;
	current_position = 0;
}

static int eat_whitespaces( unsigned int position )
{
	while( ( attribute_line[ position ] == SYMBOL_SPACE ||
		attribute_line[ position ] == SYMBOL_TAB ) &&
		position < strlen( attribute_line ) )
	{
		position++;
	}

	return position;
}

static int eat_not_whitespaces( unsigned int position )
{
	while( attribute_line[ position ] != SYMBOL_SPACE &&
		attribute_line[ position ] != SYMBOL_TAB &&
		position < strlen( attribute_line ) )
	{
		position++;
	}

	return position;
}

char * mode_attribute( char * line, unsigned int length )
{
	char * attribute = malloc( length + 1 );
	strncpy( attribute, line, length );
	attribute[ length ] = '\0';
	return attribute;
}

char * path_attribute( char * line, unsigned int length )
{
	char attribute[ length + 1 ];
	strncpy( attribute, line, length );
	attribute[ length ] = '\0';

	return expand_path( attribute );
}

char * construct_next_attribute( char * (*get_attribute)(char *, unsigned int ) )
{
	unsigned int start_position = eat_whitespaces( current_position );
	if ( start_position == strlen( attribute_line ) ) return NULL;
	current_position = eat_not_whitespaces( start_position );

	return get_attribute( attribute_line + start_position, current_position - start_position );
}
