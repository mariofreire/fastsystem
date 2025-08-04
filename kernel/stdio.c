// Fast System Standard I/O
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2024 DSP Interactive.

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>

#define isspace(c)                      (c == ' ')
#define isnumber(c)                      ((c >= '0') && (c <= '9'))
#define isalpha(c)                      (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')))
#define isalnum(c)                      (isalpha(c) || isnumber(c))
#define issign(c)                      ((c == '-') || (c == '+') || (c == '*') || (c == '/') || (c == '='))
#define isspecial(c)                      ((c == '\\') || (c == ';') || (c == '\'') || (c == '[') || (c == ']') || (c == ',') || (c == '.'))

char *itoa(int value, char *str, int base);
char *itoa_s(unsigned long num, unsigned long base);
char *strupr(const char *s);

char fillchar_b[2];

void putch(const char c)
{
	memset(fillchar_b, 0, 2);
	fillchar_b[0] = c;
	__asm__ volatile ( "int $0x80" :: "a" (4), "b" (1), "c" ((unsigned long)fillchar_b), "d" (1) );
}

void putchar(const char c)
{
	putch(c);
}

void print(const char *s)
{
	int len = strlen(s);
	__asm__ volatile ( "int $0x80" :: "a" (4), "b" (1), "c" ((unsigned long)s), "d" (len) );
}

void printnum(int x)
{
	print(itoa_s(x, 10));
}

void printhex(int x)
{
	print(itoa_s(x, 16));
}

int pvsnprintf(char* str, size_t size, const char* format, va_list ap)
{
	char buf[64];
	char *s;
	int i = 0;
	int z_n = 0;
	int p_n = 0;
	int s_n = 0;
	for (;*format;++format)
	{
		switch (*format)
		{
			case '%':
			{
				z_n = 0;
				p_n = 0;
				s_n = 0;
				s_format_chk:
				switch(*(++format))
				{
					case '-':
					{
						s_n = 1;
						goto s_format_chk;
					}
					break;
					case '0':
					{
						z_n = 0;
						while (isnumber(*format))
						{
							char n_c = (*format);
							n_c -= '0';
							z_n *= 10;
							z_n += n_c;
							format++;
						}
						*(--format);
						goto s_format_chk;
					}
					break;
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
					{
						p_n = 0;
						while (isnumber(*format))
						{
							char n_c = (*format);
							n_c -= '0';
							p_n *= 10;
							p_n += n_c;
							format++;
						}
						*(--format);
						goto s_format_chk;
					}
					break;
					case 's':
					{
						int l=0;
						int p_ln = p_n;
						s = va_arg(ap, char*);
						if (p_n > 0)
						{
							int s_ln = strlen(s);
							for(int j=0;j<s_ln;j++) p_n--;
							for(int k=0;k<p_n;k++) *str++ = ' ';
							if (s_n)
							{
								for(l=0;l<p_n;l++)
								{
									str--;
								}
							}
							else
							{
								if (p_n >= strlen(s)) *str -= strlen(s);
							}
						}
						while(*s)
						{
							*str++ = *s++;
						}
						if ((s_n) && ((p_n > 0) && (p_ln > 0) && (l > 0)))
						{							
							for(int n=0;n<(p_n);n++)
							{
								*str++ = ' ';
							}							
						}
						*s++ = '\0';
					}
					break;
					case 'c':
					{
						int l=0;
						int p_ln = p_n;
						if (p_n > 0)
						{
							int s_ln = 1;
							for(int j=0;j<s_ln;j++) p_n--;
							for(int k=0;k<p_n;k++) *str++ = ' ';
							if (s_n)
							{
								for(l=0;l<p_n;l++)
								{
									str--;
								}
							}
							else
							{
								if (p_n >= 1) *str -= 1;
							}
						}
						*str++ = va_arg(ap, int);
						if ((s_n) && ((p_n > 0) && (p_ln > 0) && (l > 0)))
						{
							for(int n=0;n<(p_ln-l);n++)
							{
								*str++ = ' ';
							}
						}
						*s++ = '\0';
					}
					break;
					case 'd':
					case 'i':
					{
						int l=0;
						int p_ln = p_n;
						itoa(va_arg(ap, long), buf, 10);
						s = buf;
						if (p_n > 0)
						{
							int s_ln = strlen(s);
							for(int j=0;j<s_ln;j++) p_n--;
							for(int k=0;k<p_n;k++) *str++ = ' ';
							if (s_n)
							{
								for(l=0;l<p_n;l++)
								{
									str--;
								}
							}
							else
							{
								if (p_n >= strlen(buf)) *str -= strlen(buf);
							}
						}
						if (z_n > 0)
						{
							int s_ln = strlen(s);
							for(int j=0;j<s_ln;j++) z_n--;
							for(int k=0;k<z_n;k++) *str++ = '0';
						}
						while(*s)
						{
							*str++ = *s++;
						}
						if ((s_n) && ((p_n > 0) && (p_ln > 0) && (l > 0)))
						{
							for(int n=0;n<(p_ln-l);n++)
							{
								*str++ = ' ';
							}
						}
						*s++ = '\0';
					}
					break;
					case 'u':
					{
						int l=0;
						int p_ln = p_n;
						s = itoa_s(va_arg(ap, long), 10);
						if (p_n > 0)
						{
							int s_ln = strlen(s);
							for(int j=0;j<s_ln;j++) p_n--;
							for(int k=0;k<p_n;k++) *str++ = ' ';
							if (s_n)
							{
								for(l=0;l<p_n;l++)
								{
									str--;
								}
							}
							else
							{
								if (p_n >= strlen(s)) *str -= strlen(s);
							}
						}
						if (z_n > 0)
						{
							int s_ln = strlen(s);
							for(int j=0;j<s_ln;j++) z_n--;
							for(int k=0;k<z_n;k++) *str++ = '0';
						}
						while(*s)
						{
							*str++ = *s++;
						}
						if ((s_n) && ((p_n > 0) && (p_ln > 0) && (l > 0)))
						{
							for(int n=0;n<(p_ln-l);n++)
							{
								*str++ = ' ';
							}
						}
						*s++ = '\0';
					}
					break;
					case 'x':
					{
						int l=0;
						int p_ln = p_n;
						s = itoa_s(va_arg(ap, long), 16);
						if (p_n > 0)
						{
							int s_ln = strlen(s);
							for(int j=0;j<s_ln;j++) p_n--;
							for(int k=0;k<p_n;k++) *str++ = ' ';
							if (s_n)
							{
								for(l=0;l<p_n;l++)
								{
									str--;
								}
							}
							else
							{
								if (p_n >= strlen(s)) *str -= strlen(s);
							}
						}
						if (z_n > 0)
						{
							int s_ln = strlen(s);
							for(int j=0;j<s_ln;j++) z_n--;
							for(int k=0;k<z_n;k++) *str++ = '0';
						}
						while(*s)
						{
							*str++ = *s++;
						}
						if ((s_n) && ((p_n > 0) && (p_ln > 0) && (l > 0)))
						{
							for(int n=0;n<(p_ln-l);n++)
							{
								*str++ = ' ';
							}
						}
						*s++ = '\0';
					}
					break;
					case 'X':
					{
						int l=0;
						int p_ln = p_n;
						s = strupr(itoa_s(va_arg(ap, long), 16));
						if (p_n > 0)
						{
							int s_ln = strlen(s);
							for(int j=0;j<s_ln;j++) p_n--;
							for(int k=0;k<p_n;k++) *str++ = ' ';
							if (s_n)
							{
								for(l=0;l<p_n;l++)
								{
									str--;
								}
							}
							else
							{
								if (p_n >= strlen(s)) *str -= strlen(s);
							}
						}
						if (z_n > 0)
						{
							int s_ln = strlen(s);
							for(int j=0;j<s_ln;j++) z_n--;
							for(int k=0;k<z_n;k++) *str++ = '0';
						}						
						while(*s)
						{
							*str++ = *s++;
						}
						if ((s_n) && ((p_n > 0) && (p_ln > 0) && (l > 0)))
						{
							for(int n=0;n<(p_ln-l);n++)
							{
								*str++ = ' ';
							}
						}
						*s++ = '\0';
					}
					break;
					case '%':
					{
						*str++ = '%';
					}
					break;
					default:
					{
						--format;
					}
					break;
				};
				continue;
			}
			break;
			default:
			{
				*str++ = *format;
			}
			break;
		};
	}
	*str++ = '\0';
	return 0;
}

int snprintf (char *s, size_t maxlen, const char *format, ...)
{
  va_list arg;
  int done;

  va_start (arg, format);
  done = pvsnprintf(s, maxlen, format, arg);
  va_end (arg);

  return done;
}

int vsprintf (char* str, const char* format, va_list arglist)
{
  return pvsnprintf (str, 1000, format, arglist);
}

int sprintf(char *buf, const char *msg, ...)
{
	va_list va_alist;
	int msgLen = 0;

	if (!msg) return msgLen;

	va_start(va_alist, msg);
	msgLen = vsprintf(buf, msg, va_alist);
	va_end(va_alist);
	
	return msgLen;	
}

int printf(const char *msg, ...)
{
	//int slen = strlen(msg)*4+1;
	char msgBuf[1024];// = (char*)malloc(slen);
	va_list va_alist;
	int printed = 0;

	if (!msg) return printed;

	//msgBuf[slen - 1] = '\0';
	msgBuf[1024 - 1] = '\0';
	va_start(va_alist, msg);
	printed = vsprintf(msgBuf, msg, va_alist);
	va_end(va_alist);

	print(msgBuf);
	
	return printed;	
}
