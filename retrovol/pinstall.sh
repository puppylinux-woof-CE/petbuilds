#!/bin/sh

if [ "`pwd`" != "/" ];then
	echo '[Desktop Entry]
Encoding=UTF-8
Type=Application
NoDisplay=true
Name=retrovol
Exec=retrovol -hide' > root/.config/autostart/retrovol.desktop
else
	echo '[Desktop Entry]
Encoding=UTF-8
Type=Application
NoDisplay=true
Name=retrovol
Exec=retrovol -hide' > /root/.config/autostart/retrovol.desktop
fi
