#!/bin/sh
echo '#!/bin/sh
exec hexchat-wrapper "$@"' > usr/local/bin/defaultchat
chmod 755 usr/local/bin/defaultchat
echo "setting HexChat as default chat"
