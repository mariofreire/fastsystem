#!/bin/sh

gcc -m32 -g -nostdlib -nostartfiles -ffreestanding -fno-builtin -Wl,--whole-archive -Wall -Wextra -I. -L. -lc $2 -o $1.elf $1.c
objcopy -O binary -j .text -j .rodata -j .data -j .bss $1.elf $1.bin
./fput.sh harddisk.img $1.bin $1.bin
