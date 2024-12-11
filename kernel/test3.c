int strlen(const char *s)
{
	int i=0;
	while (*s)
	{
		i++;
		*s++;
	}
	return i;
}

void print(const char *s)
{
	int len = strlen(s);
	asm( "int $0x80" :: "a" (4), "b" (1), "c" (s), "d" (len) );
}

char *itoa_s(unsigned long num, unsigned long base)
{
  static char hold[] = "0123456789ABCDEFGHIJKLMNOPQRTSUVWXYZ";
  static char buffer[50];
  char *str;

  str = &buffer[49];
  *str = '\0';

  do {
    *--str = hold[num % base];
    num /= base;
  } while (num != 0);

  return str;
}

void printnum(int x)
{
	print(itoa_s(x, 10));
}

void printhex(int x)
{
	print(itoa_s(x, 16));
}

unsigned long get_edx(void)
{
    unsigned long edx_reg;
    asm("mov %%edx, %0" : "=r" (edx_reg));
	return edx_reg;
}

unsigned long get_edi(void)
{
    unsigned long edi_reg;
    asm("mov %%edi, %0" : "=r" (edi_reg));
	return edi_reg;
}

int main(int argc, char **argv)
{
	print("argv=");
	printhex((unsigned long)argv);
	print("\n");
	print("argv[0]=");
	printhex((unsigned long)argv[0]);
	print("\n");
	print("argv[1]=");
	printhex((unsigned long)argv[1]);
	print("\n");
	print("argc: ");
	printnum(argc);
	print("\n");
	print("argv:");
	for(int i=0;i<argc;i++)
	{
		print(" ");		
		print(argv[i]);
	}
	print("\n");
	return 0;
}



