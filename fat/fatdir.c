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

int main(int argc, char *argv[])
{
	file_entry_t *root_dir;
	file_entry_t *first_file;
	file_entry_t *find_file;
	file_entry_t *find_path;
	uint32_t files_count;
	uint32_t root_sector;
	uint32_t sector;
	uint32_t fsector;
	uint32_t lsector;
	uint32_t cluster;
	int path_opt = 0;
	int i;
	int j;
	int has_lfn = 0;
	bool detail_first_file;
	char* column[6] = {"Name", "Type", "Size", "Cluster", "Sector", "Position"};
	path_sub_t path_sub;
	char shortfilename[MAX_FILENAME_LENGTH];
	char longfilename[1024];
	char filename[1024];
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
							detail_first_file = false;
							root_sector = getrootdirsectorstart();
							sector = root_sector;
							memset(shortfilename, 0, sizeof(shortfilename));
							if (argc >= 3)
							{
								for(int optindex=2;optindex<argc;optindex++)
								{
									if (argv[optindex][0] == '-')
									{
										if (strcmp(argv[optindex], "-d") == 0)
										{
											detail_first_file = true;
										}
										else if (strcmp(argv[optindex], "-s") == 0)
										{
											if (argc >= 4)
											{
												if (optindex < argc-1)
												{
													optindex++;
													if (isdigit(argv[optindex][0]))
													{
														sector = atol(argv[optindex]);
														if (sector < root_sector)
														{
															sector = root_sector;
														}
														else if (sector > (imagedisk_size/SECTORSIZE))
														{
															sector = root_sector;
														}
													}
													else
													{
														fprintf(stderr, "fatdir: invalid sector: \"%c\"\n", argv[optindex][0]);
														sector = root_sector;
													}
												}
												else
												{
													fprintf(stderr, "fatdir: specify sector\n");
													sector = root_sector;
												}
											}
											else
											{
												fprintf(stderr, "fatdir: specify sector\n");
												sector = root_sector;
											}
										}
										else
										{
											if (argv[optindex][0] == '-')
											{
												fprintf(stderr, "fatdir: invalid option: \"%c\"\n", argv[optindex][1]);
											}
											sector = root_sector;
										}	
									}
									else
									{
										path_opt = optindex;
									}								
								}
								
								lsector = sector;									
								if (path_opt != 0)
								{
									path_sub = getpath(argv[path_opt]);
    								cluster = 0;
    								if (path_sub.pathcount == 1)
    								{
        								sector = root_sector;
    								}
    								char newpath[1024];
    								memset(newpath, 0, 1024);
    								newpath[0] = '/';
									for(j=0;j<path_sub.pathcount;j++)
									{
										if (path_sub.path[j].path[0] != 0)
										{
											strcat(newpath, path_sub.path[j].path);
											strcat(newpath, "/");
											find_path = findfile(sector, newpath);
											if (find_path != NULL)
											{											
												if (find_path->attribute & F_ATTR_DIRECT)
												{
													cluster = getfilefirstcluster(find_path);
													sector = clustertosector(cluster);
													if (isfat16type()) 
													{
														if (cluster == 0) sector = root_sector;
													}		
													memset(shortfilename, 0, sizeof(shortfilename));
            										getshortfilename(find_path->name, shortfilename, sizeof(shortfilename));   
													strcpy(longfilename, shortfilename);
													if (getlongfilename(longfilename, lsector))
													{
														strcpy(filename, longfilename);
													}
													else
													{
														strcpy(filename, shortfilename);
													}
													lsector = sector;
													printf("Directory[%d] '%s' located at 0x%08lX\n", j, filename, sectortobytes(sector));	
												}
											}										
										}
									}
								}
							}
							
							printf("Root Address 0x%08lX\n", sectortobytes(getrootdirsector()));
							printf("-------------------------------------------------------------------------------------------------------------------\n");
							printf("%-32s\t%-10s\t%-10s\t%-10s\t%-10s\t%-10s\n", column[0], column[1], column[2], column[3], column[4], column[5]);
							printf("-------------------------------------------------------------------------------------------------------------------\n");
							files_count = listdir(sector);
							printf("-------------------------------------------------------------------------------------------------------------------\n");
							printf("%-32s\t%-10s\t%-10s\t%-10s\t%-10s\t%-10s\n", column[0], column[1], column[2], column[3], column[4], column[5]);
							printf("-------------------------------------------------------------------------------------------------------------------\n");
							printf("Total: %u\n", files_count);
							if (files_count > 0)
							{
								i = 0;
								root_dir = getfileentryofsector(sector);
								first_file = &root_dir[0];
								while((first_file->attribute == F_ATTR_LNGFNM) || (first_file->attribute == F_ATTR_VOLMID) || ((uint8_t)first_file->name[0] == FILE_NAME_DELETED) || (((uint8_t)first_file->name[0] == '.') && (((uint8_t)first_file->name[1] == '.') || ((uint8_t)first_file->name[1] == ' '))))
								{
									if (first_file->attribute == F_ATTR_LNGFNM) has_lfn = 1;
									i++;
									if (i >= 16) break;
									first_file = &root_dir[i];
								}
								if (detail_first_file)
								{
									printf("------------------------------------------------------------------------------------------\n");
									fsector = getfirstsectorofcluster(getfilefirstcluster(first_file));
									memset(shortfilename, 0, sizeof(shortfilename));
            						getshortfilename(first_file->name, shortfilename, sizeof(shortfilename));   
									strcpy(longfilename, shortfilename);
									if (has_lfn)
									{
										if (getlongfilename(longfilename, fsector))
										{
											strcpy(filename, longfilename);
										}
										else
										{
											strcpy(filename, shortfilename);
										}
										has_lfn = 0;
									}
									else
									{
										strcpy(filename, shortfilename);
									}
									if (first_file->attribute & F_ATTR_DIRECT)
									{
										printf("Directory '%s' at 0x%08lX\n", filename, sectortobytes(fsector));
									}
									else
									{
										printf("File '%s' at 0x%08lX has %u bytes\n", filename, sectortobytes(fsector), first_file->size);
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
		printf("FAT16/FAT32 List Directory\n");
		printf("   Created by Mario Freire\n");
		printf("\n");
		printf("Usage: fatdir [image-file]\n");
		printf("Example: fatdir harddisk.img\n");
		printf("\n");
	}
	return 0;
}
