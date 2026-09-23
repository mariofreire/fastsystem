#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <limits.h>
#include "fatlib.h"


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
    const char *image_name;
    const char *file_name;
    const char *final_name;
    char *new_path;
    file_entry_t *f;
    file_entry_t *entry;
    uint32_t first_cluster;
    path_sub_t path;
    uint32_t root_sector;
    uint32_t directory_sector;
    found_file_t existing;
    uint8_t dos_name[11];
    char fn[MAX_FILENAME_LENGTH];
    bool is_file = false;
    bool file_exists_on_disk = false;
    uint16_t utf16_name[LFN_MAX_CHARS];
    size_t utf16_length;
    size_t lfn_count;
    size_t directory_entries_needed;    
	char* column[6] = {"Name", "Type", "Size", "Cluster", "Sector", "Position"};

    if (argc < 2)
    {
        printf("FAT16/FAT32 Viewer\n");
        printf("   Created by Mario Freire\n");
        printf("\n");
        printf("Usage: fatview [image-file] [file-name-optional]\n");
        printf("Example: fatview harddisk.img\n");
        printf("         fatview harddisk.img loader\n");
        printf("\n");
        return 1;
    }

    image_name = argv[1];

    if (argc == 3) 
    {
    	memset(fn, 0, sizeof(fn));
    	strcpy(fn, argv[2]);
    	if (strcmp(fn, "/") == 0)
    	{
    		strcpy(fn, "/.");
    	}
    	file_name = (char*)fn;
    }
    else file_name = "/.";
    
    if (!initimagedisk(image_name))
    {
        fprintf(stderr, "Error: Cannot initialize disk image.\n");
        return 1;
    }

    if (!hasactive())
    {
        fprintf(stderr, "Error: No active FAT partition found.\n");
        uninitimagedisk();
        return 1;
    }

    if (!isfattype())
    {
        fprintf(stderr, "Error: Partition is not FAT16/FAT32.\n");
        uninitimagedisk();
        return 1;
    }

    if (!loadfat())
    {
        fprintf(stderr, "Error: Invalid FAT boot sector.\n");
        uninitimagedisk();
        return 1;
    }
    
    root_sector = getrootdirsectorstart();

    path = getpath(file_name);

    if (path.pathcount == 0)
    {
        fprintf(stderr, "Error: Invalid destination filename.\n");
        uninitimagedisk();
        return 1;
    }

    final_name = path.path[path.pathcount - 1].path;

    utf16_length = utf8_to_utf16(final_name, utf16_name, LFN_MAX_CHARS);

    if (utf16_length == 0)
    {
        fprintf(stderr, "Error: Invalid UTF-8 filename.\n");
        uninitimagedisk();
        return 1;
    }

    if (utf16_length > LFN_MAX_CHARS)
    {
        fprintf(stderr, "Error: Filename is longer than 255 UTF-16 characters.\n");
        uninitimagedisk();
        return 1;
    }

    lfn_count = lfn_entry_count(utf16_length);

    if (lfn_count == 0 || lfn_count > LFN_MAX_ENTRIES)
    {
        fprintf(stderr, "Error: Invalid LFN length.\n");
        uninitimagedisk();
        return 1;
    }

    directory_entries_needed = lfn_count + 1;

    strfilenamedos(final_name, dos_name);

    first_cluster = 0;
    if (path.pathcount == 1)
    {
        directory_sector = root_sector;
    }
    
	entry = findfile(root_sector, file_name);
	if (entry != NULL)
	{
		file_exists_on_disk = true;
		first_cluster = getfilefirstcluster(entry);
		if (entry->attribute & F_ATTR_DIRECT) 
		{
			is_file = false;
			directory_sector = clustertosector(first_cluster);
			if (isfat16type()) 
			{
				if (first_cluster == 0) directory_sector = root_sector;
			}						
		}
		else 
		{
			is_file = true;					
    		if (first_cluster != 0)
    		{
    			directory_sector = clustertosector(first_cluster);								
        	}
		}
	} else 
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
        		uint32_t file_size = getfilesize(file_name, directory_sector);
        		uint32_t file_size_on_disk = getfilesizeondisk(file_name, directory_sector);
        		uint8_t *buffer = (uint8_t*)malloc(file_size);
        		if (buffer != 0)
        		{
        			if (getfiledata(file_name, directory_sector, buffer))
        			{
        				printf("File: %s\n", file_name);
        				printf("Size: %d bytes\n", file_size);
        				printf("Size on disk: %d bytes\n", file_size_on_disk);
    					printf("-------------------------------------------------------------------------------------------------------------------\n");
            			dump_hex(buffer, file_size);
    					printf("-------------------------------------------------------------------------------------------------------------------\n");
    					printf("File: %s\n", file_name);
        				printf("Size: %d bytes\n", file_size);
        				printf("Size on disk: %d bytes\n", file_size_on_disk);
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
		else
		{
    		if (directory_sector == 0)
    		{
        		fprintf(stderr, "Error: Invalid destination directory sector.\n");
        		uninitimagedisk();
        		return 1;
    		}	
    		if (argc == 3)
    		{
    			if (directory_sector != root_sector)
    			{
    				printf("Path Address 0x%08lX\n", sectortobytes(directory_sector));
    			}
    		}
    		printf("Root Address 0x%08lX\n", sectortobytes(root_sector));
    		printf("-------------------------------------------------------------------------------------------------------------------\n");
    		printf("%-32s\t%-10s\t%-10s\t%-10s\t%-10s\t%-10s\n", column[0], column[1], column[2], column[3], column[4], column[5]);
    		printf("-------------------------------------------------------------------------------------------------------------------\n");
    		int totalfiles = listdir(directory_sector);
    		printf("-------------------------------------------------------------------------------------------------------------------\n");
    		printf("%-32s\t%-10s\t%-10s\t%-10s\t%-10s\t%-10s\n", column[0], column[1], column[2], column[3], column[4], column[5]);
    		printf("-------------------------------------------------------------------------------------------------------------------\n");
    		printf("Total: %u\n", totalfiles);
    	}
    }
    else
    {
		fprintf(stderr, "Error: %s: No such file or directory.\n", file_name);
		uninitimagedisk();
		return 1;
    }

    uninitimagedisk();

    return 0;
}
