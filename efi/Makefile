.POSIX:
.PHONY: all clean

SOURCE = main.c
TARGET = BOOTIA32.EFI

# Uncomment for gcc, or move to OS check above, etc.
# i686-elf-gcc
CC = i686-w64-mingw32-gcc \
	-Wl,--subsystem,10 \
	-e _efi_main 

# Uncomment for clang, or move to OS check above, etc.
#CC = clang \
	-target i686-unknown-windows \
	-fuse-ld=lld-link \
	-Wl,-subsystem:efi_application \
	-Wl,-entry:efi_main

CFLAGS = \
	-std=c17 \
	-mno-red-zone \
	-ffreestanding \
	-nostdlib 

all: $(TARGET) put

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -rf $(TARGET)

run:
	qemu-system-i386 -m 512 -smbios type=0,uefi=on -bios ovmf32.bin -hda ../images/efi.img

put:
	./fput.sh ../images/efi.img BOOTIA32.EFI EFI/BOOT/BOOTIA32.EFI
	./fput.sh ../images/efi.img BOOTX64.EFI EFI/BOOT/BOOTX64.EFI

