qemu-system-i386 -m 2048 -drive format=raw,media=disk,index=0,file=harddisk.img -drive id=usb-disk0,if=none,file=hdtest.img -device pci-ohci,id=ohci -device usb-storage,drive=usb-disk0,bus=ohci.0
