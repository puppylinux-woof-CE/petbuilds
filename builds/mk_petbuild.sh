#!/bin/sh

# scriot to generate a petbuild
#set -x
CWD=`pwd`

usage() {
	echo "USAGE:	./${0##*/} <URL/of/PACKAGE> <\"A DESCRIPTION\"> <DEPENDS> <CATEGORY>"
	echo "eg;	./${0##*/} http://example.com/example_source-1.2.3.tar.gz \
\"a light weight example\" +libsample,+libexample BuildingBlock"
	echo "	CATEGORY and DEPENDS are optional"
	echo
	echo "	You can add .desktop and pinstall/puninstall files if you wish"
	echo "	to the resulting program directory which you will find in"
	echo "	$CWD"
	echo "CATEGORIES:
	Desktop
	System
	Setup
	Utility
	Filesystem
	Graphic
	Document
	Business
	Personal
	Network
	Internet
	Multimedia
	Fun"
	echo
	echo "Minimal error checking is done. Garbage goes in, garbage comes out!"
	echo
	echo "	This doesn't work in all cases! YMMV"
	exit 0
}

[ "$1" ] || usage
[ "$2" ] || usage
[ "$3" ] && DEPENDS="$3" || DEPENDS=''
[ "$4" ] && CATEGORY="$4"|| CATEGORY=BuildingBlock

SOURCE="${1##*/}" 
SRCURL="${1%/*}" 
DESCRIPTION="\"$2\"" 

PROG=${SOURCE%-*}
VERSION=${SOURCE##*-}
VERSION=`echo $VERSION | sed 's%\.tar.*%%'`
COMPRESSION=${SOURCE##*.}

mkdir -p "$PROG"

cat > $PROG/${PROG}.petbuild << _PET
# $PROG
# Builds from https://github.com/puppylinux-woof-CE/petbuilds

. ../func
. ../build.conf

URL=$SRCURL
PKG=$PROG
VER=$VERSION
COMP=tar.${COMPRESSION}
DESC=$DESCRIPTION
DEPS=$DEPENDS
CAT=$CATEGORY
DESKTOP=${PROG}.desktop
CWD=\$(pwd)
[ -z "\$MWD" ] && MWD=\$(dirname \$CWD)	

ARCH=\$(uname -m)
case \$ARCH in
 *64) LIBDIR=lib64 ;;
 *) LIBDIR=lib ;;
esac

build() {
	cd \${PKG}-\${VER}
	./configure --prefix=/usr \\
				--localstatedir=/var \\
				--sysconfdir=/etc \\
				--libdir=/usr/\${LIBDIR}
	[ "\$?" -eq 0 ] || exit
	make \$MKFLG
	[ "\$?" -eq 0 ] || exit
	make DESTDIR=\$CWD/\${PKG}-install install
	cd -
}
	
package() {
	get_files \${PKG}-install \${PKG}-\${VER}-\${ARCH}
	(cd \${PKG}-\${VER}; \$MWD/split.sh ../\${PKG}-install)
	# add this recipe
	install -d -m 0755 ./\${PKG}-\${VER}-\${ARCH}/usr/share/doc
	cat \${PKG}.petbuild > ./\${PKG}-\${VER}-\${ARCH}/usr/share/doc/\${PKG}-build-recipe
	if [ -f "\$DESKTOP" ];then
		install -d -m 0755 ./\${PKG}-\${VER}-\${ARCH}/usr/share/applications
		cat \$DESKTOP > ./\${PKG}-\${VER}-\${ARCH}/usr/share/applications/\$DESKTOP
	fi
	# delete any icon cache or library cache
	find ./\${PKG}-\${VER}-\${ARCH} -type f -name '*cache' -delete
	[ -f ./pinstall.sh ] && install -m 0755 pinstall.sh ./\${PKG}-\${VER}-\${ARCH}/
	[ -f ./puninstall.sh ] && install -m 0755 puninstall.sh ./\${PKG}-\${VER}-\${ARCH}/
	for p in \$(ls|grep "\\-\${ARCH}"|grep -v "files\$") ; do
		case \$p in
			*_DEV*) DESC="\$PKG development"; DEPS=+\${PKG} ;;
			*_DOC*) DESC="\$PKG documentation"; DEPS=+\${PKG} ;;
			*_NLS*) DESC="\$PKG locales"; DEPS=+\${PKG} ;;
		esac	
		echo "packaging \$p"	
		dir2pet -x -s -w="\$DESC" -d="\$DEPS" -c="\$CAT" -p=\${p} 2>&1 >/dev/null
		rm -r \$p
		mv \${p}.pet ../0pets_out
	done
	rm -r \${PKG}-install
	rm -r \${PKG}-\${VER}
	mv -f *.files ../0logs
	echo "done!"
}

# main
retrieve \${PKG}-\${VER}.\${COMP}
extract \${PKG}-\${VER}.\${COMP}
build
package	
_PET
