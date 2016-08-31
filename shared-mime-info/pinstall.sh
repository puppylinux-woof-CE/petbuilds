#!/bin/sh
if [ "`pwd`" != '/' ];then #woof
	echo "Updating mime database..."
	chroot . update-mime-database usr/share/mime
else
	update-mime-database /usr/share/mime
fi
