#!/bin/sh

COMMON32_ISO_URL="https://github.com/woodenshoe-wi/common32/releases/download/v0.0.1-alpha/common32-0.0.1.iso"
COMMON32_DEVX_URL="https://github.com/woodenshoe-wi/common32/releases/download/v0.0.1-alpha/devx_common32_0.0.1.sfs"

COMMON64_ISO_URL="https://github.com/woodenshoe-wi/common64/releases/download/v0.0.1/common64-0.0.1.iso"
COMMON64_DEVX_URL="https://github.com/woodenshoe-wi/common64/releases/download/v0.0.1/devx_common64_0.0.1.sfs"

# autodetect HOSTARCH - how hard can it be?
case `uname -m` in
	i?86)HOSTARCH=x86 ;;
	x86_64|amd64)HOSTARCH=x86_64 ;;
	arm*)HOSTARCH=arm ;;
	*) echo "Error detecting host architecture, exiting..." ; exit 1 ;;
esac

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

if [ "$?" = "0" ]; then
  exit
else
  echo
  echo "Type exit and press <Enter> to leave run_woof"
  echo
fi
' > /tmp/petbuilds-bashrc

function download_if_missing() {
	local ISO_URL
	local ISO_NAME
	local DEVX_URL
	local DEVX_NAME
	ISO_URL=${1}
	DEVX_URL=${2}
	if [ "$ISO_URL" = "" ]; then
		echo "download_if_missing(): missing ISO_URL"
		exit 1
	elif [ "$DEVX_URL" = "" ]; then
		echo "download_if_missing(): missing DEVX_URL"
		exit 1
	fi

	mkdir -p local-repositories/petbuilds

	ISO_NAME=${ISO_URL//*\/}
	DEVX_NAME=${DEVX_URL//*\/}
	[ ! -f "local-repositories/petbuilds/${ISO_NAME}" ] && \
		wget -P local-repositories/petbuilds/ "$ISO_URL"
	[ ! -f "local-repositories/petbuilds/${DEVX_NAME}" ] && \
		wget -P local-repositories/petbuilds/ "$DEVX_URL"
}

function run_build() {
	local ISO_NAME
	local DEVX_NAME
	ISO_NAME=${1}
	DEVX_NAME=${2}
	if [ "$ISO_NAME" = "" ]; then
		echo "run_build(): missing ISO_NAME"
		exit 1
	elif [ "$DEVX_NAME" = "" ]; then
		echo "run_build(): missing DEVX_NAME"
		exit 1
	fi

	if [ ! -f "local-repositories/petbuilds/${ISO_NAME}" ]; then
		echo "run_build(): no such file: local-repositories/petbuilds/${ISO_NAME}"
		exit 1
	elif [ ! -f "local-repositories/petbuilds/${DEVX_NAME}" ]; then
		echo "run_build(): no such file: local-repositories/petbuilds/${DEVX_NAME}"
		exit 1
	fi

	run_woof/run_woof "local-repositories/petbuilds/${ISO_NAME}" \
	"local-repositories/petbuilds/${DEVX_NAME}" "${PWD}" /tmp/petbuilds-bashrc
}


# download any missing devx/ISOs
if [ "${HOSTARCH#x86}" != "${HOSTARCH}" ]; then
	[ -n "${COMMON32_ISO_URL}" -a -n "${COMMON32_DEVX_URL}" ] && \
		download_if_missing "${COMMON32_ISO_URL}" "${COMMON32_DEVX_URL}"
fi
if [ "${HOSTARCH}" = "x86_64" ]; then
	[ -n "${COMMON64_ISO_URL}" -a -n "${COMMON64_DEVX_URL}" ] && \
		download_if_missing "${COMMON64_ISO_URL}" "${COMMON64_DEVX_URL}"
fi


# download any missing sources
echo -n > /tmp/petbuilds_download_errors.txt
export DOWNLOAD_ONLY="yes"
petbuilds/basic/build_all.sh "${1}"
export DOWNLOAD_ONLY=""
if [ -s /tmp/petbuilds_download_errors.txt ]; then
	echo "Error downloading sources."
	exit 1
else
	rm /tmp/petbuilds_download_errors.txt
fi


# run the build once in each compatible distro
if [ "${HOSTARCH#x86}" != "${HOSTARCH}" ]; then
	[ -n "${COMMON32_ISO_URL}" -a -n "${COMMON32_DEVX_URL}" ] && \
		run_build "${COMMON32_ISO_URL//*\/}" "${COMMON32_DEVX_URL//*\/}"
fi
if [ "${HOSTARCH}" = "x86_64" ]; then
	[ -n "${COMMON64_ISO_URL}" -a -n "${COMMON64_DEVX_URL}" ] && \
		run_build "${COMMON64_ISO_URL//*\/}" "${COMMON64_DEVX_URL//*\/}"
fi
