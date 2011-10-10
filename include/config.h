#ifndef _MTD_CONFIG_H_
#define _MTD_CONFIG_H_

#include <stdio.h>

typedef struct config_record_s
{
	int watch_instance;
	const char * name;
	int (*function)( const char * old_name, const char * new_name );
	const char * pattern;
	const char * source_path;
	const char * destination_path;
	struct config_record_s * next;
} config_record_t;

typedef FILE config_t;

config_record_t * read_config( config_t * fstream );
void destroy_config( config_record_t * config_head );

#endif /* #define _MTD_CONFIG_H_ */
