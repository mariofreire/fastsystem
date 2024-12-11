@echo off
rem piso edit harddisk.img -y -add loader /
qemu-system-i386 -m 256 -hda harddisk.img
