// Fast System Kernel Loader - Functions and Types from Base
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2024 DSP Interactive.

#ifndef __FUNCBASE_H__
#define __FUNCBASE_H__

#define NULL 0LL

#define TRUE 1
#define FALSE 0
#define true TRUE
#define false FALSE

#define isspace(c)                      (c == ' ')
#define isnumber(c)                      ((c >= '0') && (c <= '9'))
#define isalpha(c)                      (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')))
#define isalnum(c)                      (isalpha(c) || isnumber(c))
#define issign(c)                      ((c == '-') || (c == '+') || (c == '*') || (c == '/') || (c == '='))
#define isspecial(c)                      ((c == '\\') || (c == ';') || (c == '\'') || (c == '[') || (c == ']') || (c == ',') || (c == '.'))
#define isdigit(c)                      ((c >= '0') && (c <= '9'))

#define flagbit(value, index) ((value >> index) & ((1 << 1) - 1))

#define UCHAR8A(value) ((unsigned char)(value))
#define UCHAR8B(value) ((unsigned char)((value)>> 8))
#define UCHAR8C(value) ((unsigned char)((value)>>16))
#define UCHAR8D(value) ((unsigned char)((value)>>24))
#define UINT16(a,b) ((unsigned long)((unsigned char)(a)|((unsigned char)(b)<<8)))
#define UINT32(a,b,c,d) ((unsigned long)((unsigned char)(a)|((unsigned char)(b)<<8)|((unsigned char)(c)<<16)|((unsigned char)(d)<<24)))
#define USHORT16(a,b) ((unsigned long)(((unsigned long)(a)<<16)|((unsigned short)(b))))


#define HIGH16(a) ((unsigned short)(((a)>>16)&0xFFFF))
#define LOW16(a) ((unsigned short)((a)&0xFFFF))

typedef long unsigned int size_t;
typedef unsigned char bool;

extern size_t strlen(const char *s);
extern char *strcpy(char *s1, const char *s2);
extern void* memcpy(void *s1, const void *s2, size_t n);
extern void* memset(void *s, int c, size_t n);
extern unsigned char inb(unsigned short port);
extern unsigned short inw(unsigned short port);
extern unsigned long inl( unsigned short port );
extern void outb(unsigned short port, unsigned char value);
extern void outw(unsigned short port, unsigned short value);
extern void outl(unsigned short port, unsigned long value);
extern void printk(const char *msg, ...);

#endif // __FUNCBASE_H__
