#  use formal rpi3 image to check build and test flow and environment

wget https://downloads.raspberrypi.org/raspbian/images/raspbian-2018-11-15/2018-11-13-raspbian-stretch.zip

## for qemu rpi3 image 

curl https://raw.githubusercontent.com/dhruvvyas90/qemu-rpi-kernel/master/kernel-qemu-4.4.34-jessie --output kernel-qemu-4.4.34-jessie

sudo qemu-system-arm -kernel ./kernel-qemu-4.4.34-jessie -append "root=/dev/sda2 panic=1 rootfstype=ext4 rw" -hda 2018-11-13-raspbian-stretch.img -cpu arm1176  -m 256 -M versatilepb -nographic -no-reboot -serial stdio -net nic -net user -net tap,ifname=vnet0,script=no,downscript=no

##  the package that could need 
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

qemu-system-aarch64 -M raspi3 -kernel kernel8.img -nographic 

qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial null -serial stdio ( need to run on ubuntu desktop )

#### -----------------------------------
qemu-system-aarch64 -M raspi3 -kernel kernel8.img -nographic -serial pty -serial pty

It works fine this time. I got following output:
vvfat ./data chs 1024,16,63
QEMU 2.2.0 monitor - type 'help' for more information
(qemu) char device redirected to /dev/pts/87 (label serial0)
char device redirected to /dev/pts/88 (label serial1)

Then I do `cat /dev/pts/86` I see the kernel boot progress messages.
When the guest has booted, I ssh to guest and do following experiment:
(1)  on host:
#cat /dev/pts88

on guest:
#echo "alex" > /dev/ttyAMA1

I see "alex" on host.

(2) on guest:
#cat /dev/ttyAMA1

on host:
#echo "alex" > /dev/pts88

#### --------------------------------

qemu-system-aarch64 -M raspi3 -kernel kernel8.img -nographic -serial null -monitor stdio 

#### --------------------------------

qemu-system-aarch64 -M raspi3 -kernel kernel8.img -nographic -serial telnet:localhost:4321,server,nowait

qemu-system-aarch64 -M raspi3b -kernel kernel8.img -nographic  -serial stdio -append "console=ttyAMA0"


qemu-system-aarch64 -m 1024 -M raspi3 -kernel kernel8.img -append "console=ttyAMA0" -nographic -serial mon:stdio


## Raspberry 3 Kernel image and use qemu to test image
ref :  https://gist.github.com/G-UK/196638656e822a2b67f7843db875646d
       https://www.nayab.xyz/rpi3b-elinux/embedded-linux-rpi3-050-linux

git clone http://github.com/raspberrypi/linux.git --depth 1

cd linux

export ARCH=arm64

export CROSS_COMPILE=aarch64-linux-gnu-

make bcmrpi3_defconfig

make -j$(nproc)


cp arch/arm64/boot/Image ./kernel8.img

cp arch/arm64/boot/dts/overlays/*.dtbo ../boot/overlays/

cp arch/arm64/boot/dts/broadcom/*.dtb .

cd ..

### Qemu rpi3 usage 
qemu-system-aarch64 -m 1024 -M raspi3 -kernel ./kernel8.img -dtb ./bcm2710-rpi-3-b-plus.dtb -drive "file=./2016-11-25-raspbian-jessie.img,if=none,index=0,media=disk,format=raw,id=disk0" -append "console=ttyAMA0" -nographic 

qemu-system-aarch64 -m 1024 -M raspi3 -kernel kernel8.img -dtb bcm2710-rpi-3-b-plus.dtb -sd 2016-11-25-raspbian-jessie.img -append "console=ttyAMA0" -nographic 

ref : https://stackoverflow.com/questions/60552355/qemu-baremetal-emulation-how-to-view-uart-output 

      https://fadeevab.com/how-to-setup-qemu-output-to-console-and-automate-using-shell-script/

qemu-system-aarch64 -semihosting --semihosting-config enable=on,target=native -nographic -serial mon:stdio -M raspi3 -m 1024  -kernel kernel8.img 

qemu-system-aarch64 -M raspi3 -m 1024  -nographic -kernel kernel8.img  -drive "file=./2016-11-25-raspbian-jessie.img,if=none,index=0,media=disk,format=raw,id=disk0" -append "root=/dev/sda console=ttyS0"

qemu-system-aarch64 -M raspi3 -m 1024  -nographic -kernel kernel8.img  -drive "file=./2016-11-25-raspbian-jessie.img,if=none,index=0,media=disk,format=raw,id=disk0" -append "root=/dev/sda console=ttyS0" -serial pipe:./guest

## ref :  Build rpi linux kernel from linux source code 

sudo apt install gcc build-essential automake gcc-arm-linux-gnueabihf

https://downloads.raspberrypi.org/raspbian/images/raspbian-2016-11-29/2016-11-25-raspbian-jessie.zip

git clone http://github.com/raspberrypi/linux.git --branch raspberrypi-kernel_1.20180619-1 --single-branch --depth 1  

PS:  this kernel version is 4.14 

export ARCH=arm

export CROSS_COMPILE=arm-linux-gnueab

patch -p1 < ./linux-arm.patch (from : https://github.com/dhruvvyas90/qemu-rpi-kernel/tree/master/tools)

KERNEL=kernel7

make versatile_defconfig

make -j $(nproc)

cp arch/arm/boot/zImage kernel7.img