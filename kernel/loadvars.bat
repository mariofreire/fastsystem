@echo off
qemu-system-i386 -m 256 -drive format=raw,media=disk,index=0,file=harddisk.img -device loader,addr=0x800000,file=vars.bin -device loader,addr=0x808000,file=vars-info.bin -device loader,addr=0x810000,file=enum.bin -device loader,addr=0x818000,file=enum-info.bin
