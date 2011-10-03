#ifndef _MTD_WATCH_H_
#define _MTD_WATCH_H_

void init_watches( char * config_file, int inotify_instance );
void print_watches( void );
void parse_inotify_event( struct inotify_event * event );

#endif /* #define _MTD_WATCH_H_ */
