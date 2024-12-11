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



