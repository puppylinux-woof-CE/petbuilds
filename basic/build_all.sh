#!/bin/sh

# packages can be built individually if you wish

REALPATH_BUILD_ALL=$(realpath ${0%build_all.sh})
if [ "$REALPATH_BUILD_ALL" = "$PWD" ] ; then
	z0base_dir=""
else
	export z0base_dir="$REALPATH_BUILD_ALL"
fi

if [ -n "$z0base_dir" -a -f ${z0base_dir}/build.conf ] ; then
	. ${z0base_dir}/build.conf
	export md5sumstxt=${z0base_dir}/md5sums.txt
	zORDER=${z0base_dir}/ORDER
	mkdir -p local-repositories/petbuilds/0sources
	export z0sources=$(cd local-repositories/petbuilds/0sources ; pwd)
	mkdir -p petbuilds-out
	export MWD=$(cd petbuilds-out ; pwd)
	[ ! -f petbuilds-out/func ] && ln -s ../${0%/build_all.sh}/func petbuilds-out/
	[ ! -f petbuilds-out/build.conf ] && ln -s ../${0%/build_all.sh}/build.conf petbuilds-out/
	[ ! -f petbuilds-out/split.sh ] && ln -s ../${0%/build_all.sh}/split.sh petbuilds-out/
	. /etc/DISTRO_SPECS
	export z0pets_out=${MWD}/0pets_out/${DISTRO_TARGETARCH}/${DISTRO_BINARY_COMPAT}/${DISTRO_DB_SUBNAME}
	mkdir -p $z0pets_out
	z0pets_out_specs=${z0pets_out}/0pets_out.specs
	export z0logs=${MWD}/0logs/${DISTRO_TARGETARCH}/${DISTRO_BINARY_COMPAT}/${DISTRO_DB_SUBNAME}
	mkdir -p $z0logs
elif [ -f ./build.conf ] ; then
	. ./build.conf
	export MWD=`pwd`
	zORDER=ORDER
	z0pets_out=${MWD}/0pets_out
	z0pets_out_specs=${MWD}/0pets_out.specs
else
	echo "Error: unable to find build.conf"
	exit 1
fi


usage() {
	echo
	echo "Usage:-"
	echo
	echo "Just run ${0##*/}"
	echo "All petbuilds in the queue will be run and hopefully built"
	echo
	echo "If you only want to build a single package, then use"
	echo "the generic  package name as the argument to ${0##*/}"
	echo
	echo "eg; 	${0##*/} rox-filer"
	
	echo
	echo "	Licenced under the GPLv2"
	echo "	report bugs to https://github.com/puppylinux-woof-CE/petbuilds"
	exit 0
}

get_specs() {
	[ -f $z0pets_out_specs ] && rm $z0pets_out_specs
	cd $z0pets_out
	for pet in *.pet; do 
		echo -n "$pet "
		specs=`tar -xvJf "$pet" --no-anchored 'pet.specs' 2>/dev/null`
		cat $specs >> $z0pets_out_specs
		rm -rf ${pet%.*}
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

build_it() {
	pkg=${1/\//} # remove trailing slash if using bash completion
	case "$1" in
		-h|-help|--help) usage ;;

		# allow ORDER-* files to be selected from command line
		*ORDER*)
			if [ -f "$1" ]; then
				zORDER="$1"
				build_all
				get_specs
				exit
			elif [ -f "${z0base_dir}/$1" ]; then
				zORDER="${z0base_dir}/$1"
				build_all
				get_specs
				exit
			fi
		;;
	esac
	[ -d "pkgs/$1" -o -d "${z0base_dir}/pkgs/$1" ] || usage
	echo "
+=============================================================================+

building $pkg"
	echo -n > ${z0logs}/${pkg}build.log
	if [ -n "$z0base_dir" ] ; then
		mkdir -p ${MWD}/pkgs/$pkg
		cp -a ${z0base_dir}/pkgs/$pkg ${MWD}/pkgs/
		[ "`which git`" != "" ] && echo "building $pkg from petbuild commit $(cd ${z0base_dir}/pkgs ; git log -1 $pkg | grep commit | cut -f 2 -d ' ' | cut -c -7)" >> ${z0logs}/${pkg}build.log
		cd ${MWD}/pkgs/$pkg
	else
		[ "`which git`" != "" ] && echo "building $pkg from petbuild commit $(cd pkgs ; git log -1 $pkg | grep commit | cut -f 2 -d ' ' | cut -c -7)" >> ${z0logs}/${pkg}build.log
		cd pkgs/$pkg
	fi
	sh ${pkg}.petbuild 2>&1 | tee -a ${z0logs}/${pkg}build.log
	cd -
	echo "done building $pkg"
	exit
}

build_all() {
	for pkg in `cat $zORDER`; do
		pkg_exits=`ls ${z0pets_out}|grep "^$pkg"|grep "pet$"`
		if [ "$pkg_exits" ];then
			echo "$pkg exists ... skipping"
			sleep 0.5
			continue
		fi
		echo
		echo "
+=============================================================================+

building $pkg"
		sleep 1 
		if [ -n "$z0base_dir" ] ; then
			mkdir -p ${MWD}/pkgs/$pkg
			cp -a ${z0base_dir}/pkgs/$pkg ${MWD}/pkgs/
			cd ${MWD}/pkgs/$pkg
		else
			cd pkgs/$pkg
		fi
		sh ${pkg}.petbuild 2>&1 | tee ${z0logs}/${pkg}build.log
		if [ "$?" -eq 1 ];then 
			echo "$pkg build failure"
			case $HALT_ERRS in
				0)cd - ; exit 1 ;;
			esac
		fi
		cd -
	done
}

[ "$1" ] && build_it "$1" || build_all

echo "
+=============================================================================+

getting specs"
# get the specs
get_specs
echo


echo "all done!" && exit 0
