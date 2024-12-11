#!/bin/sh


gcc -c test1.c -o test1.o -m32 -ffreestanding
nasm -f elf cstart.asm -o cstart.o
ld -m elf_i386 -Ttext 0x1000 --entry=_start cstart.o test1.o -o test1.elf
objcopy -O binary test1.elf test1.bin
./fput.sh harddisk.img test1.bin test1.bin
