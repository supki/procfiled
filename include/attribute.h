#ifndef _MTD_CONFIG_ATTRIBUTE_H_
#define _MTD_CONFIG_ATTRIBUTE_H_

void init_attributes_line( char * line );
char * mode_attribute( char * line, unsigned int length );
char * path_attribute( char * line, unsigned int length );
char * construct_next_attribute( char * (*get_attribute)(char *, unsigned int ) );

#endif /* #define _MTD_CONFIG_ATTRIBUTE_H_ */
