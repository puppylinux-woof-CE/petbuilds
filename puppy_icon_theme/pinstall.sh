#!/bin/sh
# we don't wanna ship woof with a .cache file so we only 
# execute this if we are in a running install...
CWD=`pwd`
if [ "$CWD" == '/' ];then
	gtk-update-icon-cache -f "/usr/share/icons/Puppy Standard/"
fi
