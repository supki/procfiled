#ifndef _MTD_LOGGER_H_
#define _MTD_LOGGER_H_

void log_syslog_start( void );
void log_end( void );
void log_info( const char * format, ... );
void log_notice( const char * format, ... );
void log_warning( const char * format, ... );
void log_error_and_exit( const char * format, ... );

#endif /* #define _MTD_LOGGER_H_ */
