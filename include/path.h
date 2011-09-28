#ifndef _MTD_PATH_H_
#define _MTD_PATH_H_

char * construct_path( const char * dir_name, const char * file_name );
void destroy_path( char * path );
char * expand_path( char * path );
char * expand_path_length( char * line, unsigned int length );

#endif /* #define _MTD_PATH_H_ */
