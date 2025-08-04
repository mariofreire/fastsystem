#include <io.h>
#include <conio.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int mouse_x=0;
int mouse_y=0;
int mouse_b=0;

int oldmouse_x=0;
int oldmouse_y=0;
int oldmouse_b=0;

void getmouse(int *x, int *y, int *b)
{
	int m_x = 0;
	int m_y = 0;
	int m_b = 0;
	__asm__ volatile ( "int $0x80" : "=c"(m_x),"=d"(m_y),"=b"(m_b) : "a" (512), "b" (0), "c" (0), "d" (0) );
	*x = m_x;
	*y = m_y;
	*b = m_b;
}

//Mouse functions
void mouse_handler()
{
  oldmouse_x = mouse_x;
  oldmouse_y = mouse_y;
  oldmouse_b = mouse_b;
  getmouse(&mouse_x, &mouse_y, &mouse_b);
  if ((oldmouse_x != mouse_x) || (oldmouse_y != mouse_y) || (oldmouse_b != mouse_b))
  {
	  clrscr();
	  printf("Cursor Position: X = %i, Y = %i, B = %i\n", mouse_x, mouse_y, mouse_b);
  }
}

int initmouse()
{
	int r=0;
	//mouse_install
	//__asm__ volatile ( "int $0x80" : "=a"(r) : "a" (508), "b" (0), "c" (0), "d" (0) );
	__asm__ volatile ( "int $0x80" : "=a"(r) : "a" (513), "b" (0), "c" (0), "d" (0) );
	return r;
}

void uninitmouse()
{
	//mouse_uninstall
	__asm__ volatile ( "int $0x80" : : "a" (514), "b" (0), "c" (0), "d" (0) );
}

int main() 
{
	int exit_q = 0;
	if (initmouse()) {
		while(exit_q == 0)
		{
			mouse_handler();
			if (mouse_b & 1) exit_q = 1;
		}
	}
	uninitmouse();
    return 0;
}

