/*
 * Move Torrent Daemon
 *
 * Automatically moves .torrent files from SRC_PATH to DEST_PATH.
 * Based on Inotify
 *
 */
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <unistd.h>

#define DAEMON_NAME   "mtd"
#define EVENT_SIZE    ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN ( 1024 * ( EVENT_SIZE + 16 ) )
#define SRC_PATH      "/home/maksenov/Downloads/"
#define DEST_PATH     "/home/maksenov/Downloads/.torrent/"
#define TORRENT_EXT   "torrent"

void signal_handler( int signal )
{
	syslog( LOG_WARNING, "Unhandled signal (%d) %s", strsignal( signal ) );
}

int main( )
{
	signal(SIGHUP, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGINT, signal_handler);
	signal(SIGQUIT, signal_handler);

	syslog( LOG_INFO, "%s daemon starting up", DAEMON_NAME );

	setlogmask( LOG_UPTO( LOG_INFO ) );
	openlog( DAEMON_NAME, LOG_CONS, LOG_USER );

	syslog(LOG_INFO, "starting the daemonizing process");

	pid_t pid = fork( );
	if ( pid < 0 )
	{
		exit( EXIT_FAILURE );
	}
	if ( pid > 0 )
	{
		exit( EXIT_SUCCESS );
	}

	umask(0);

	pid_t sid = setsid( );
	if ( sid < 0 )
	{
		exit( EXIT_FAILURE );
	}

	if ( ( chdir( "/" ) ) < 0 )
	{
		exit( EXIT_FAILURE );
	}

	close( STDIN_FILENO );
	close( STDOUT_FILENO );
	close( STDERR_FILENO );

	int inotify_instance = inotify_init( );
	int watch_instance = inotify_add_watch( inotify_instance, SRC_PATH, IN_CREATE );

	char buffer[ EVENT_BUF_LEN ];
	while ( 1 )
	{
		struct inotify_event * event;

		int length = read( inotify_instance, (void *)buffer, EVENT_BUF_LEN );
		if ( length < 0 )
		{
			return EXIT_FAILURE;
		}

		int i = 0;
		while ( i < length )
		{
			struct inotify_event * event = ( struct inotify_event * ) &buffer[ i ];

			/* Check if created file is not a directory */
			if ( ( event->len ) && ( event->mask & IN_CREATE ) && !( event->mask & IN_ISDIR ) )
			{
				/* Check if created file is .torrent file */
				if ( !strcmp( event->name + strlen( event->name ) - strlen( TORRENT_EXT ), TORRENT_EXT ) )
				{
					char old_path[256], new_path[256];
					strcpy( old_path, SRC_PATH );
					strcpy( old_path + strlen( SRC_PATH ), event->name );
					strcpy( new_path, DEST_PATH );
					strcpy( new_path + strlen( DEST_PATH ), event->name );
					
					if ( rename( old_path, new_path ) == -1 )
					{
						return EXIT_FAILURE;
					}
				}
			}

			i += EVENT_SIZE + event->len;
		}
	}

	inotify_rm_watch( inotify_instance, watch_instance );
	close( inotify_instance );

	syslog( LOG_INFO, "%s daemon exiting", DAEMON_NAME );

	return EXIT_SUCCESS;
}
