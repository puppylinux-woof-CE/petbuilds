#!/bin/sh
# evince
if [ `pwd` -ne '/' ];then
	echo '#! /bin/sh
exec evince "$@"' > root/Choices/MIME-types/application_pdf
	chmod 755 root/Choices/MIME-types/application_pdf
else
	echo '#! /bin/sh
exec evince "$@"' > $HOME/Choices/MIME-types/application_pdf
	chmod 755 $HOME/Choices/MIME-types/application_pdf
fi
