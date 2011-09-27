#ifndef _MTD_CONFIG_H_
#define _MTD_CONFIG_H_

#include <stdio.h>

typedef struct config_record_s
{
	const char * command;
	const char * file_mask;
	const char * source_path;
	const char * destination_path;
	struct config_record_s * next;
} config_record_t;

FILE * open_config( void );
config_record_t * read_config( FILE * fstream );
void print_config( config_record_t * record );

#endif /* #define _MTD_CONFIG_H_ */
