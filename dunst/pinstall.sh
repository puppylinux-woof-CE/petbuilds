#!/bin/sh
if [ `pwd` != "/" ];then
 
 [ ! -h usr/bin/notify ] && (cd usr/bin;ln -sf notify-send notify)
else
 mkdir -p ${HOME}/.config/dunst
 cp -a /usr/share/dunst/dunstrc ${HOME}/.config/dunst/
 [ ! -h /usr/bin/notify ] && (cd /usr/bin;ln -sf notify-send notify)
 xdg-open $XDG_CONFIG_HOME/dunst.desktop
fi
