// Fast System Console I/O
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2024 DSP Interactive.

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

void cprint(int color, const char *s)
{
	printf("\033[%dm %s\033[m", color, s);
}

int cprintf(int color, const char *msg, ...)
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

	cprint(color, msgBuf);
	
	return printed;	
}

char* gets(char *s)
{
	int len = sizeof(s);
	memset(s, 0, len);
	__asm__ volatile ( "int $0x80" :: "a" (3), "b" (0), "c" ((unsigned long)s), "d" (len) );
	return s;
}

int getchar(void)
{
	char s[2];
	char ch = 0;
	int len = 1;
	memset(s, 0, 2);
	__asm__ volatile ( "int $0x80" :: "a" (3), "b" (0), "c" ((unsigned long)s), "d" (len) );
	ch = s[0];
	return ch;
}

char getch(void)
{
	char ch = getchar();
	return ch;
}

void clrscr()
{
    printf("\033[2J");
    printf("\033[H");
}

void gotoxy(int x, int y)
{
	printf("\033[%d;%dH", x, y);
}
