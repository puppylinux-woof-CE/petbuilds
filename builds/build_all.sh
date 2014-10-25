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

OLD_PATH=$PATH 

petbuilds_trap_exit() {    
   export PATH=$OLD_PATH 
} 

trap petbuilds_trap_exit EXIT 

petbuilds_bootstrap() { 
	# Get this scripts path so we use our modified scripts rather than the 
	# original ones 
	SDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )" 
	echo "\$SDIR:=$SDIR" 
	if [[ $PATH != *$SDIR* ]]; then 
		export PATH=$SDIR:$PATH 
		DIRTOPET=`which dir2pet` 
		AGE=$(date +%s -r  $DIRTOPET) # =1413022441 is current verion 
		if [ "$AGE" -lt  1413022441 ];then 
			echo "dir2pet  is built before $(date -r $DIRTOPET) ." 
			echo "Get the corresponding one from ..."
			echo "http://distro.ibiblio.org/puppylinux/pet_packages-noarch/dir2pet-0.0.1-noarch.pet"
			exit 1
		fi      
	fi 

} 
petbuilds_bootstrap

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
			0)cd - ; exit 1 ;;
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
