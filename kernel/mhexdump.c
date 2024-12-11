// Fast System Memory Hex Dump
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2024 DSP Interactive.

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

unsigned int atoh(char *s);

void dump_hex(const void *data, size_t size) {
    const unsigned char *buffer = (const unsigned char *)data;
    size_t i, j;

    for (i = 0; i < size; i += 16) {
        printf("%06x: ", (unsigned int)i);

        for (j = 0; j < 16; j++) {
            if (i + j < size)
                printf("%02x ", buffer[i + j]);
            else
                printf("   ");

            if (j % 16 == 7)
                printf(" ");
        }

        printf("\n");
    }
}

int main(int argc, char *argv[]) 
{
	if (argc > 1)
	{
		dump_hex((unsigned char*)atoh(argv[1]), 0x100);
	}
	else
	{
		printf("Memory Hex Dump         by   Mario Freire\n");
		printf("Copyright (C) 2024 DSP Interactive.\n");
		printf("\n");
		printf("Usage:\n");
		printf("mhexdump [memory-address]\n");
		printf("\n");
		printf("Example:\n");
		printf("mhexdump 0xB8000\n");
		printf("\n");
	}
	return 0;
}

