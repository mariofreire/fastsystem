// Fast System I/O
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2024 DSP Interactive.

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <io.h>


unsigned char inb(unsigned short port)
{
    unsigned char r;
    __asm__ volatile   ( "inb %1, %0"
                   : "=a"(r)
                   : "Nd"(port)
                   : "memory");
    return r;
}

void outb(unsigned short port, unsigned char value)
{
    __asm__ volatile   ( "outb %0, %1"
				   : 
				   : "a"(value), 
				     "Nd"(port) 
				   : "memory");
}

unsigned short inw(unsigned short port)
{
    unsigned short r;
    __asm__ volatile   ( "inw %1, %0"
                   : "=a"(r)
                   : "Nd"(port)
                   : "memory");
    return r;
}

void outw(unsigned short port, unsigned short value)
{
    __asm__ volatile   ( "outw %0, %1"
				   : 
				   : "a"(value), 
				     "Nd"(port) 
				   : "memory");
}


unsigned long inl( unsigned short port )
{
  unsigned long r;
  __asm__ volatile    ("inl %1, %0\n"
				   : "=a"( r )
				   : "dN"( port ));
  return r;
}

void outl(unsigned short port, unsigned long value)
{
  __asm__ volatile ("outl %1, %0\n"
					:
					: "dN"(port),
					  "a"(value));
}

void enable_interrupt(void)
{
	__asm__ volatile ("sti");
}

void disable_interrupt(void)
{
	__asm__ volatile ("cli");
}

void outportb(unsigned short port, unsigned char value)
{
	disable_interrupt();
	outb(port, value);
	enable_interrupt();
}

void outportw(unsigned short port, unsigned short value)
{
	disable_interrupt();
	outw(port, value);
	enable_interrupt();
}

void outportl(unsigned short port, unsigned long value)
{
	disable_interrupt();
	outl(port, value);
	enable_interrupt();
}

void set_irq(unsigned char num, isr_t base)
{
	__asm__ volatile ( "int $0x80" :: "a" (486), "b" ((unsigned long)base), "c" (num+32), "d" (0) );
}

void sound(uint32_t freq)
{
 	uint32_t f_div;
 	uint8_t t;
 	f_div = (1193180 / freq);
 	outb(0x43, 0xb6);
 	outb(0x42, (uint8_t)(f_div));
 	outb(0x42, (uint8_t)(f_div >> 8));
 	t = inb(0x61);
  	if (t != (t | 3)) 
	{
 		outb(0x61, t | 3);
 	}
}

void nosound()
{
	uint8_t t;
	t = (inb(0x61) & 0xFC);
	outb(0x61, t);
}

unsigned long readsector(unsigned long sector, void *buffer)
{
	unsigned long result = 0LL;
	__asm__ volatile ( "int $0x80" : "=a"(result) : "a" (413), "b" (0), "c" (sector), "d" ((unsigned long)buffer) );
	return result;
}
