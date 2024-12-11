@echo off
qemu-system-i386.exe -m 256 -drive format=raw,media=disk,index=0,file=harddisk.img -device loader,addr=0x800000,file=../kernel/vars.bin -device loader,addr=0x808000,file=../kernel/vars-info.bin -device loader,addr=0x810000,file=../kernel/enum-serial.bin -device loader,addr=0x818000,file=../kernel/enum-info-serial.bin -serial tcp:192.168.0.2:12345,server -monitor stdio -nographic
