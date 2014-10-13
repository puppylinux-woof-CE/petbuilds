#!/bin/sh
echo '#!/bin/sh
exec geany "$@"' > usr/local/bin/defaulttexteditor
chmod 755 usr/local/bin/defaulttexteditor
echo "Setting geany as defaulttexteditor"
