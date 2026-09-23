// Fast System Standard I/O
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2024 DSP Interactive.

#ifndef __STDIO_H__
#define __STDIO_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>

/* String / conversion functions */
char *itoa(int value, char *str, int base);
char *itoa_s(unsigned long num, unsigned long base);
char *strupr(const char *s);

/* Character / output functions */
void putch(const char c);
void putchar(const char c);
void print(const char *s);
void printnum(int x);
void printhex(int x);

/* Formatted output functions */
int pvsnprintf(char *str, size_t size, const char *format, va_list ap);
int snprintf(char *s, size_t maxlen, const char *format, ...);
int vsprintf(char *str, const char *format, va_list arglist);
int sprintf(char *buf, const char *msg, ...);
int printf(const char *msg, ...);

#endif // __STDIO_H__
