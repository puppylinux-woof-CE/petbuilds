#!/bin/sh
# evince
echo '#! /bin/sh
exec evince "$@"' > usr/local/bin/defaultpdfviewer
chmod 755 usr/local/bin/defaultpdfviewer
echo "set evince to defaultpdfviewer"
