#!/bin/sh
# don't clobber pfontview if it is set for mime
if [ "`pwd`" != "/" ];then #woof
  grep -q "pfontview" root/Choices/MIME-types/application_x-font-ttf 2>/dev/null
  if [ "$?" != 0 ];then
  echo '#!/bin/bash
exec sfontview "$@"' > root/Choices/MIME-types/application_x-font-ttf
chmod 755 root/Choices/MIME-types/application_x-font-ttf
  fi
else #running os
  grep -q "pfontview" /root/Choices/MIME-types/application_x-font-ttf 2>/dev/null
    if [ "$?" != 0 ];then
    echo '#!/bin/bash
exec sfontview "$@"' > /root/Choices/MIME-types/application_x-font-ttf
chmod 755 /root/Choices/MIME-types/application_x-font-ttf
    fi
fi
