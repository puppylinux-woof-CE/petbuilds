#!/bin/sh

if [ "`which git`" = "" ]; then
	echo "Cannot find git."
	echo "Git is required to use check-status.sh"
	exit 1
fi

REALPATH_CHECK_STATUS=$(realpath ${0%check-status.sh})
if [ "$REALPATH_CHECK_STATUS" = "$PWD" ] ; then
	if [ -d pkgs ]; then
		z0base_dir=""
	elif [ -d basic/pkgs ]; then
		cd basic
		z0base_dir=""
	else
		echo "Cannot find 'pkgs' directory."
		exit 1
	fi
else
	if [ -d "${REALPATH_CHECK_STATUS}/pkgs" ]; then
		z0base_dir="$REALPATH_CHECK_STATUS"
	elif [ -d "${REALPATH_CHECK_STATUS}/basic/pkgs" ]; then
		z0base_dir="${REALPATH_CHECK_STATUS}/basic"
	else
		echo "Cannot find 'pkgs' directory."
		exit 1
	fi
fi


case "$1" in
	-h|-help|--help) echo "./check-status.sh [ORDER-file]" ;;

	"")
		if [ -n "$z0base_dir" ]; then
			zORDER="${z0base_dir}/ORDER"
		else
			zORDER=ORDER
		fi
	;;
	*ORDER*)
		if [ -f "$1" ]; then
			zORDER="$1"
		elif [ -f "${z0base_dir}/$1" ]; then
			zORDER="${z0base_dir}/$1"
		else
			echo "File not found: $1"
			exit 1
		fi
	;;
	*)
		echo "Unknown option: $1"
		exit 1
esac

RET_VAL="0"

for PKG in `cat $zORDER`
do
	FOUND_LOGS=`find 0logs -iname "${PKG}build.log"`
	for ONE_LOG in $FOUND_LOGS
	do

		LOG_COMMIT=`grep "^building $PKG from petbuild commit" "$ONE_LOG" | cut -f 6 -d  ' '`
		#echo "$LOG_COMMIT"

		PETBUILDS_COMMIT=$(cd ${z0base_dir}/pkgs ; git log -1 $PKG | grep commit | cut -f 2 -d ' ' | cut -c -7)
		#echo "$PETBUILDS_COMMIT"

		if [ "$LOG_COMMIT" != "$PETBUILDS_COMMIT" ]; then
			RET_VAL="2"

			PET_PATH=${ONE_LOG//0logs\/}
			PET_PATH=${PET_PATH%\/*}
			#echo "$PET_PATH"

			FOUND_PETS=`find "0pets_out/$PET_PATH" -iname "${PKG}*.pet"`
			echo -e "These pets are out of date:\n${FOUND_PETS}\n"
		fi

	done
done

exit "$RET_VAL"
