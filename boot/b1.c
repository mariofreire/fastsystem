asm (".code16gcc");

asm ("jmp main");

extern void putc(char c);

void print(const char *s) {
    while(*s) {
        putc(*s);
        ++s;
    }
}

void main()
{
  print("Hello World!");
}

