#!/bin/sh
if [ '`pwd`' = "/" ];then
	Xdialog -yesno "Would you like to make /etc/init.d/minidlna executable
	and configure /etc/minidlna.conf for your system?" 0 0 0
	case $? in
		0) chmod 755 /etc/init.d/minidlna
			cp -a /etc/minidlna.conf.sample /etc/minidlna.conf
			defaulttexteditor /etc/minidlna.conf &
			;;
		*)exit ;;
	esac
fi
