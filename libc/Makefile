# $@ = target file
# $< = first dependency
# $^ = all dependencies

NASM ?= nasm
CC   ?= gcc
AR   ?= ar

CFLAGS := -m32 -nostdlib -nostartfiles -s -static -ffreestanding -fno-omit-frame-pointer -fno-builtin -fno-exceptions -fno-leading-underscore -fno-pie -fPIC -I. -r

ARFLAGS := rcs

NASMFLAGS := -f elf

# First rule is the one executed when no parameters are fed to the Makefile
all: libc.a libc-a.a

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	$(NASM) $< $(NASMFLAGS) -o $@

libc.a: crt0.o io.o conio.o stdio.o string.o errno.o syscall.o taskthread.o unistd.o arith64.o
	$(AR) $(ARFLAGS) $@ $^

libc-a.a: cstart.o io.o conio.o stdio.o string.o errno.o syscall.o taskthread.o unistd.o arith64.o
	$(AR) $(ARFLAGS) $@ $^

clean:
	rm -f *.o *.a
