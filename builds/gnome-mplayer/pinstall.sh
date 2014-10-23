#!/bin/bash
if [ "$(pwd)" != "/" ];then  #woof
  echo "Setting gnome-mplayer as defaultmediaplayer and defaultvideoplayer"
  echo '#!/bin/bash
exec gnome-mplayer "$@"' > usr/local/bin/defaultmediaplayer
  echo '#!/bin/bash
exec gnome-mplayer "$@"' > usr/local/bin/defaultvideoplayer
chmod 755 usr/local/bin/defaultmediaplayer
chmod 755 usr/local/bin/defaultvideoplayer

  else
  Xdialog --yesno "Set gnome-mplayer as the default media player?" 0 0 
  case $? in
  0)echo '#!/bin/bash
exec gnome-mplayer "$@"' > /usr/local/bin/defaultmediaplayer
chmod 755 /usr/local/bin/defaultmediaplayer
  ;;
  *) echo not
  ;;
  esac
fi
