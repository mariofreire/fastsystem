// Fast System Standard Library
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2024 DSP Interactive.

#ifndef __STDLIB_H__
#define __STDLIB_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

void *malloc(size_t size);
void free(void *ptr);
void msleep(unsigned int milliseconds);
void sleep(unsigned int seconds);

#endif // __STDLIB_H__
