# $@ = target file
# $< = first dependency
# $^ = all dependencies

# First rule is the one executed when no parameters are fed to the Makefile
all: libc.a fskrnl gen-vars write-vars boot put run

# Notice how dependencies are built as needed
fskrnl.elf: kernel_start.o fskrnl.o memory.o functions.o arith64.o
	ld -m elf_i386 -o $@ -T link.ld $^
	
fskrnl: fskrnl.elf
	objcopy -O binary $< $@
	objcopy --only-keep-debug $< $@.debug
	objcopy --extract-symbol $< $@.sym

kernel_start.o: kernel_start.asm
	nasm $< -f elf -o $@

functions.o: functions.asm
	nasm $< -f elf -o $@

memory.o: memory.c
	gcc -g -Os -ffreestanding -fno-pie -fno-exceptions -s -static -fno-builtin -nostdlib -nostartfiles -m32 -c $< -o $@

fskrnl.o: fskrnl.c
	gcc -g -Os -ffreestanding -fno-pie -fno-exceptions -s -static -fno-builtin -nostdlib -nostartfiles -m32 -c $< -o $@

arith64.o: arith64.c
	gcc -g -Os -ffreestanding -fno-pie -fno-exceptions -s -static -fno-builtin -nostdlib -nostartfiles -m32 -c $< -o $@

crt0.o: crt0.c
	gcc -m32 -nostdlib -nostartfiles -s -static -ffreestanding -fno-omit-frame-pointer -fno-builtin -fno-exceptions -fno-leading-underscore -fno-pie -fPIC -r -c $< -o $@

io.o: io.c
	gcc -m32 -nostdlib -nostartfiles -s -static -ffreestanding -fno-omit-frame-pointer -fno-builtin -fno-exceptions -fno-leading-underscore -fno-pie -fPIC -I. -r -c $< -o $@

conio.o: conio.c
	gcc -m32 -nostdlib -nostartfiles -s -static -ffreestanding -fno-omit-frame-pointer -fno-builtin -fno-exceptions -fno-leading-underscore -fno-pie -fPIC -r -c $< -o $@

stdio.o: stdio.c
	gcc -m32 -nostdlib -nostartfiles -s -static -ffreestanding -fno-omit-frame-pointer -fno-builtin -fno-exceptions -fno-leading-underscore -fno-pie -fPIC -r -c $< -o $@

stdlib.o: stdlib.c
	gcc -m32 -nostdlib -nostartfiles -s -static -ffreestanding -fno-omit-frame-pointer -fno-builtin -fno-exceptions -fno-leading-underscore -fno-pie -fPIC -r -c $< -o $@

string.o: string.c
	gcc -m32 -nostdlib -nostartfiles -s -static -ffreestanding -fno-omit-frame-pointer -fno-builtin -fno-exceptions -fno-leading-underscore -fno-pie -fPIC -r -c $< -o $@

errno.o: errno.c
	gcc -m32 -nostdlib -nostartfiles -s -static -ffreestanding -fno-omit-frame-pointer -fno-builtin -fno-exceptions -fno-leading-underscore -fno-pie -fPIC -r -c $< -o $@

libc.a: crt0.o io.o conio.o stdio.o stdlib.o string.o errno.o arith64.o
	ar rcs $@ $^
	
testc.elf: testc.c
	gcc -m32 -nostdlib -nostartfiles -ffreestanding -fno-omit-frame-pointer -fno-builtin -fno-exceptions -fno-leading-underscore -Wl,--whole-archive -Wall -Wextra -L. -lc -o $@ $^

testc.bin: testc.elf
	objcopy -O binary -j .text -j .rodata -j .data -j .bss $< $@
	./fput.sh harddisk.img $@ $@

pci.o: pci.c
	gcc -m32 -nostdlib -nostartfiles -s -static -ffreestanding -fno-omit-frame-pointer -fno-builtin -fno-exceptions -fno-leading-underscore -fno-pie -fPIC -r -c $< -o $@

libpci.a: pci.o
	ar rcs $@ $^
	
lspci.elf: lspci.c
	gcc -m32 -g -nostdlib -nostartfiles -ffreestanding -fno-builtin -Wl,--whole-archive -Wall -Wextra -L. -lc -lpci -o $@ $^

lspci.bin: lspci.elf
	objcopy -O binary -j .text -j .rodata -j .data -j .bss $< $@
	./fput.sh harddisk.img $@ $@

ahci-hba.elf: ahci-hba.c
	gcc -m32 -g -nostdlib -nostartfiles -ffreestanding -fno-builtin -Wl,--whole-archive -Wall -Wextra -L. -lc -lpci -o $@ $^

ahci-hba.bin: ahci-hba.elf
	objcopy -O binary -j .text -j .rodata -j .data -j .bss $< $@
	./fput.sh harddisk.img $@ $@

ahci-01.elf: ahci-01.c
	gcc -m32 -g -nostdlib -nostartfiles -ffreestanding -fno-builtin -Wl,--whole-archive -Wall -Wextra -L. -lc -lpci -o $@ $^

ahci-01.bin: ahci-01.elf
	objcopy -O binary -j .text -j .rodata -j .data -j .bss $< $@
	./fput.sh harddisk.img $@ $@

ahci-02.elf: ahci-02.c
	gcc -m32 -g -nostdlib -nostartfiles -ffreestanding -fno-builtin -Wl,--whole-archive -Wall -Wextra -L. -lc -lpci -o $@ $^

ahci-02.bin: ahci-02.elf
	objcopy -O binary -j .text -j .rodata -j .data -j .bss $< $@
	./fput.sh harddisk.img $@ $@

ahci-03.elf: ahci-03.c
	gcc -m32 -g -nostdlib -nostartfiles -ffreestanding -fno-builtin -Wl,--whole-archive -Wall -Wextra -L. -lc -lpci -o $@ $^

ahci-03.bin: ahci-03.elf
	objcopy -O binary -j .text -j .rodata -j .data -j .bss $< $@
	./fput.sh harddisk.img $@ $@

gen-vars.o: gen-vars.c
	gcc -m32 -c $< -o $@

gen-vars: gen-vars.o
	gcc -m32 -o $@ $^

write-vars: 
	./gen-vars

clean:
	rm -f fskrnl.sym fskrnl.debug fskrnl.elf kernel_start.o functions.o fskrnl.o fskrnl loader gen-vars gen-vars.o vars.bin vars-info.bin enum.bin enum-info.bin
	rm -f *.o *.a

boot:	
	../boot/writeboot harddisk.img ../boot/boot.bin

put:
	./fput.sh harddisk.img ../kernel/fskrnl fskrnl
	./fput.sh harddisk.img ../loader/loader loader
	./fput.sh grub.img ../kernel/fskrnl fskrnl
	./fput.sh grub.img ../loader/loader loader
	./fput.sh grub.img ../kernel/fskrnl.elf boot/fskrnl.elf

run:
	qemu-system-i386 -m 2048 -drive format=raw,media=disk,index=0,file=harddisk.img

loadvars:
	qemu-system-i386 -m 2048 -drive format=raw,media=disk,index=0,file=harddisk.img \
	 -device loader,addr=0x800000,file=../kernel/vars.bin -device loader,addr=0x808000,file=../kernel/vars-info.bin \
	 -device loader,addr=0x810000,file=../kernel/enum.bin -device loader,addr=0x818000,file=../kernel/enum-info.bin

debug:
	gdb  ../kernel/fskrnl.debug --init-command=../loader/initdebug.gdbc

run_debug:
	qemu-system-i386 -m 2048 -drive format=raw,media=disk,index=0,file=harddisk.img -gdb tcp::1234 -S \
	 -device loader,addr=0x800000,file=../kernel/vars.bin -device loader,addr=0x808000,file=../kernel/vars-info.bin \
	 -device loader,addr=0x810000,file=../kernel/enum.bin -device loader,addr=0x818000,file=../kernel/enum-info.bin

run_ahci_ich9:
	qemu-system-i386 -m 2048 -drive id=disk,if=none,file=harddisk.img -device ahci,id=ahci -device ide-hd,drive=disk,bus=ahci.0

run_ehci_usb:	
	qemu-system-i386 -m 2048 -drive id=disk,if=none,file=harddisk.img -device usb-ehci,id=ehci -device usb-storage,drive=disk,bus=ehci.0

norun: fskrnl gen-vars write-vars boot put

run_logging:
	qemu-system-i386 -m 2048 -drive format=raw,media=disk,index=0,file=harddisk.img -d int -no-shutdown -no-reboot

noreboot:
	qemu-system-i386 -m 2048 -drive format=raw,media=disk,index=0,file=harddisk.img -no-shutdown -no-reboot

noshutdown:
	qemu-system-i386 -m 2048 -drive format=raw,media=disk,index=0,file=harddisk.img -no-shutdown -no-reboot

run_kernel:
	qemu-system-i386 -m 2048 -drive format=raw,media=disk,index=0,file=harddisk.img -kernel fskrnl.elf -append "INIT=/init ROOT=/dev/hd1 MOUNT_POINT=/"

run_splash:
	qemu-system-i386 -m 2048 -drive format=raw,media=disk,index=0,file=harddisk.img -boot splash=bootsplash.jpg,menu=on

run_grub:
	qemu-system-i386 -m 2048 -drive format=raw,media=disk,index=0,file=grub.img

run_serial:
	qemu-system-i386 -m 2048 -drive format=raw,media=disk,index=0,file=harddisk.img -device loader,addr=0x800000,file=../kernel/vars.bin -device loader,addr=0x808000,file=../kernel/vars-info.bin -device loader,addr=0x810000,file=../kernel/enum-serial.bin -device loader,addr=0x818000,file=../kernel/enum-info-serial.bin -serial tcp::12345,server -monitor stdio -nographic

run_kernel_verbose:
	qemu-system-i386 -m 2048 -drive format=raw,media=disk,index=0,file=harddisk.img -kernel fskrnl.elf -append "INIT=/init ROOT=/dev/hd1 MOUNT_POINT=/" -device loader,addr=0x810000,file=../kernel/enum-verbose.bin -device loader,addr=0x818000,file=../kernel/enum-info-verbose.bin

