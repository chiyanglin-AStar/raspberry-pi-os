## 01 add uart0 to kernel code

###  major reference
https://github.com/s-matyukevich/raspberry-pi-os.git
lesson1

https://github.com/bztsrc/raspi3-tutorial
01_bareminimum 、 02_multicorec 、03_uart1

### Qemu verification 

qemu-system-aarch64 -M raspi3 -kernel kernel8.img -nographic

qemu-system-aarch64 -M raspi3 -kernel kernel8.img -nographic -serial file:uart0 -serial file:uart1

qemu-system-aarch64 -m 128 -M raspi3 -serial null -serial mon:stdio -nographic -kernel build/kernel8.elf