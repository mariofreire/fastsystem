#!/bin/sh

nasm -f elf module_start.asm -o module_start.o
nasm -f elf loader_module_functions.asm -o loader_module_functions.o
gcc -g -Os -ffreestanding -fno-pie -fno-exceptions -s -static -fno-builtin -nostdlib -nostartfiles -m32 -c loader_module.c -o loader_module.o
ld -m elf_i386 -o loader_module.elf -Ttext=0x1000 module_start.o loader_module_functions.o loader_module.o
objcopy -O binary loader_module.elf loader_module

