// Fast System String
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2024 DSP Interactive.

#ifndef __STRING_H__
#define __STRING_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Character classification macros */

#define isspace(c) \
    ((c) == ' ')

#define isnumber(c) \
    (((c) >= '0') && ((c) <= '9'))

#define isalpha(c) \
    ((((c) >= 'a') && ((c) <= 'z')) || \
     (((c) >= 'A') && ((c) <= 'Z')))

#define isalnum(c) \
    (isalpha(c) || isnumber(c))

#define issign(c) \
    (((c) == '-') || ((c) == '+') || \
     ((c) == '*') || ((c) == '/') || \
     ((c) == '='))

#define isspecial(c) \
    (((c) == '\\') || ((c) == ';') || \
     ((c) == '\'') || ((c) == '[') || \
     ((c) == ']') || ((c) == ',') || \
     ((c) == '.'))


/* String functions */

size_t strlen(const char *s);

char *strcpy(char *s1, const char *s2);

char *strrev(const char *s);

int strcmp(const char *s1, const char *s2);

char *strcat(char *s1, const char *s2);

char *strncpy(char *s1, const char *s2, size_t n);

int strncmp(const char *s1, const char *s2, size_t n);

char *strupr(const char *s);

char *strlwr(const char *s);

char *strchr(const char *s, int c);

void strcatb(char *s1, char *s2);


/* Memory functions */

void *memset(void *s, int c, size_t n);

void *memcpy(void *s1, const void *s2, size_t n);


/* String formatting / conversion */

void str_pad_left(const char *s1, char *s2, int padding);

unsigned int atoh(char *s);

long int atol(char *s);

int atoi(char *s);

char *itob(unsigned long num, unsigned long base);

char *itob64(unsigned long long num, unsigned long long base);

char *itoa_s(unsigned long num, unsigned long base);

char *itoa(int value, char *str, int base);


#endif // __STRING_H__
