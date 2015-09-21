#!/bin/sh
# directly pilfered from new2dir
#support build number

[ "$1" ] && tgt="${1}"

TIMEOUT='-t 1'
ARCH=`uname -m`
if [ ! "$CPUTYPE" ];then
  if [ -f config.log ];then
  BuildCPU=`grep -m1 '^build_cpu='.*'' config.log |cut -f 2 -d "'"`
  HostCPU=`grep -m1 '^host_cpu='.*'' config.log |cut -f 2 -d "'"`
    if [ "$HostCPU" != "$BuildCPU" ];then
    echo "WARNING build_cpu='$BuildCPU' NOT host_cpu='$HostCPU'"
    fi
  CPUTYPE="$BuildCPU"
  fi
  if [ ! "$CPUTYPE" ];then
    if [ ! "$TIMEOUT" ];then
    CPUTYPE="i486"
    else
    CPUTYPE="$ARCH"
    fi
  else
  echo "Found '$CPUTYPE'"
  fi
fi

[ -z "$2" ] || CPUTYPE=${CPUTYPE}_${2}
CURRDIR="`pwd`"
PKGDIR="../`basename "$CURRDIR"`"
xPKGDIR="`basename "$CURRDIR"`"
EXE_TARGETDIR="${PKGDIR}-${CPUTYPE}" #relative path.
EXE_PKGNAME="`basename $EXE_TARGETDIR`"
RELPATH="`dirname $EXE_TARGETDIR`"
#difficult task, separate package name from version part... 
#not perfect, some start with non-numeric version info...
xNAMEONLY="`echo -n "$xPKGDIR" | sed -e 's/\-[0-9].*$//g'`"
#...if that fails, do it the old way...
[ "$xNAMEONLY" = "$xPKGDIR" ] && xNAMEONLY="`echo "$xPKGDIR" | cut -f 1 -d "-"`"
NAMEONLY="${RELPATH}/${xNAMEONLY}"
#abasename="`basename ${PKGDIR}`"
apattern="s/${xNAMEONLY}\\-//g"
VERONLY="`echo -n "$xPKGDIR" | sed -e "$apattern"`"
DOC_TARGETDIR="${NAMEONLY}_DOC-${VERONLY}-${CPUTYPE}"
DEV_TARGETDIR="${NAMEONLY}_DEV-${VERONLY}-${CPUTYPE}"
NLS_TARGETDIR="${NAMEONLY}_NLS-${VERONLY}-${CPUTYPE}"

NLSSPLIT=yes
DOCSPLIT=yes
DEVSPLIT=yes
EXESPLIT=yes

mkdir "$EXE_TARGETDIR" 2>/dev/null

#cat ${RELPATH}/${EXE_PKGNAME}.files |
while read ONEFILE
do
 ONEFILE="../${ONEFILE}"
 ONEBASE="`basename "$ONEFILE"`"
 ONEPATH="`dirname "$ONEFILE"`"
 echo "Processing ${ONEFILE}"
 #echo $ONEPATH
 NEWPATH="`echo $ONEPATH|sed "s%$tgt%%"`"
 #echo $NEWPATH
 [ "$ONEFILE" = "$tgt" ] && continue
 #strip the file...
 if [ ! -h "$ONEFILE" ];then #make sure it isn't a symlink
  file "$ONEFILE" | grep 'ELF' | grep -q 'shared object' && strip --strip-debug "$ONEFILE"
  file "$ONEFILE" | grep 'ELF' | grep -q 'executable' && strip --strip-unneeded "$ONEFILE"
 fi
 sync

 if [ "$DEVSPLIT" = "yes" ];then
  #find out if this is development file...
  DEVFILE="`echo -n "$ONEFILE" | grep -E '/include/|/pkgconfig/|/aclocal|/cvs/|/svn/'`"
  if [ "$DEVFILE" ];then
   mkdir -p "${DEV_TARGETDIR}/${NEWPATH}" 2>/dev/null
   cp -af "$ONEFILE" "${DEV_TARGETDIR}/${NEWPATH}/" 2>/dev/null
   continue
  fi
  
  #all .a and .la files... and any stray .m4 files...
  LAFILE="`echo -n "$ONEBASE" | grep --extended-regexp '\.a$|\.la$|\.m4$'`"
  if [ "$LAFILE" ];then
    mkdir -p "${DEV_TARGETDIR}/${NEWPATH}" 2>/dev/null
    cp -af "$ONEFILE" "${DEV_TARGETDIR}/${NEWPATH}/" 2>/dev/null
   continue
  fi  
 fi

 if [ "$NLSSPLIT" = "yes" ];then
  #find out if this is an international language file...
  NLSFILE="`echo -n "$ONEFILE" | grep -E '/locale|/nls|/i18n'`"
  if [ "$NLSFILE" ];then
   mkdir -p "${NLS_TARGETDIR}/${NEWPATH}" 2>/dev/null
   cp -af "$ONEFILE" "${NLS_TARGETDIR}/${NEWPATH}/" 2>/dev/null
   continue
  fi
 fi

 if [ "$DOCSPLIT" = "yes" ];then
  #find out if this is a documentation file...
  DOCFILE="`echo "$ONEFILE" | grep -E '/man|/doc|/docs|/info|/gtk-doc|/faq|/manual|/examples|/help|/htdocs'|grep -v '\.h'`" #eg 'documents.h' in geany
  if [ "$DOCFILE" != "" ];then
   mkdir -p "${DOC_TARGETDIR}/${NEWPATH}" 2>/dev/null
   cp -af "$ONEFILE" "${DOC_TARGETDIR}/${NEWPATH}/" 2>/dev/null
   continue
  fi
 fi

 #anything left over goes into the main 'executable' package...
 if [ "$EXESPLIT" = "yes" ];then
  mkdir -p "${EXE_TARGETDIR}/${NEWPATH}" 2>/dev/null
  cp -af "$ONEFILE" "${EXE_TARGETDIR}/${NEWPATH}/" 2>/dev/null
 fi
done < ${RELPATH}/${EXE_PKGNAME}.files

#130121 grab a .pot if it exists...
sync
pnPTN="/${xNAMEONLY}.pot"
if [ "`grep "$pnPTN" ${RELPATH}/${EXE_PKGNAME}.files`" = "" ];then
 FNDPOT="$(find ${CURRDIR}/ -type f -name "${xNAMEONLY}.pot" | head -n 1)"
 if [ "$FNDPOT" ];then
  mkdir -p ${EXE_TARGETDIR}/usr/share/doc/nls/${xNAMEONLY} 2>/dev/null
  cp -f "$FNDPOT" ${EXE_TARGETDIR}/usr/share/doc/nls/${xNAMEONLY}/
  echo "${tgt#*/}/usr/share/doc/nls/${xNAMEONLY}/" >> ${RELPATH}/${EXE_PKGNAME}.files
  echo "${tgt#*/}/usr/share/doc/nls/${xNAMEONLY}/${xNAMEONLY}.pot" >> ${RELPATH}/${EXE_PKGNAME}.files
 fi
fi

sync

echo "all done"
exit

###END###
