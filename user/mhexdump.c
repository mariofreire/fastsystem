// Fast System Memory Hex Dump
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2024 DSP Interactive.

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define ADDRESS(x) ((void*)(uint32_t)x)

unsigned int atoh(char *s);

int isprint(int c) {
    return c >= 0x20 && c <= 0x7E;
}

void dump_hex(const void *data, size_t size) {
    const unsigned char *buffer = (const unsigned char *)data;
    size_t i, j;

    for (i = 0; i < size; i += 16) {
        printf("%06X: ", (unsigned int)i);
        for (j = 0; j < 16; j++) {
            if (i + j < size)
                printf("%02X ", buffer[i + j]);
            else
                printf("   ");	
        }		
		printf(" ");			
		for (int j = 0; j < 16; j++) {
			if (i + j < size)
				printf("%c", isprint(buffer[i + j]) ? buffer[i + j] : '.');
			else
				printf(" ");
		}
        printf("\n");
    }
}

int main(int argc, char *argv[]) 
{
	if (argc > 1)
	{
		size_t dump_size;
		if (argc == 3) dump_size = atol(argv[2]);
		else dump_size = 0x100;
		dump_hex((unsigned char*)atoh(argv[1]), dump_size);
	}
	else
	{
		printf("Memory Hex Dump         by   Mario Freire\n");
		printf("Copyright (C) 2024 DSP Interactive.\n");
		printf("\n");
		printf("Usage:\n");
		printf("mhexdump [memory-address]\n");
		printf("mhexdump [memory-address] [dump-size]\n");
		printf("\n");
		printf("Example:\n");
		printf("mhexdump 0xB8000\n");
		printf("mhexdump 0x7C00 512\n");
		printf("\n");
	}
	return 0;
}


