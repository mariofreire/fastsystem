// Fast System String
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2024 DSP Interactive.

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define isspace(c)                      (c == ' ')
#define isnumber(c)                      ((c >= '0') && (c <= '9'))
#define isalpha(c)                      (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')))
#define isalnum(c)                      (isalpha(c) || isnumber(c))
#define issign(c)                      ((c == '-') || (c == '+') || (c == '*') || (c == '/') || (c == '='))
#define isspecial(c)                      ((c == '\\') || (c == ';') || (c == '\'') || (c == '[') || (c == ']') || (c == ',') || (c == '.'))

size_t strlen(const char *s)
{
	const char *s1 = s;
	if (s == NULL)
	{
		return 0;
	}
	for (s1 = s; *s1; s1++);
	return (s1 - s);
}

char *strcpy(char *s1, const char *s2)
{
	char *dest = s1;
	while ((*dest++ = *s2++) != 0);
	return s1;
}

char *strrev(const char *s)
{
	size_t len;
	char *dest = (char*)s;	
	char *s1;	
	char *s2;	
	char c;
	if (s == NULL)
	{
		return 0;
	}	
	len = strlen(s);
	if (len > 0)
	{
		for (s1=(char*)s, s2=((char*)s)+len;s1<s2--;s1++)
		{
			c = *s1;
			*s1 = *s2;
			*s2 = c;
		}
	}
	return dest;
}

int strcmp(const char *s1, const char *s2)
{
	while (*s1 == *s2)
	{
		if (!*s1)
		{
			return 0;
		}
		s1++;
		s2++;
	}
	return *(unsigned const char*)s1 - *(unsigned const char*)s2;	
}

char *strcat(char *s1, const char *s2)
{
	char *dest = s1;
	while (*dest)
	{
		*dest++;
	}
	while ((*dest++ = *s2++) != 0);
	return s1;
}

char* strncpy (char *s1, const char *s2, size_t n)
{
  char *dest = s1;
  while (n--)
  {
    if (!(*dest++ = *s2++))
    {
      while (n--)
        *dest++ = 0;
      break;
    }
  }
  return s1;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
	while (n--)
	{
		if (*s1 != *s2++)
		{
			return *(unsigned const char*)s1 - *(unsigned const char*)--s2;
		}
		if (!*s1++)
		{
			break;
		}
	}
	return 0;
}

char *strupr(const char *s)
{
	size_t len;
	char *dest = (char*)s;
	char *s1;
	const char c = ('a' - 'A');
	if (s == NULL)
	{
		return 0;
	}
	len = strlen(s);
	if (len > 0)
	{
		for (s1=(char*)s;*s1;s1++)
		{
			*s1 = (((*s1 >= 'a') && (*s1 <= 'z')) ? (*s1 - c) : *s1);
		}
	}
	return dest;
}

char *strlwr(const char *s)
{
	size_t len;
	char *dest = (char*)s;
	char *s1;
	const char c = ('a' - 'A');
	if (s == NULL)
	{
		return 0;
	}
	len = strlen(s);
	if (len > 0)
	{
		for (s1=(char*)s;*s1;s1++)
		{
			*s1 = (((*s1 >= 'A') && (*s1 <= 'Z')) ? (*s1 + c) : *s1);
		}
	}
	return dest;
}

char *strchr (const char *s, int c)
{
  char ch = (char)c;

  while (*s)
  {
    if (*s == ch)
	{
      return (char *)s;
	}
    s++;
  }

  if (!ch)
  {
    return (char *)s;
  }

  return NULL;
}

void strcatb(char* s1, char* s2)
{
	int i, j;
    int rlen = strlen(s1);
    int llen = strlen(s2);
    for(i=rlen;i>=0;i--)
    {
        s1[i+llen] = s1[i];
    }
    for(j=0;j<llen;j++)
    {
        s1[j] = s2[j];
    }
}

void* memset(void *s, int c, size_t n)
{
	unsigned char *dest = s;
	while (n--)
	{
		*dest++ = (unsigned char)c;
	}
	return s;
}

void* memcpy(void *s1, const void *s2, size_t n)
{
	char *dest = s1;
	const char *source = s2;
	while (n--)
	{
		*dest++ = *source++;
	}
	return s1;
}

void str_pad_left(const char *s1, char *s2, int padding)
{
	char *s = s2;
	if (!padding) return;
	int slen = strlen(s1);
	memset(s, ' ', padding);
	memcpy(s, s1, slen);
}

unsigned int atoh(char *s)
{
    unsigned int i=0;
    unsigned char c=0;
    while(*s)
    {
        c = (unsigned char)(*s);
        if ((c >= 'a') && (c <= 'f'))
        {
            c -= 'a';
            c += 'A';
        }
        if (((c >= '0') && (c <= '9'))
         || ((c >= 'A') && (c <= 'F')))
        {
            if ((c >= 'A') && (c <= 'F')) 
            {
                c -= 'A';
                c += 10;
            }
            else c -= '0';
            i *= 16;
            i += c;
        }
        s++;
    }
    return i;
}

long int atol(char *s)
{
    int i=0,n=0;
    unsigned char c=0;
    if (*s == '-')
    {
        n = 1;
        s++;
    }
    while(*s)
    {
        c = (unsigned char)(*s);
        if ((c >= '0') && (c <= '9'))
        {
            c -= '0';
            i *= 10;
            i += c;
        }
        s++;
    }
    if (n)
    {
        i = 0-i;
    }
    return i;
}

int atoi(char *s)
{
    return (int)atol(s);
}

char *itob(unsigned long num, unsigned long base)
{
  static char hold[] = "0123456789ABCDEFGHIJKLMNOPQRTSUVWXYZ";
  static char buffer[50];
  char *str;

  str = &buffer[49];
  *str = '\0';

  do {
    *--str = hold[num % base];
    num /= base;
  } while (num != 0);

  return str;
}

char *itob64(unsigned long long num, unsigned long long base)
{
  static char hold[] = "0123456789ABCDEFGHIJKLMNOPQRTSUVWXYZ";
  static char buffer[50];
  char *str;

  str = &buffer[49];
  *str = '\0';

  do {
    *--str = hold[num % base];
    num /= base;
  } while (num != 0);

  return str;
}

char *itoa_s(unsigned long num, unsigned long base)
{
  static char hold[] = "0123456789ABCDEFGHIJKLMNOPQRTSUVWXYZ";
  static char buffer[50];
  char *str;

  str = &buffer[49];
  *str = '\0';

  do {
    *--str = hold[num % base];
    num /= base;
  } while (num != 0);

  return str;
}

char *itoa(int value, char *str, int base)
{
    char * rc;
    char * ptr;
    char * low;
    if ( base < 2 || base > 36 )
    {
        *str = '\0';
        return str;
    }
    rc = ptr = str;
    if ( value < 0 && base == 10 )
    {
        *ptr++ = '-';
    }
    low = ptr;
    do
    {
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + value % base];
        value /= base;
    } while ( value );
    *ptr-- = '\0';
    while ( low < ptr )
    {
        char tmp = *low;
        *low++ = *ptr;
        *ptr-- = tmp;
    }
    return rc;
}

