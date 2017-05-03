#!/bin/sh
if [ "`pwd`" != '/' ];then
	echo '#!/bin/sh
exec gnumeric "$@"' > usr/local/bin/defaultspreadsheet
	chmod 755 usr/local/bin/defaultspreadsheet
else
	echo '#!/bin/sh
exec gnumeric "$@"' > /usr/local/bin/defaultspreadsheet
	chmod 755 /usr/local/bin/defaultspreadsheet
fi
