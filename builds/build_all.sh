#!/bin/sh

# packages can be built individually if you wish
# to do so just run (for example)
#sh rox_filer.petbuild
# inside of the rox_filer dir

CWD=`pwd`

for pkg in `cat ORDER`; do
	cd $pkg
	sh ${pkg}.petbuild
	[ "$?" -eq 1 ] && echo "$pkg build failure" && exit 1
	cd -
done

echo "all done!" && exit 0
