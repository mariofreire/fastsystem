#!/bin/sh


nasm -f elf cstart.asm -o cstart.o
gcc -c exec.c -o exec.o -m32 -ffreestanding
ld -m elf_i386 cstart.o exec.o -o exec.elf
objcopy -O binary exec.elf exec.bin
./fput.sh harddisk.img exec.bin exec.bin
