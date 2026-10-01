// Fast System Application Test using libc (C Library)
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2024 DSP Interactive.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void cprint(int color, int state, const char *s)
{
	if (state)
		printf("\033[1;%dm%s\033[0m\n", color, s);
	else
		printf("\033[%dm%s\033[0m\n", color, s);
}

int main(int argc, char *argv[]) 
{
	int color;
	char msg[256];
	strcpy(msg, "Hello World!");
	if (argc > 1)
	{
		color = atol(argv[1]);
	}
	else
	{
		color = 37;
	}
	if ((argc == 1) || (argc == 2))
	{
		cprint(color,0,msg);
	}
	else
	{
		if (argc == 3)
		{
			cprint(color,0,argv[2]);
		}
		else
		{
			if (argc > 2)
			{
				cprint(color,atol(argv[2]),argv[3]);
			}
		}
	}
	return 0;
}



