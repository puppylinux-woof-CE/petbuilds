#!/bin/sh
# we don't wanna ship woof with a .cache file so we only 
# execute this if we are in a running install...
CWD=`pwd`
if [ "$CWD" == '/' ];then
	gtk-update-icon-cache -f "/usr/share/icons/Puppy Flat/"
fi

# construct the ROX desktop theme
echo -n "Constructing Puppy Flat ROX desktop icon theme... "
TGT_DIR=usr/local/lib/X11/themes/PuppyFlat
SRC_DIR='usr/share/icons/Puppy Flat/48'
mkdir -p ${TGT_DIR}
for m in application-x-compressed.svg application-pet.svg;do
	case $m in
		*compressed*)t=archive.svg;;
		*pet*)t=pet.svg;;
	esac
	cp -af "${SRC_DIR}/mimetypes/$m" ${TGT_DIR}/${t}
done
for a in internet.svg chat.svg paint.svg edit.svg libreoffice-calc.svg libreoffice-draw.svg libreoffice-writer.svg calendar.svg terminal.svg mail.svg x.svg network.svg module.svg screen_lock.svg multimedia.svg;do
	case $a in
		internet*)u=www.svg;;
		*calc*)u=spread.svg;;
		*draw*)u=draw.svg;;
		*writer*)u=word.svg;;
		calendar*)u=date.svg;;
		terminal*)u=console.svg;;
		mail*)u=email.svg;;
		network*)u=connect.svg;;
		module*)u=utility.svg;;
		screen_lock*)u=lock-screen.svg;;
		chat*|paint*|edit*|x*|multimedia*)u=$a;;
	esac
	cp -af "${SRC_DIR}/apps/$a" ${TGT_DIR}/${u}
done
for p in directory.svg trash_full.svg trash_empty.svg;do
	case $p in
		dir*)v=folder.svg;;
		trash_empty*)v=trashcan_empty.svg;;
		trash_full*)v=trashcan_full.svg;;
	esac
	cp -af "${SRC_DIR}/places/$p" ${TGT_DIR}/${v}
done
for d in cdrom.svg floppy.svg harddisk.svg disk_usb.svg computer.svg flashcard.svg;do
	case $d in
		cdrom*)w=optical.svg;;
		harddisk*)w=drive.svg;;
		*usb*)w=usbdrv.svg;;
		computer*)w=pc.svg;;
		flashcard*)w=card.svg;;
		*)w=$d;;
	esac
	cp -af "${SRC_DIR}/devices/$d" ${TGT_DIR}/${w}
done
for x in save.svg home.svg shutdown.svg;do
	cp -af "${SRC_DIR}/actions/$x" ${TGT_DIR}/${x}
done
for c in game.svg preferences.svg dialog-info.svg;do
	case $c in
		game*)y=games.svg;;
		preferences*)y=configuration.svg;;
		*info*)y=help.svg;;
	esac
	cp -af "${SRC_DIR}/categories/$c" ${TGT_DIR}/${y}
done		
echo "done!"
