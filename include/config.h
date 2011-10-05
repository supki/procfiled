#ifndef _MTD_CONFIG_H_
#define _MTD_CONFIG_H_

#include <stdio.h>

typedef struct config_record_s
{
	int watch_instance;
	const char * command;
	const char * pattern;
	const char * source_path;
	const char * destination_path;
	struct config_record_s * next;
} config_record_t;

typedef FILE config_t;

config_t * open_config( const char * path );
config_record_t * read_config( FILE * fstream );

#endif /* #define _MTD_CONFIG_H_ */
