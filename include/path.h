#ifndef _MTD_PATH_H_
#define _MTD_PATH_H_

char * construct_path( const char * dir_name, const char * file_name );
void destroy_path( char * path );
char * shell_expand_path( char * path );

#endif /* #define _MTD_PATH_H_ */
