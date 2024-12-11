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
	__asm__ volatile ( "int $0x80" :: "a" (4), "b" (1), "c" ((unsigned long)s), "d" (len) );
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
    __asm__ volatile ("mov %%edx, %0" : "=r" (edx_reg));
	return edx_reg;
}

unsigned long get_edi(void)
{
    unsigned long edi_reg;
    __asm__ volatile ("mov %%edi, %0" : "=r" (edi_reg));
	return edi_reg;
}

int main(int argc, char **argv)
{
	print("argc: ");
	printnum(argc);
	print("\n");
	for(int i=0;i<argc;i++)
	{
		print("argv[");
		printnum(i);
		print("]: ");		
		print(argv[i]);
		print("\n");
	}
	return 0;
}



