#  Raspberry Pi Kernel Tutorial -- Qemu usage 

qemu-system-aarch64 -M raspi3 -kernel kernel8.img -nographic

qemu-system-aarch64 -M raspi3 -kernel kernel8.img  -serial mon:stdio
