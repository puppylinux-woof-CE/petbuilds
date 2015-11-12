#!/bin/sh
if [ `pwd` != '/' ];then
 echo '$!/bin/sh
exec gpicview "$@"' > usr/local/bin/defaultimageviewer
chmod 755 usr/local/bin/defaultimageviewer
else
 echo '$!/bin/sh
exec gpicview "$@"' > /usr/local/bin/defaultimageviewer
chmod 755 /usr/local/bin/defaultimageviewer
fi
