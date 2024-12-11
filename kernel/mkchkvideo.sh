#!/bin/sh


gcc -c chkvideo.c -o chkvideo.o -m32 -ffreestanding
nasm -f elf cstart.asm -o cstart.o
ld -m elf_i386 -Ttext 0x1000 --entry=_start cstart.o chkvideo.o -o chkvideo.elf
objcopy -O binary chkvideo.elf chkvideo.bin
./fput.sh harddisk.img chkvideo.bin chkvideo.bin
