sudo apt-get install build-essential libncurses-dev bison flex libssl-dev libelf-dev

sudo apt-get install gcc-arm-linux-gnueabi u-boot-tools lzop

export ARCH=arm

export CROSS_COMPILE=arm-linux-gnueabi-


## aarch64 gnu compiler 


sudo apt-get install gcc-aarch64-linux-gnu

export ARCH=aarch64

export CROSS_COMPILE=aarch64-linux-gnueabi-

make -j $(nproc)

## use qemu to test image 

sudo apt install qemu cpio qemu-system qemu-system-aarch64

qemu-system-aarch64 -M raspi3 -kernel kernel8.img

qemu-system-aarch64 -m 1024 -M raspi3 -kernel kernel8.img -append "console=ttyAMA0" -nographic -serial mon:stdio

qemu-system-aarch64 -m 1024 -M raspi3 -kernel kernel8.img -dtb bcm2710-rpi-3-b-plus.dtb -sd 2020-08-20-raspios-buster-armhf.img -append "console=ttyAMA0 root=/dev/mmcblk0p2 rw rootwait rootfstype=ext4" -nographic -device usb-net,netdev=net0 -netdev user,id=net0,hostfwd=tcp::5555-:22