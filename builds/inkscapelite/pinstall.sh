#!/bin/sh

echo '#!/bin/sh' > ./usr/local/bin/defaultdraw
echo 'exec inkscapelite "$@"' >> ./usr/local/bin/defaultdraw
