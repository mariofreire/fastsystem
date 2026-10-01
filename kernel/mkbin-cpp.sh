#!/bin/sh

g++ -m32 -g -nostdlib -nostartfiles -ffreestanding -fno-builtin -fno-sized-deallocation -Wl,--whole-archive -Wall -Wextra -I../libc -L../libc -lc $2 -o $1.elf $1.cpp
objcopy -O binary -j .text -j .rodata -j .data -j .bss $1.elf $1.bin
./fput.sh ../images/harddisk.img $1.bin $1.bin
