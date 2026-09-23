// Fast System Console I/O
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2024 DSP Interactive.

#ifndef __CONIO_H__
#define __CONIO_H__

int getchar(void);
char getch(void);
char* gets(char *s);
void cprint(int color, const char *s);
int cprintf(int color, const char *msg, ...);
void clrscr();
void gotoxy(int x, int y);

#endif // __CONIO_H__
