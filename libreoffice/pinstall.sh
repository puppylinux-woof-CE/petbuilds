#!/bin/sh
#post-install script.

if [ "`pwd`" != "/" ];then

  echo '#!/bin/sh' > ./usr/local/bin/defaultwordprocessor
  echo 'exec libreoffice --writer "$@"' >> ./usr/local/bin/defaultwordprocessor
  chmod 755  ./usr/local/bin/defaultwordprocessor
  echo '#!/bin/sh' > ./usr/local/bin/defaultspreadsheet
  echo 'exec libreoffice --calc "$@"' >> ./usr/local/bin/defaultspreadsheet
  chmod 755  ./usr/local/bin/defaultspreadsheet

fi
