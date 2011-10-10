MTD - Move Torrent Daemon
=========================

What is MTD?
------------
Move Torrent Daemon is a daemon, that automatically works with files. It could manage your ~/Downloads directory, for example. Consider:
###config example###
	$ cat ~/.mtdconf
	move	*.torrent	~/Downloads	~/.torrents
	move	*.png		~/Downloads	~/Pictures
	move	*.jpg		~/Downloads	~/Pictures
	move	*.gif		~/Downloads	~/Pictures
	move	*.avi		~/Downloads	~/Videos
	move	*.mp4		~/Downloads	~/Videos
	copy	*.chm		~/Downloads	~/Dropbox/Books
	copy	*.pdf		~/Downloads	~/Dropbox/Books

	link	*HD*		~/Videos	~/Videos/HD

MTD is based on Inotify subsystem, so you need Linux Kernal >= 2.6.13 and glibc >= 2.4

What about licensing?
---------------------
MTD is licensed under GPLv3

How to install (or uninstall) MTD?
----------------------------------
###install###
	$ DIR="/usr/local/bin"
	$ ./install.sh --dir=${DIR}
	$ mtd
###uninstall###
	$ DIR="/usr/local/bin"
	$ ./install.sh --uninstall --dir=${DIR}

How to use it?
--------------
First, you need configuration file in next format:

	<command> <pattern> <from> <to>

Second, you need to start daemon:

	$ mtd [options]

That's all!

###options###
* `--kill` - Kill the currently running MTD session.
* `--foreground (-f)` - Don't detach from console.
* `--conf FILE` - Read configuration options from FILE (defaults: ~/.mtdconf for user, /etc/mtd.conf for root).
* `--pidfile FILE` - Write pid into FILE (defaults: ~/.mtdpid for user, /var/run/mtd.pid for root).
* `--version` - Display version information and exit.

What is implemented?
--------------------
* auto-updating daemon after configuration file change
* `move` command
* `link` command
* `copy` command

What will be implemented?
-------------------------

Who did this?
-------------
Matvey Aksenov
matvey.aksenov@gmail.com
