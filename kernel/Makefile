# $@ = target file
# $< = first dependency
# $^ = all dependencies

NASM        ?= nasm
CC          ?= gcc
AR          ?= ar
LD          ?= ld
GDB         ?= gdb
OBJCOPY     ?= objcopy
QEMU        ?= qemu-system-i386

CFLAGS := \
	-m32 \
	-ffreestanding \
	-fno-pie \
	-fno-pic \
	-fno-builtin \
	-fno-stack-protector \
	-fno-strict-aliasing \
	-fno-asynchronous-unwind-tables \
	-fno-unwind-tables \
	-fno-exceptions \
	-mno-sse \
	-mno-mmx \
	-mpreferred-stack-boundary=2 \
	-nostdlib \
	-nostartfiles \
	-Os \
	-g

ARFLAGS := rcs

NASMFLAGS := -f elf

LDFLAGS := -m elf_i386 -T link.ld -nostdlib

QEMUFLAGS := -m 2048

# First rule is the one executed when no parameters are fed to the Makefile
all: fskrnl gen-vars write-vars boot put testc.bin lspci.bin getargs.bin

# Notice how dependencies are built as needed
fskrnl.elf: kernel_start.o fskrnl.o memory.o string.o io.o pcidev.o storage.o ata.o ahci.o ehci.o nvme.o fat.o time.o vfs.o fatfs.o module.o functions.o arith64.o
	ld $(LDFLAGS) -o $@ $^

fskrnl: fskrnl.elf
	$(OBJCOPY) -O binary $< $@
	$(OBJCOPY) --only-keep-debug $< $@.debug
	$(OBJCOPY) --extract-symbol $< $@.sym

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	$(NASM) $< $(NASMFLAGS) -o $@

testc.elf: testc.c
	$(CC) -m32 -nostdlib -nostartfiles -ffreestanding -fno-omit-frame-pointer -fno-builtin -fno-exceptions -fno-leading-underscore -Wl,--whole-archive -Wall -Wextra -L../libc -lc -o $@ $^

testc.bin: testc.elf
	objcopy -O binary -j .text -j .rodata -j .data -j .bss $< $@
	../fat/fput ../images/harddisk.img $@ $@

lspci.elf: lspci.c
	$(CC) -m32 -g -nostdlib -nostartfiles -ffreestanding -fno-builtin -Wl,--whole-archive -Wall -Wextra -L../libc -L../libpci -I../libc -I../libpci -lc -lpci -o $@ $^

lspci.bin: lspci.elf
	objcopy -O binary -j .text -j .rodata -j .data -j .bss $< $@
	../fat/fput ../images/harddisk.img $@ $@

getargs.elf: getargs.c
	$(CC) -m32 -g -nostdlib -nostartfiles -ffreestanding -fno-builtin -Wl,--whole-archive -Wall -Wextra -L../libc -L../libpci -I../libc -I../libpci -lc -o $@ $^

getargs.bin: getargs.elf
	objcopy -O binary -j .text -j .rodata -j .data -j .bss $< $@
	../fat/fput ../images/harddisk.img $@ $@

gen-vars.o: gen-vars.c
	$(CC) -m32 -c $< -o $@

gen-vars: gen-vars.o
	$(CC) -m32 -o $@ $^

write-vars:
	./gen-vars enum-verbose vars
	./gen-vars enum-serial vars
	./gen-vars enum vars

modules: mod/hello.o

mod/hello.o: mod/hello.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f fskrnl loader gen-vars
	rm -f *.o *.a *.bin *.elf *.sym *.debug

boot:
	../boot/writeboot ../images/harddisk.img ../boot/boot.bin

put:
	../fat/fput ../images/harddisk.img ../kernel/fskrnl fskrnl
	../fat/fput ../images/grub.img ../kernel/fskrnl fskrnl
	../fat/fput ../images/grub.img ../kernel/fskrnl.elf boot/fskrnl.elf

run:
	$(QEMU) $(QEMUFLAGS) -drive format=raw,media=disk,index=0,file=../images/harddisk.img

loadvars:
	$(QEMU) $(QEMUFLAGS) -drive format=raw,media=disk,index=0,file=../images/harddisk.img \
	 -device loader,addr=0x800000,file=../kernel/vars.bin -device loader,addr=0x808000,file=../kernel/vars-info.bin \
	 -device loader,addr=0x810000,file=../kernel/enum.bin -device loader,addr=0x818000,file=../kernel/enum-info.bin

debug:
	$(GDB)  ../kernel/fskrnl.debug --init-command=../loader/initdebug.gdbc

run_debug:
	$(QEMU) $(QEMUFLAGS) -drive format=raw,media=disk,index=0,file=../images/harddisk.img -gdb tcp::1234 -S \
	 -device loader,addr=0x800000,file=../kernel/vars.bin -device loader,addr=0x808000,file=../kernel/vars-info.bin \
	 -device loader,addr=0x810000,file=../kernel/enum.bin -device loader,addr=0x818000,file=../kernel/enum-info.bin

run_ahci_ich9:
	$(QEMU) $(QEMUFLAGS) -drive id=disk,if=none,format=raw,file=../images/harddisk.img -device ich9-ahci,id=ahci0 -device ide-hd,drive=disk,bus=ahci0.1

run_ehci_usb:
	$(QEMU) $(QEMUFLAGS) -drive id=disk,if=none,format=raw,file=../images/harddisk.img -device usb-ehci,id=ehci -device usb-storage,drive=disk,bus=ehci.0

norun: fskrnl gen-vars write-vars boot put

run_logging:
	$(QEMU) $(QEMUFLAGS) -drive format=raw,media=disk,index=0,file=../images/harddisk.img -d int -no-shutdown -no-reboot

noreboot:
	$(QEMU) $(QEMUFLAGS) -drive format=raw,media=disk,index=0,file=../images/harddisk.img -no-shutdown -no-reboot

noshutdown:
	$(QEMU) $(QEMUFLAGS) -drive format=raw,media=disk,index=0,file=../images/harddisk.img -no-shutdown -no-reboot

run_kernel:
	$(QEMU) $(QEMUFLAGS) -drive format=raw,media=disk,index=0,file=../images/harddisk.img -kernel fskrnl.elf -append "INIT=/init ROOT=/dev/hd1 MOUNT_POINT=/"

run_splash:
	$(QEMU) $(QEMUFLAGS) -drive format=raw,media=disk,index=0,file=../images/harddisk.img -boot splash=bootsplash.jpg,menu=on

run_grub:
	$(QEMU) $(QEMUFLAGS) -drive format=raw,media=disk,index=0,file=../images/grub.img

run_serial:
	$(QEMU) $(QEMUFLAGS) -drive format=raw,media=disk,index=0,file=../images/harddisk.img -device loader,addr=0x800000,file=../kernel/vars.bin -device loader,addr=0x808000,file=../kernel/vars-info.bin -device loader,addr=0x810000,file=../kernel/enum-serial.bin -device loader,addr=0x818000,file=../kernel/enum-serial-info.bin -serial tcp::12345,server -monitor stdio -nographic

run_kernel_verbose:
	$(QEMU) $(QEMUFLAGS) -drive format=raw,media=disk,index=0,file=../images/harddisk.img -kernel fskrnl.elf -append "INIT=/init ROOT=/dev/hd1 MOUNT_POINT=/" -device loader,addr=0x810000,file=../kernel/enum-verbose.bin -device loader,addr=0x818000,file=../kernel/enum-verbose-info.bin
