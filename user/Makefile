CC      = gcc
LD      = ld
OBJCOPY = objcopy

CFLAGS  = -m32 -g -nostdlib -nostartfiles -ffreestanding -fno-builtin -Wl,--whole-archive -Wall -Wextra -I../libc
LDFLAGS = -T ../libc/bin.lds -L../libc -lc

SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)

ELFS    = $(SOURCES:.c=.elf)
BINS    = $(SOURCES:.c=.bin)

all: $(ELFS) $(BINS)

%.elf: %.o
	$(LD) $(LDFLAGS) -o $@ $< -lc

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@
	../fat/fput ../images/harddisk.img $< $<
	../fat/fput ../images/harddisk.img $@ $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o *.elf *.bin

.PHONY: all clean
