#include <stdio.h>

void print(const char *s)
{
	printf("%s", s);
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

int main(int argc, char **argv)
{
	int i;
	print("argc: ");
	printnum(argc);
	print("\n");
	for(i=0;i<argc;i++)
	{
		print("argv[");
		printnum(i);
		print("]: ");		
		print(argv[i]);
		print("\n");
	}
	return 0;
}



