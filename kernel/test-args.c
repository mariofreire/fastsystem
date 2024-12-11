#include <stdio.h>
#include <stdlib.h>

void getargv(char *s1, const char *s2, int n)
{
	char *tmp_argvx = s1;
	int tmp_argc=0;
	while(*s2)
	{
		if (*s2 == ' ') 
		{
			*tmp_argvx = '\0';
			tmp_argc++;
		}
		else 
		{
			if (tmp_argc == n)
			{
				*tmp_argvx++ = *s2;
			}
		}
		s2++;
	}
	*tmp_argvx = '\0';
}

int getargc(const char *s)
{
    int r=0;
    if (*s) r = 1;
	while(*s)
	{
		if (*s == ' ') 
		{
			r++;
		}
		s++;
	}
	return r;
}


int main(int argc, char *argv[])
{
    printf("\n");
    printf("Real Arguments:\n");
    printf("argc: %d\n", argc);
	for(int i=0;i<argc;i++)
	{
		printf("argv[%d]: %s\n", i, argv[i]);
	}
    printf("\n");
    printf("Simulated Arguments:\n");
	const char *tmp_sh = "mkdir -p /usr/share/folder1/folder2/folder3";
	int tmp_argc=getargc(tmp_sh);
	printf("argc: %d\n", tmp_argc);
	for(int i=0;i<tmp_argc;i++)
	{
		char *tmp_argv = (char*)malloc(512);
		getargv(tmp_argv, tmp_sh, i);
		printf("argv[%d]: %s\n", i, tmp_argv);
		free(tmp_argv);
	}
    printf("\n");
    return 0;
}
