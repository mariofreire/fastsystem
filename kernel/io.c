// Fast System I/O
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2024 DSP Interactive.

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>


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


