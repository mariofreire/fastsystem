// Fast System String
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2024 DSP Interactive.

#ifndef __IO_H__
#define __IO_H__

unsigned char inb(unsigned short port);
void outb(unsigned short port, unsigned char value);
unsigned short inw(unsigned short port);
void outw(unsigned short port, unsigned short value);
unsigned long inl( unsigned short port );
void outl(unsigned short port, unsigned long value);

#endif // __IO_H__
