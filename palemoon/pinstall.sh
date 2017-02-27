#!/bin/sh

# palemoon
if [ "`pwd`" != "/" ];then
	echo '#!/bin/sh
exec palemoon "$@"' > usr/local/bin/defaultbrowser
chmod 755 usr/local/bin/defaultbrowser
(cd usr/local/bin/; ln -sf defaultbrowser defaulthtmlviewer)
echo "Setting Palemoon as default browser"
fi
