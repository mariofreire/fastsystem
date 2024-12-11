// UUID v4 Generator
// Author: Mario Freire
// Version 1.0
// Copyright (C) 2024 DSP Interactive.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef RAND_MAX
#define RAND_MAX 32767
#endif

unsigned long int rand_next = 1;

void fsrand(unsigned int seed)
{
    rand_next = seed;
}

int frand(void)
{
    int r;
    rand_next = ((rand_next * 1103515245) + 12345);
    r = ((unsigned int)(rand_next / 65536) % 32768);
    return r;
}

char *HexToStr(unsigned long val)
{
    char table[17] = {"0123456789abcdef"};
    char hex[8];
    char *r;
    int i=val;
    memset(hex, 0, 8);
    r = (char*)hex;
    char *s = r;
    char *rev = s;
    if (val == 0)
    {
        *s++ = '0';
    }    
    while(i > 0)
    {
        *s++ = table[(i % 16)];
        i /= 16;
    }
    *s-- = '\0';
    while(rev < s)
    {
        char ch = *rev;
        *rev++ = *s;
        *s-- = ch;
    }
    r = (char*)hex;
    return r;
}

unsigned long StrToHex(char* str)
{
    unsigned long hex,val;
    int n;
    hex=0;
    for(n=0;n<strlen(str);n++) 
    {
		if(str[n] >= '0' && str[n] <= '9')
		{
			val = (unsigned long)(str[n]-'0');
		}
		else if(str[n] >= 'a' && str[n] <= 'f')
		{
			val = ((unsigned long)(str[n]-'a')) + 0xA;
		}
		else if(str[n] >= 'A' && str[n] <= 'F')
		{
			val = ((unsigned long)(str[n]-'A')) + 0xA;
		}
		else
		{
			val = 0;
		}
		hex <<= 4;
		hex |= val;
    }
    return hex;
}

void uuidv4(char *str)
{
    char uuid[37];
    char base[37];
    strcpy(base, "10000000-1000-4000-8000-100000000000");
    int len = strlen(base);
    int slen=0;
    uuid[0] = '\0'; 
    for(int i=0;i<len;i++)
    {
        if ((base[i] >= '0') && (base[i] <= '9'))
        {
            char ch[2];
            ch[0] = base[i];
            ch[1] = '\0';
            unsigned char n = StrToHex(ch);
            unsigned char c = (n ^ frand() & (15 >> (n / 4)));
            if (slen == 12) c = ((base[slen+2] - '0') % 10);
            char hex[2];
            strcpy(hex, HexToStr(c));
            strcat(uuid, hex);
            if (slen == 7) strcat(uuid, "-");
            if (slen == 11) strcat(uuid, "-");
            if (slen == 15) strcat(uuid, "-");
            if (slen == 19) strcat(uuid, "-");
            slen++;
        }
    }
    slen += 4;
    uuid[slen] = '\0';     
    strcpy(str, uuid);
}

int uuidv4_validate(const char *str)
{
    int r=0, v=0, i=0;
    if (!str) return 0;
    if (strlen(str) != 36) return 0;
    while(*str)
    {
        if ((i == 8) && (*str != '-')) v--;
        if ((i == 13) && (*str != '-')) v--;
        if ((i == 18) && (*str != '-')) v--;
        if ((i == 23) && (*str != '-')) v--;
        if (((*str >= '0') && (*str <= '9')) || 
        ((*str >= 'a') && (*str <= 'f')) || 
        ((*str >= 'A') && (*str <= 'F')))
        {
            if (((i >= 0) && (i <= 7)) || 
                ((i >= 9) && (i <= 12)) || 
                ((i >= 14) && (i <= 17)) || 
                ((i >= 19) && (i <= 22)) || 
                ((i >= 24) && (i <= 36)))
            {
                if ((i == 14) && (*str != '4')) v--;
                if ((i == 19) && 
                ((*str != '8' && (*str != '9' && 
                (*str != 'a' && (*str != 'b')) && 
                (*str != 'A' && (*str != 'B')))))) v--;
                v++;
            }
        }
        i++;
        str++;
    }
    if (v == 32) r = 1;
    return r;
}

int main()
{
    char uuid1[75];
    fsrand(time(NULL));
    uuidv4(uuid1);
	if (uuidv4_validate(uuid1))
	{
		printf("%s\n", uuid1);
	}
    return 0;
}
