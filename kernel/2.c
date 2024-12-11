#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct
{
    char name[128];
    char value[128];
} var_t;

var_t vars[256];
int vars_count = 4;


unsigned char checksum_str (char *str)
{
  unsigned char sum = 0;                                                                                                                                                                                           
  for (int i=128;i!=0;i--)
  {               
      sum = ((sum & 1) ? 0x80 : 0) + (sum >> 1) + *str++;
  }   
  return sum;
}

unsigned long get_sys_vars_checksum(void)
{
    unsigned long checksum=0;
    for(int i=0;i<vars_count;i++)
    {
        checksum += checksum_str(vars[i].name);
        checksum += checksum_str(vars[i].value);
        checksum += vars_count;
        checksum += vars_count >> i;
    }
    checksum <<= 4;
    checksum &= 0xffffffff;
	return checksum;
}

int main()
{
    char names[4][128] = {"INIT","ROOT","BOOT","AUTO"};
    srand(time(NULL));
    for(int i=0;i<vars_count;i++)
    {
        int r = rand()%vars_count;
        strcpy(vars[i].name, names[r]);
        printf("%s\n", vars[i].name);
    }
    
    printf("%d\n", (int)get_sys_vars_checksum());

    return 0;
}
