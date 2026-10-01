// Fast System Standard Library
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2024 DSP Interactive.

#include "unistd.h"
#include "syscall.h"

int brk(void *addr)
{
	syscall_regs_t r;
	r = syscall1(45, (unsigned long)((void*)addr));
	return r.eax;
}

void *sbrk(unsigned long increment)
{
	syscall_regs_t r1;
	syscall_regs_t r2;
    static char *cur;
    char *old;    
    if (!cur) 
    {
    	r1 = syscall1(45, 0);
    	cur = (char *)((unsigned long)r1.eax);
    }
    if (increment == 0) return cur;
    old = cur;
    r2 = syscall1(45, (unsigned long)(cur + increment));
    if (r2.eax < 0) return (void *)-1;
    cur += increment;
    return old;
}

void exit(int status)
{
	syscall1(1, (unsigned long)status);
}

void exit_group(int status)
{
	syscall1(252, (unsigned long)status);
}

void *malloc(size_t size)
{
	syscall_regs_t r;
	r = syscall1(385, (unsigned long)size);
	return (void *)r.edx;
}

void free(void *ptr)
{
	syscall1(386, (unsigned long)ptr);
}

unsigned int msleep(unsigned int milliseconds)
{
	syscall_regs_t r;
	r = syscall1(162, milliseconds);
	return (unsigned int)r.eax;
}

unsigned int sleep(unsigned int seconds)
{
	syscall_regs_t r;
	unsigned int milliseconds = (seconds * 1000);
	r = syscall1(162, milliseconds);
	return (unsigned int)r.eax;
}




