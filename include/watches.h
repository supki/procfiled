#ifndef _MTD_WATCH_H_
#define _MTD_WATCH_H_

#define EVENT_SIZE    ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN ( 1024 * ( EVENT_SIZE + 16 ) )

void init_watches( char * config_file );
int get_inotify_events( char * events );
void parse_inotify_event( struct inotify_event * event );

#endif /* #define _MTD_WATCH_H_ */
