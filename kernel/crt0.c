asm (".code32");
asm ("jmp _start");

#include <stdint.h>
#include <stddef.h>

extern int main(int argc, char **argv);

extern void *malloc(size_t size);
extern void free(void *ptr);


void exit(int code)
{
	__asm__ volatile ( "int $0x80" :: "a" (1), "b" (code) );
}

unsigned long _get_esi(void)
{
    unsigned long esi_reg;
    __asm__ volatile ("mov %%esi, %0" : "=r" (esi_reg));
	return esi_reg;
}

unsigned long _get_edi(void)
{
    unsigned long edi_reg;
    __asm__ volatile ("mov %%edi, %0" : "=r" (edi_reg));
	return edi_reg;
}


typedef struct
{
	int argc;
	char argv[256][256];
} argparam_t;

int _start(void *param)
{
	int argc;
	char **argv = (char**)malloc(256*256);
	int exitcode=0;
	if (param == ((void*)0)) return -1;
	const void **argp = (const void **)param;
	argparam_t *args = (argparam_t*)argp;
	if (args == ((void*)0)) return -1;
	if (argp == ((void*)0)) return -1;
	argc = args->argc;
	for(int i=0;i<argc;i++)
	{
		if (args->argv[i] == NULL)
		{
			argc = i;
			break;
		}
		argv[i] = args->argv[i];
	}
	exitcode = main(argc, argv);
	free(argv);
	exit(exitcode);
	return exitcode;
}
