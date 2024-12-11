# $@ = target file
# $< = first dependency
# $^ = all dependencies

# First rule is the one executed when no parameters are fed to the Makefile
all: run

# Notice how dependencies are built as needed
bootload.elf: loadmbr.o functions.o
	ld -m elf_i386 -o $@ -T link.ld $^
	
mbr.bin: bootload.elf
	objcopy -O binary $< $@

functions.o: functions.asm
	nasm $< -f elf -o $@

loadmbr.o: loadmbr.c
	gcc -g -O2 -O3 -Os -ffreestanding -fno-pie -fno-exceptions -static -nostdlib -nostartfiles -m16 -c $< -o $@

run: mbr.bin
	./writembr harddisk.img mbr.bin

clean:
	rm bootload.elf functions.o loadmbr.o mbr.bin

