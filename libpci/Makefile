# $@ = target file
# $< = first dependency
# $^ = all dependencies

CC ?= gcc
AR ?= ar

CFLAGS := -m32 -nostdlib -nostartfiles -s -static -ffreestanding -fno-omit-frame-pointer -fno-builtin -fno-exceptions -fno-leading-underscore -fno-pie -fPIC -I. -r

# First rule is the one executed when no parameters are fed to the Makefile
all: libpci.a

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

libpci.a: pci.o
	$(AR) $(ARFLAGS) $@ $^

clean:
	rm -f *.o *.a
