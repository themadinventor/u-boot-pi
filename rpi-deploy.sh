cat rpi-gpu/first32k.bin u-boot.bin > kernel.img

echo "Deploying to" $1

/sbin/mkfs.vfat -F 32 -n boot /dev/$1
mount /dev/$1 /mnt

cp rpi-gpu/bootcode.bin rpi-gpu/start.elf /mnt/
cp kernel.img /mnt/kernel.img

umount /mnt
sync
