// Fast System Standard Library
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2024 DSP Interactive.

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>

void *malloc(size_t size)
{
	unsigned long ptr_address = 0LL;
	__asm__ volatile ( "int $0x80" : "=d"(ptr_address) : "a" (385), "b" (size), "c" ((unsigned long)0), "d" (0) );
	return (void*)ptr_address;
}

void free(void *ptr)
{
	unsigned long ptr_address = (unsigned long)ptr;
	__asm__ volatile ( "int $0x80" :: "a" (386), "b" (ptr_address), "c" ((unsigned long)0), "d" (0) );
}

void msleep(unsigned int milliseconds)
{
	__asm__ volatile ( "int $0x80" :: "a" (162), "b" (milliseconds), "c" ((unsigned long)0), "d" (0) );
}

void sleep(unsigned int seconds)
{
	unsigned int milliseconds = (seconds * 1000);
	__asm__ volatile ( "int $0x80" :: "a" (162), "b" (milliseconds), "c" ((unsigned long)0), "d" (0) );
}





