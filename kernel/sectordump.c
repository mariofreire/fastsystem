// Fast System Sector Dump
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2025 DSP Interactive.

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>

#define SECTORSIZE 512

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

void dump_hex_address(unsigned long offset, const void *data, size_t size) {
    const unsigned char *buffer = (const unsigned char *)data;
    size_t i, j;

    for (i = 0; i < size; i += 16) {
        printf("%06X: ", (unsigned int)offset+i);
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
		uint32_t sectorcount;
		uint8_t *sector;
		if (argc > 2)
		{
			sectorcount = atol(argv[2]);
		}
		else
		{
			sectorcount = 1;
		}
		uint32_t result = 0;
		uint32_t nextsector = 0;
		uint32_t currentsector = atol(argv[1]);
		uint32_t firstcurrentsector = atol(argv[1]);
		sector = (uint8_t*)malloc(sectorcount*SECTORSIZE);
		if (sector == NULL) {
			printf("Cannot allocate memory.\n");
			return 0;
		}
		memset(sector, 0, sectorcount*SECTORSIZE);
		while(currentsector < firstcurrentsector+sectorcount)
		{
			result = readsector(currentsector, sector+(nextsector*SECTORSIZE));
			if (result == 0)
			{
				printf("Disk Error\n");
				free(sector);
				return 0;
			}
			currentsector++;
			nextsector++;
		}
		dump_hex_address(firstcurrentsector*SECTORSIZE, sector, sectorcount*SECTORSIZE);
		free(sector);
	}
	else
	{
		printf("Sector Dump         by   Mario Freire\n");
		printf("Copyright (C) 2025 DSP Interactive.\n");
		printf("\n");
		printf("Usage:\n");
		printf("sectordump [sector]\n");
		printf("sectordump [sector] [sectorcount]\n");
		printf("\n");
		printf("Example:\n");
		printf("sectordump 63\n");
		printf("sectordump 127 2\n");
		printf("\n");
	}
	return 0;
}

