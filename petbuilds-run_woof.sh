#!/bin/sh

# For some reason there has to be a blank line between entries,
# otherwise every other one gets skipped.
DISTROS='
http://distro.ibiblio.org/puppylinux/puppy-slacko-6.3.2/32/slacko-6.3.2-uefi.iso|http://distro.ibiblio.org/puppylinux/puppy-slacko-6.3.2/32/devx_slacko_6.3.2.sfs

http://distro.ibiblio.org/puppylinux/puppy-slacko-6.3.2/64/slacko64-6.3.2-uefi.iso|http://distro.ibiblio.org/puppylinux/puppy-slacko-6.3.2/64/devx_slacko64_6.3.2.sfs

http://distro.ibiblio.org/puppylinux/puppy-tahr/iso/tahrpup -6.0-CE/tahr-6.0.6-uefi.iso|http://distro.ibiblio.org/puppylinux/puppy-tahr/iso/tahrpup -6.0-CE/devx_tahr_6.0.6.sfs

http://distro.ibiblio.org/puppylinux/puppy-tahr/iso/tahrpup -6.0-CE/tahr64-6.0.6-uefi.iso|http://distro.ibiblio.org/puppylinux/puppy-tahr/iso/tahrpup -6.0-CE/devx_tahr64_6.0.6.sfs
'


[ -d ../run_woof ] && cd ..
if [ ! -d run_woof ]; then
	echo "Cannot find run_woof."
	exit 1
fi

# /tmp/petbuilds-bashrc is used in place of default run_woof bashrc
echo '#!/bin/sh

. /etc/profile

PS1="run_woof\\$ "

cd /root/share

petbuilds/basic/build_all.sh '"$1"'

[ "$?" = "0" ] && exit
' > /tmp/petbuilds-bashrc

# download any missing devx/ISOs
while read -r ONE_DISTRO
do
	[ "$ONE_DISTRO" = "" ] && continue
	IFS="|" read -r ISO_URL DEVX_URL <<< "$ONE_DISTRO"
	ISO_NAME=${ISO_URL//*\/}
	DEVX_NAME=${DEVX_URL//*\/}
	[ ! -f "$ISO_NAME" ] && wget "$ISO_URL"
	[ ! -f "$DEVX_NAME" ] && wget "$DEVX_URL"
done <<< "${DISTROS}"

# run the build once in each distro
while read ONE_DISTRO
do
	[ "$ONE_DISTRO" = "" ] && continue
	IFS="|" read -r ISO_URL DEVX_URL <<< "$ONE_DISTRO"
	ISO_NAME=${ISO_URL//*\/}
	DEVX_NAME=${DEVX_URL//*\/}

	run_woof/run_woof $ISO_NAME $DEVX_NAME $PWD /tmp/petbuilds-bashrc

done <<< "${DISTROS}"
