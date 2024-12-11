#!/bin/sh


nasm -f elf cstart.asm -o cstart.o
gcc cstart.o chkvideo.c -o chkvideo.elf -m32 -ffreestanding -nostdlib -nostartfiles
objcopy -O binary -j .text -j .rodata -j .data -j .bss chkvideo.elf chkvideo.bin
./fput.sh harddisk.img chkvideo.bin chkvideo.bin
