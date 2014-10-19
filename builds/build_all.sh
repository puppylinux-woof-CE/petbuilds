#!/bin/sh

# packages can be built individually if you wish
# to do so just run (for example)
#sh rox_filer.petbuild
# inside of the rox_filer dir

export MWD=`pwd`

. ./build.conf

get_specs() {
	[ -f 0pets_out.specs ] && rm 0pets_out.specs
	cd 0pets_out
	for pet in *.pet; do 
		echo -n "$pet "
		specs=`tar -xvJf "$pet" --no-anchored 'pet.specs' 2>/dev/null`
		cat $specs >> ../0pets_out.specs
		rm -r ${pet%.*}
	done
	cd -
}

for pkg in `cat ORDER`; do
	pkg_exits=`ls ./0pets_out|grep "^$pkg"|grep "pet$"`
	if [ "$pkg_exits" ];then
		echo "$pkg exists ... skipping"
		sleep 0.5
		continue
	fi
	echo
	cd $pkg
	echo "
+=============================================================================+

building $pkg"
	sleep 1 
	sh ${pkg}.petbuild 2>&1 | tee ../0logs/${pkg}build.log
	if [ "$?" -eq 1 ];then 
		echo "$pkg build failure"
		case $HALT_ERRS in
			0) exit 1 ;;
		esac
	fi
	cd -
done
echo "
+=============================================================================+

getting specs"
# get the specs
get_specs
echo


echo "all done!" && exit 0
