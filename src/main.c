#include <errno.h>
#include <stdlib.h>
#include <sys/inotify.h>

#include "daemonize.h"
#include "logger.h"
#include "watches.h"

#define for_each_inotify_event( events, length ) \
	for ( int i = 0; i < length; i += EVENT_SIZE + ( (struct inotify_event *) &events[ i ] )->len )

int main( int argc, char * argv[] )
{
	get_options( argc, argv );
	daemonize( );
	init_watches( get_configfile_path( ) );

	char events[ EVENT_BUF_LEN ];
	while ( 1 )
	{
		int length = get_inotify_events( events );
		if ( length < 0 )
		{
			if ( errno == EINTR )
			{
				log_notice( "Inotify events read function was interrupted, probably OS was going to suspend." );
				continue;
			}
			break;
		}

		for_each_inotify_event( events, length )
		{
			parse_inotify_event( (struct inotify_event *) &events[ i ] );
		}
	}

	return EXIT_FAILURE;
}
