#!/bin/sh

echo "Setting leafpad as defaulttextviewer"

echo '#!/bin/sh
exec leafpad "$@"' > usr/local/bin/defaulttextviewer
chmod 755 usr/local/bin/defaulttextviewer
