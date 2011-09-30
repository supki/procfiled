#!/bin/sh
set -e

software="mtd"

while [ $# -ge 1 ]; do
	case $1 in
		--dir )
			if [ -d "$2" ]; then
				dir="$2"
				shift
			fi
			shift
			;;

		--uninstall )
			uninstall_flag="y"
			shift
			;;
	esac
done

if [ -z "${dir}" ]; then
	echo "$0 [--uninstall] --dir DIRECTORY"
	exit -1
fi

if [ -z "${uninstall_flag}" ]; then
	make -s SOFTWARE=${software} rebuild
	cp --force ${software} ${dir}/${software}
	echo "Installation is complete."
else
	if [ -e "${dir}/${software}" ]; then
		rm ${dir}/${software}
		echo "Uninstallation is complete."
	else
		echo "Nothing to uninstall!"
	fi
fi
