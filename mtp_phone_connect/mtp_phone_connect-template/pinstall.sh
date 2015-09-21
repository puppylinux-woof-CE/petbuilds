#!/bin/sh

# TO BE REMOVED
#[ -f /etc/udev/rules.d/51-android-mtp.rules ] && rm /etc/udev/rules.d/51-android-mtp.rules
[ `pwd` = '/' ] && udevadm control --reload-rules
