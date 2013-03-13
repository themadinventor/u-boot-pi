#!/bin/bash

if [ -z "$1" ]
then
    echo "Usage: $0 path_to_block_device_partition (e.g. /dev/sdb1)"
    exit 1
fi

if [ `whoami` != "root" ]
then
    echo "ERROR: This script must be run as root as it will reformat and temporarily mount your SD card."
    echo
    echo "Please run it using:"
    echo "  sudo $0 $1"
    exit 1
fi

echo "Deploying to" $1
echo
echo "WARNING: This partition will be reformatted. All current data will be lost!"
echo "Please make sure that $1 is not your system partition... ;)"
echo

read -p "Press [Enter] to continue or Ctrl-C to abort"
echo

/sbin/mkfs.vfat -F 32 -n boot $1

MNTPATH=/tmp/__rpiboot

mkdir -p $MNTPATH
mount $1 $MNTPATH

cp rpi-gpu/bootcode.bin rpi-gpu/start.elf $MNTPATH
cat rpi-gpu/first32k.bin u-boot.bin > /tmp/__kernel.img
mv /tmp/__kernel.img $MNTPATH/kernel.img

sleep 1

umount $MNTPATH
rmdir --ignore-fail-on-non-empty $MNTPATH

sync
