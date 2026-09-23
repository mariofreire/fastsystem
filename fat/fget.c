#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "fatlib.h"

char *strupr1(const char *s);

const char *basename(const char *path)
{
    const char *base = path;

    if (path == 0)
        return 0;

    while (*path != '\0') {
        if (*path == '/' || *path == '\\')
            base = path + 1;

        path++;
    }

    return base;
}

int file_write(const char* filename, const char* data, int size)
{
	FILE *fp;
	fp = fopen(filename, "wb");
	if (fp == NULL) {
		return 1;
	}
	else {
		int fst = fwrite(data, size, 1, fp);
		if (fst != 1) {
			fclose(fp);
			return 2;
		}
	}
	fclose(fp);
	return 0;
}

int main(int argc, char *argv[])
{
	file_entry_t *entry;
    bool is_file = false;
    bool file_exists_on_disk = false;
    uint32_t first_cluster;
    uint32_t root_sector;
    uint32_t directory_sector;
	char filename[1024];
	char outfilename[1024];
	char imagedisk_name[1024];	
	if (argc > 1)
	{
		strcpy(imagedisk_name, argv[1]);
		
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
							root_sector = getrootdirsectorstart();
							directory_sector = root_sector;
							if (argc == 3)
							{
								strcpy(filename, argv[2]);
								strcpy(outfilename, basename(argv[2]));
								entry = findfile(root_sector, filename);
								if (entry != NULL)
								{
									file_exists_on_disk = true;
									first_cluster = getfilefirstcluster(entry);
									if (entry->attribute & F_ATTR_DIRECT) 
									{
										is_file = false;					
									}
									else 
									{
										is_file = true;					
    									if (first_cluster != 0)
    									{
    										directory_sector = clustertosector(first_cluster);								
        								}
									}
								}
								else 
								{
									file_exists_on_disk = false;
									is_file = false;
									directory_sector = root_sector;
								}
    							if (file_exists_on_disk)
    							{
									if (is_file)
									{
    									if (first_cluster != 0)
    									{
        									uint32_t file_size = getfilesize(filename, directory_sector);
        									uint8_t *buffer = (uint8_t*)malloc(file_size);
        									if (buffer != 0)
        									{
        										if (getfiledata(filename, directory_sector, buffer))
        										{
    												int status = file_write(outfilename, buffer, file_size);
													if (status == 0)
													{
														printf("Got file successfully.\n");
													}
													else
													{
														if (status == 1) fprintf(stderr, "Error: Cannot create file to write.\n");
														else if (status == 2) fprintf(stderr, "Error: Cannot write file.\n");
														else fprintf(stderr, "Error: Cannot get file to write.\n");
														free(buffer);
														uninitimagedisk();
														return 1;
													}
        										}
        										else
        										{
                									fprintf(stderr, "Error: Cannot read file data.\n");
                									free(buffer);
                									uninitimagedisk();
                									return 1;
        										}
            									free(buffer);
        									}
        									else
            								{
                								fprintf(stderr, "Error: Cannot allocate file buffer.\n");
                								uninitimagedisk();
                								return 1;
            								}
    									}
									}	
								}	
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
		printf("FAT16/FAT32 File Get\n");
		printf("   Created by Mario Freire\n");
		printf("\n");
		printf("Usage: fget [image-file] [file-name]\n");
		printf("Example: fget harddisk.img loader\n");
		printf("\n");
	}
	return 0;
}
