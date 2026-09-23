#!/bin/sh

gcc -m32 -g -nostdlib -nostartfiles -ffreestanding -fno-builtin -Wl,--whole-archive -Wall -Wextra -I. $2 -c $1.c -o $1.o
ld -T bin.lds -L. $1.o -lc $3 -o $1.elf
objcopy -O binary $1.elf $1.bin
./fput.sh harddisk.img $1.bin $1.bin
