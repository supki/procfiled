#ifndef _MTD_CONFIG_H_
#define _MTD_CONFIG_H_

typedef struct config_record_s
{
	const char * command;
	const char * file_mask;
	const char * source_path;
	const char * destination_path;
	struct config_record_s * next;
} config_record_t;

char * read_config_line( FILE * fstream );
config_record_t * read_config_record( FILE * fstream );
config_record_t * read_config( FILE * fstream );
void print_config_record( config_record_t * record );
void print_config( config_record_t * record );

#endif /* #define _MTD_CONFIG_H_ */
