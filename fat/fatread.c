#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <limits.h>
#include "fatlib.h"

char *strupr1(const char *s);

int is_print(int c)
{
    return c >= 0x20 && c <= 0x7E;
}

void dump_hex(const void *data, size_t size) 
{
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
				printf("%c", is_print(buffer[i + j]) ? buffer[i + j] : '.');
			else
				printf(" ");
		}
        printf("\n");
    }
}

uint64_t fat_filesize(const char *filename)
{
    FAT_FILE *fp;
    off_t size;

    fp = fat_open(filename, "rb");
    if (fp == NULL)
        return 0;

    if (fat_seek(fp, 0, SEEK_END) != 0)
    {
        fat_close(fp);
        return 0;
    }

    size = fat_tell(fp);
    fat_close(fp);

    if (size < 0)
        return 0;

    return (uint64_t)size;
}

int main(int argc, char *argv[])
{
	char imagedisk_name[1024];
	char filename[1024];
	FAT_FILE *fp;
	
	if (argc > 2)
	{
		strcpy(imagedisk_name, argv[1]);
		strcpy(filename, argv[2]);
		if (fileexists(imagedisk_name))
		{
			if (initimagedisk(imagedisk_name))
			{
				if (hasactive())
				{
					if (isfattype())
					{
						if (loadfat())
						{
							fp = fat_open(filename, "rb");
							if (!fp)
							{
								fprintf(stderr, "Error: %s: File not found.\n", filename);
								uninitimagedisk();
								return -1;
							}
							else
							{
								printf("File name: %s\n", filename);
								uint64_t file_size = fat_filesize(filename);
								printf("File size: %ld\n", file_size);
								uint8_t *file_data = (uint8_t*)malloc(file_size);
								if (!file_data)
								{
									fprintf(stderr, "Error: Cannot allocate file data.\n");
									fat_close(fp);
									uninitimagedisk();
									return -1;
								}
								else
								{
									if (fat_read(file_data, file_size, 1, fp) != 0)
									{
										printf("---------------------------------------------------------------------------\n");
										dump_hex(file_data, file_size);
										printf("---------------------------------------------------------------------------\n");
									}
									else
									{
										fprintf(stderr, "File Read Error.\n");
										free(file_data);
										fat_close(fp);
										uninitimagedisk();
										return -1;
									}
								}
								free(file_data);							
								fat_close(fp);
							}
						}
					}
				}				
				uninitimagedisk();
			}
		}		
	}
	else
	{
		printf("FAT16/FAT32 Read File\n");
		printf("   Created by Mario Freire\n");
		printf("\n");
		printf("Usage: fatread [image-file] [file-name]\n");
		printf("Example: fatread harddisk.img loader\n");
		printf("\n");
	}
	return 0;
}
