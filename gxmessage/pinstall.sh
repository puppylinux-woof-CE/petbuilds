#!/bin/sh
#post-install script

echo "post-install for gxmessage, creating symlink to replace xmessage"

ln -snf gxmessage usr/bin/xmessage

#...note, some puppies will have the xmessage executable at /usr/X11R7/bin,
# but that's ok, as the new symlink at /usr/bin will be found first and used.
