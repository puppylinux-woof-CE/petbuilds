#!/bin/sh
if [ "`pwd`" = "/" ];then
	Xdialog -msgbox "Please enable dropbear in the Service Manager" 0 0 0
fi
