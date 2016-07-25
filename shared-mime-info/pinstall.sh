#!/bin/sh
if [ "`pwd`" != '/' ];then #woof
	echo "updating mime database"
	chroot . update-mime-database /usr/share/mime
else
	update-mime-database /usr/share/mime
fi
