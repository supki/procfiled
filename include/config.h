#ifndef _MTD_CONFIG_H_
#define _MTD_CONFIG_H_

#include <stdio.h>

typedef struct config_record_s
{
	const char * command;
	const char * pattern;
	const char * source_path;
	const char * destination_path;
	struct config_record_s * next;
} config_record_t;

typedef FILE config_t;

config_t * open_config( void );
config_record_t * read_config( FILE * fstream );
void print_config( config_record_t * config_head );

#endif /* #define _MTD_CONFIG_H_ */
