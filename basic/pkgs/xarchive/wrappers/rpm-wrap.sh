#! /bin/bash
# rpm-wrap.sh - bash rpm wrapper for xarchive frontend
# Copyright (C) 2005 Lee Bigelow <ligelowbee@yahoo.com> 
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#110808 rerwin: Replace all redirections to stderr to be appends, to protect error log.

# set up exit status variables
E_UNSUPPORTED=65

# Supported file extentions for tar 
EXTS="rpm"

# Setup awk program
AWK_PROGS="mawk gawk awk"
AWK_PROG=""
for awkprog in $AWK_PROGS; do
    if [ "$(which $awkprog)" ]; then
        AWK_PROG="$awkprog"
        break
    fi
done

# Programs to wrap
RPM_PROG="rpm"
RPM2CPIO_PROG="rpm2cpio"
CPIO_PROG="cpio"

# setup variables opt and archive.
# the shifting will leave the files passed as
# all the remaining args "$@"
opt="$1"
shift 1
archive="$1"
shift 1

# the option switches
case "$opt" in
    -i) # info: output supported extentions for progs that exist
        if [ ! "$AWK_PROG" ]; then
            echo none of the awk programs $AWK_PROGS found >> /dev/stderr
            echo extentions $EXTS ignored >> /dev/stderr
        elif [ ! "$(which $RPM2CPIO_PROG)" ]; then
            echo $RPM2CPIO_PROG required by $0, but not found >> /dev/stderr
            echo extentions $EXTS ignored >> /dev/stderr
        elif [ ! "$(which $CPIO_PROG)" ]; then
            echo $CPIO_PROG required by $0, but not found >> /dev/stderr
            echo extentions $EXTS ignored >> /dev/stderr
        elif [ "$(which $RPM_PROG)" ]; then
            for ext in $EXTS; do
                printf "%s;" $ext
            done
        else
            echo program $RPM_PROG not found >> /dev/stderr 
            echo extentions $EXTS ignored >> /dev/stderr
        fi
        printf "\n"
        exit
        ;;

    -o) # open: mangle output of rpm cmd for xarchive 
        RP=$(which $RPM_PROG 2>/dev/null)
        if [ -L $RP ] ; then
          # busybox
          $RPM_PROG -qpl "$archive" |
          while read file
          do
             #name,size,attr,uid,gid,date,time,link
             echo "$file;?;?;?;?;?;?;?"
          done
          exit

        else
          # format of output:
          # usr grp attr size time(epoch) name 
          # 1   2   3    4    5           6
          $RPM_PROG -q --qf '[%{FILEUSERNAME} %{FILEGROUPNAME} %{FILEMODES:perms} %{FILESIZES} %{FILEMTIMES} %{FILENAMES}\n]' -p "$archive" | $AWK_PROG '
          {
          uid=$1
          gid=$2
          attr=$3
          size=$4
          time_cmd="date -d \"1970-01-01 UTC " $5 " seconds\" +\"%H:%M\""
          time_cmd | getline time
          close(time_cmd)
          date_cmd="date -d \"1970-01-01 UTC " $5 " seconds\" +\"%Y-%m-%d\""
          date_cmd | getline date
          close(date_cmd)
          
          split($0, linesplit, $5 " ")
          name=linesplit[2] 
          link="-"          
          printf "%s;%s;%s;%s;%s;%s;%s;%s\n",name,size,attr,uid,gid,date,time,link
          }'
        fi
        exit
        ;;
    -a) # adding to archive unsupported
        # use appropriate rpm tools to build rpms
        exit $E_UNSUPPORTED
        ;;

    -n) # create new archive unsupported 
        # use appropriate rpm tools to build rpms
        exit $E_UNSUPPORTED
        ;;

    -r) # removing from archive unsupported 
        # use appropriate rpm tools to modify rpms
        exit $E_UNSUPPORTED
        ;;

    -e) # extract: from archive passed files 
        # convert rpm to a temporary cpio file
        tmpcpio="$(mktemp -t cpiotmp.XXXXXX)"
        $RPM2CPIO_PROG "$archive" > "$tmpcpio"
        # extract files from the temporary cpio
        while [ "$1" ]; do
            $CPIO_PROG --no-absolute-filenames -idv ".$1" < "$tmpcpio"
            shift 1
        done
        wrapper_status=$?
        # remove temporary cpio
        rm "$tmpcpio"
        exit $wrapper_status
        ;;

     *) echo "error, option $opt not supported"
        echo "use one of these:" 
        echo "-i                #info" 
        echo "-o archive        #open" 
        echo "-a archive files  #add" 
        echo "-n archive file   #new" 
        echo "-r archive files  #remove" 
        echo "-e archive files  #extract" 
        exit
esac
