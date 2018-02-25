#!/bin/sh
if [ "`pwd`" = '/' ];then
	update-mime-database /usr/share/mime
fi
