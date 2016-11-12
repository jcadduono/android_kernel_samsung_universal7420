#!/bin/bash
# TWRP kernel for Samsung Galaxy Note 5 / S6 / S6 Edge / S6 Edge+ build script by jcadduono
# This build script builds all variants in ./VARIANTS
#
###################### CONFIG ######################

# root directory of universal7420 git repo (default is this script's location)
RDIR=$(pwd)

# output directory of device
OUT_DIR=$HOME/build/twrp/device/samsung

############## SCARY NO-TOUCHY STUFF ###############

ARCH=arm64
KDIR=$RDIR/build/arch/$ARCH/boot

[ "$1" ] && BUILD_DEVICE=$1

while read -r line; do
	set -- $line
	DEVICE=$1
	shift
	if [ "$BUILD_DEVICE" ] && [ ! "$BUILD_DEVICE" = "$DEVICE" ]; then
		continue
	fi
	while [ $# -gt 0 ]; do
		"$RDIR/build.sh" "$DEVICE" "$1" || exit 1
		MOVE_IMAGES "$DEVICE" "$1"
		DDIR="$OUT_DIR/$DEVICE${1%eur}"
		echo "Moving kernel Image and dtb.img to $DDIR/..."
		mkdir -p "$DDIR" || exit 1
		mv "$KDIR/Image" "$KDIR/dtb.img" "$DDIR/"
	done
done < "$RDIR/VARIANTS"

echo "Finished building TWRP kernels!"
