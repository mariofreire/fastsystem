asm (".code32");
asm ("jmp _start");

extern int main(int argc, char **argv);


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
	char **argv;
	int exitcode=0;
	const char **argp = (const char **)param;
	argparam_t *args = (argparam_t*)argp;
	if (args == ((void*)0)) return -1;
	argc = args->argc;
	for(int i=0;i<argc;i++)
	{
		argv[i] = args->argv[i];
	}
	exitcode = main(argc, argv);
	//exit(exitcode);
	return exitcode;
}
