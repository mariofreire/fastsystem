# $@ = target file
# $< = first dependency
# $^ = all dependencies

# First rule is the one executed when no parameters are fed to the Makefile
all: writembr mbr-c.bin mbr.bin pmbr.bin run

# Notice how dependencies are built as needed
bootload.elf: loadmbr.o functions.o
	ld -m elf_i386 -o $@ -T link.ld $^
	
mbr-c.bin: bootload.elf
	objcopy -O binary $< $@

functions.o: functions.asm
	nasm $< -f elf -o $@

loadmbr.o: loadmbr.c
	gcc -g -O2 -O3 -Os -ffreestanding -fno-pie -fno-exceptions -static -nostdlib -nostartfiles -m16 -c $< -o $@

writembr.o: writembr.c
	gcc -c $< -o $@

writembr: writembr.o
	gcc $< -o $@

mbr.bin: mbr.asm
	nasm $< -f bin -o $@

pmbr.bin: pmbr.asm
	nasm $< -f bin -o $@

run: mbr.bin
	./writembr ../images/harddisk.img mbr.bin
	./writembr ../images/efi.img pmbr.bin

clean:
	rm -f *.o *.elf *.bin
	rm -f writembr

