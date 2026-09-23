# $@ = target file
# $< = first dependency
# $^ = all dependencies

# First rule is the one executed when no parameters are fed to the Makefile
all: loader boot put # run (optional)

# Notice how dependencies are built as needed
loader.elf: loader_start.o loader.o functions.o pci.o ahci.o ehci.o arith64.o
	ld -m elf_i386 -o $@ -T link.ld $^  --no-warn-rwx-segments
loader: loader.elf
	objcopy -O binary $< $@
	objcopy --only-keep-debug $< $@.debug
	objcopy --extract-symbol $< $@.sym

loader_start.o: loader_start.asm
	nasm $< -f elf -o $@

functions.o: functions.asm
	nasm $< -f elf -o $@

loader.o: loader.c
	gcc -g -Os -ffreestanding -fno-pie -fno-exceptions -s -static -fno-builtin -fno-asynchronous-unwind-tables -nostdlib -nostartfiles -m32 -I../kernel -c $< -o $@

pci.o: pci.c
	gcc -g -Os -ffreestanding -fno-pie -fno-exceptions -s -static -fno-builtin -fno-asynchronous-unwind-tables -nostdlib -nostartfiles -m32 -I../kernel -c $< -o $@

ahci.o: ahci.c
	gcc -g -Os -ffreestanding -fno-pie -fno-exceptions -s -static -fno-builtin -fno-asynchronous-unwind-tables -nostdlib -nostartfiles -m32 -I../kernel -c $< -o $@

ehci.o: ehci.c
	gcc -g -Os -ffreestanding -fno-pie -fno-exceptions -s -static -fno-builtin -fno-asynchronous-unwind-tables -nostdlib -nostartfiles -m32 -I../kernel -c $< -o $@

arith64.o: arith64.c
	gcc -g -Os -ffreestanding -fno-pie -fno-exceptions -s -static -fno-builtin -fno-asynchronous-unwind-tables -nostdlib -nostartfiles -m32 -c $< -o $@

clean:
	rm -f loader.sym loader.debug loader.elf loader_start.o functions.o loader.o loader
	rm -f *.o *.a

boot:
	../boot/writeboot ../images/harddisk.img ../boot/boot.bin

put:
	../fat/fput ../images/harddisk.img ../loader/loader loader
	../fat/fput ../images/grub.img ../loader/loader loader

run:
	qemu-system-i386 -m 512 -drive format=raw,media=disk,index=0,file=../images/harddisk.img

loadvars:
	qemu-system-i386 -m 512 -drive format=raw,media=disk,index=0,file=../images/harddisk.img \
	 -device loader,addr=0x800000,file=../kernel/vars.bin -device loader,addr=0x808000,file=../kernel/vars-info.bin \
	 -device loader,addr=0x810000,file=../kernel/enum.bin -device loader,addr=0x818000,file=../kernel/enum-info.bin

debug:
	gdb ../loader/loader.debug --init-command=../loader/initdebug.gdbc

run_debug:
	qemu-system-i386 -m 512 -drive format=raw,media=disk,index=0,file=../images/harddisk.img -gdb tcp:192.168.0.2:1234 -S \
	 -device loader,addr=0x800000,file=../kernel/vars.bin -device loader,addr=0x808000,file=../kernel/vars-info.bin \
	 -device loader,addr=0x810000,file=../kernel/enum.bin -device loader,addr=0x818000,file=../kernel/enum-info.bin

norun: loader boot put
