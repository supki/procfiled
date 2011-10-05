#include <fcntl.h>
#include <stdlib.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <unistd.h>

#include "command.h"

int copy( const char * old_name, const char * new_name )
{
	struct stat stat_buf;
	if ( stat( old_name, &stat_buf ) == -1 ) return -1;

	int source = open( old_name, O_RDONLY );
	int target = open( new_name, O_WRONLY | O_CREAT | O_TRUNC, stat_buf.st_mode );
	if ( source == -1 || target == -1 ) return -1;

	if ( sendfile( target, source, NULL, stat_buf.st_size ) == -1 ) return -1;

	close (source);
	close (target);

	return 0;
}

