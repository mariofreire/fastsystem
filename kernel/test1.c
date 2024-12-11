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

int main(void)
{
	print("Hello World!\n");
	return 0;
}



