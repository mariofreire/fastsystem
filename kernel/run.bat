@echo off
rem piso edit harddisk.img -y -add fskrnl /
qemu-system-i386 -m 2048 -drive format=raw,media=disk,index=0,file=harddisk.img
