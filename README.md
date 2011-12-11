Procfiled
=========================

What is Procfiled?
------------
Procfiled is a daemon automatically working with files. It could manage your ~/Downloads directory:
###config example###
	$ cat ~/.procfiledconf
	move	*.torrent	~/Downloads	~/Downloads/.torrents
	move	*.png		-		~/Pictures
	move	*.jpg		-		-
	move	*.gif		-		-
	move	*.avi		-		~/Videos
	move	*.mp4		-		-
	copy	*.pdf		-		~/Dropbox/Books
	copy	*.chm		-		-

	link	*HD*		~/Videos	~/Videos/HD

With this config Procfiled will move \*.torrent from ~/Downloads to ~/Downloads/.torrents, \*.png, \*.jpg and \*.gif to ~/Pictures, \*.avi and \*.mp4 to ~/Videos; copies \*.pdf and \*.chm to ~/Dropbox/Books and links \*HD\* from ~/Videos to ~/Videos/HD.

Procfiled is based on Inotify subsystem, so you need Linux Kernal >= 2.6.13 and glibc >= 2.4

What about licensing?
---------------------
Procfiled is licensed under GPLv3

How to install (or uninstall) Procfiled?
----------------------------------
###install###
	$ DIR="/usr/local/bin"
	$ ./install.sh --dir=${DIR}
	$ procfiled
###uninstall###
	$ DIR="/usr/local/bin"
	$ ./install.sh --uninstall --dir=${DIR}

How to use it?
--------------
First, you need configuration file in next format:

	<command> <pattern> <from> <to>

Second, you need to start daemon:

	$ procfiled [options]

That's all!

###options###
* `--kill` - Kill the currently running Procfiled session.
* `--foreground (-f)` - Don't detach from console.
* `--conf FILE` - Read configuration options from FILE (defaults: ~/.procfiledconf for user, /etc/procfiled.conf for root).
* `--pidfile FILE` - Write pid into FILE (defaults: ~/.procfiledpid for user, /var/run/procfiled.pid for root).
* `--version` - Display version information and exit.

What is implemented?
--------------------
* auto-updating daemon after configuration file change
* `move` command
* `link` command
* `copy` command

What will be implemented?
-------------------------
*  
