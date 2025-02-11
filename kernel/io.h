// Fast System I/O
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2024 DSP Interactive.

#ifndef __IO_H__
#define __IO_H__

#include <stdint.h>

typedef struct
{
	unsigned long ds;
	unsigned long edi;
	unsigned long esi;
	unsigned long ebp;
	unsigned long esp;
	unsigned long ebx;
	unsigned long edx;
	unsigned long ecx;
	unsigned long eax;
	unsigned long int_no;
	unsigned long error_code;
	unsigned long eip;
	unsigned long cs;
	unsigned long eflags;
	unsigned long useresp;
	unsigned long ss;
} registers_t;

typedef struct 
{
    unsigned long edi;
	unsigned long esi;
	unsigned long ebp;
	unsigned long esp;
	unsigned long ebx;
	unsigned long edx;
	unsigned long ecx;
	unsigned long eax;
    unsigned short ds;
	unsigned short es;
	unsigned short fs;
	unsigned short gs;
	unsigned short ss;
    unsigned long eflags;
} registers32_t;

typedef void (*isr_t)(registers_t *);

extern unsigned char inb(unsigned short port);
extern unsigned short inw(unsigned short port);
extern unsigned long inl(unsigned short port);
extern void outb(unsigned short port, unsigned char value);
extern void outw(unsigned short port, unsigned short value);
extern void outl(unsigned short port, unsigned long value);
extern void msleep(unsigned int milliseconds);
extern void enable_interrupt(void);
extern void disable_interrupt(void);
extern void ehci_irq();
extern void set_irq(unsigned char num, isr_t base);
extern void outportb(unsigned short port, unsigned char value);
extern void outportw(unsigned short port, unsigned short value);
extern void outportl(unsigned short port, unsigned long value);
extern void sound(uint32_t freq);
extern void nosound();

#endif // __IO_H__
