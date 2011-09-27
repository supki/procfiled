#ifndef _MTD_WATCH_H_
#define _MTD_WATCH_H_

#include "config.h"

typedef struct watch_record_s
{
	int wd;
	config_record_t * info;
	struct watch_record_s * next;
} watch_record_t;

watch_record_t * init_watches( int inotify_instance, config_record_t * config_head );
void print_watches( watch_record_t * watch_head );

#endif /* #define _MTD_WATCH_H_ */
