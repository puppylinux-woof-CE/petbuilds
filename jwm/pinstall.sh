#!/bin/sh
#post-install script.
#Puppy Linux
#Woof: if current directory is rootfs-complete, it has the final filesystem.

if [ "`pwd`" != "/" ];then

 if [ ! -f ./usr/bin/fvwm95 ];then
  if [ ! -f ./usr/bin/fbpanel ];then
   if [ ! -f ./usr/bin/lxpanel ];then
    echo -n "jwm" > ./etc/windowmanager
   fi
  fi
 fi
 #if [ -x usr/local/bin/fixjwmrc ];then
	#echo "Attempting to fix old jwm config files"
	#chroot . usr/local/bin/fixjwmrc
 #fi
#else
 #Xdialog -title "Fix JWM Configs" \
	#-yesno "This is a new version of JWM
			#and configs may need upgrading.
			#Click 'Yes' to upgrade, 'No' to quit.
			
			#You can run /usr/local/bin/fixjwmrc later." 0 0 || \
			#exit
	#[ ! -x /usr/local/bin/fixjwmrc ] && \
	#Xdialog -title "Fix JWM Configs" -msgbox "Fail" 0 0 && \
	#exit
	#/usr/local/bin/fixjwmrc | xmessage -name 'Fix JWM Configs' -file -
	
fi

#end#
