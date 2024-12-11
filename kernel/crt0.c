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

void _start(void)
{
	int exitcode=0;
	int argc = _get_edi();
	unsigned long _argv[256];
	unsigned long argv_addr = _get_esi();
	unsigned long argv_offset = 0;
	for (int i=0;i<256;i++)
	{
		_argv[i] = argv_addr+argv_offset;
		argv_offset += 256;
	}
	unsigned long a_addr = (unsigned long)_argv;	
	char **argv = (char**)a_addr;
	exitcode = main(argc, argv);
	exit(exitcode);
}
