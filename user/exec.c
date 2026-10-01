#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <unistd.h>

int execv(const char *path, char *const argv[])
{
    int result;
    __asm__ volatile (
        "int $0x80"
        : "=a"(result)
        : "a" (11), "b" ((unsigned long)path), "c" ((unsigned long)argv), "d" ((unsigned long)0)
        : "memory"
    );
    return result;
}

void sys_execv(const char *filename, char *const argv[])
{
	__asm__ volatile ( "int $0x80" :: "a" (11), "b" ((unsigned long)filename), "c" ((unsigned long)argv), "d" ((unsigned long)0) );
}
void sys_execve(const char *filename, char *const argv[], char *const envp[])
{
	__asm__ volatile ( "int $0x80" :: "a" (11), "b" ((unsigned long)filename), "c" ((unsigned long)argv), "d" ((unsigned long)envp) );
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("Error\n");
		return 1;
	}
	if (argc > 1)
	{
		char _argv[256][256];
		int i=0;
		while(i < 255)
		{
			if (argv[i+1] != 0)
			{
				strcpy(_argv[i], argv[i+1]);
				i++;
			}
			else break;
		}
		_argv[i][0] = '\0';
		//memcpy(&_argv[0][0], &argv[1], 256*255);
		//char **_argv = argv+1;
		//sys_execve(argv[1], NULL, NULL);
		int result = execv(argv[1], (char**)_argv);
		exit(result);
	}
	return 0;
}
