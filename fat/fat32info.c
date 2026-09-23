#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "fat32info.h"

uint32_t imagedisk_size;
FILE * imagedisk_file;
mbr_t *mbr;
fat32_boot_sector_t *bootsector;
partition_entry_t *partition;     
partition_entry_t *main_partition;
uint8_t active_partition;
uint8_t MBR[SECTORSIZE];
uint8_t BOOTSECTOR[SECTORSIZE];
char volume_id[11];

uint8_t fileexists(char *filename)
{
	FILE *fp;
	fp = fopen(filename, "rb");
	if (!fp) return 0;
	fclose(fp);
	return 1;
}

int filesize(char *filename)
{
	FILE *fp;
	int sz,oldpos;
	fp = fopen(filename, "rb");
	if (!fp) return 0;
	oldpos = ftell(fp);
	fseek(fp, 0, SEEK_END);
	sz = ftell(fp);
	fseek(fp, oldpos, SEEK_SET);
	fclose(fp);
	return sz;
}

uint8_t initimagedisk(char *filename)
{
		imagedisk_size = filesize(filename);
		if (imagedisk_size < SECTORSIZE) return 0;
        imagedisk_file = fopen(filename, "rb");
        if (!imagedisk_file) return 0;
        loadmbr();
        partition = (partition_entry_t*)mbr->partition;
        main_partition = (partition_entry_t*)&partition[active_partition];
		return 1;
}

uint8_t uninitimagedisk(void)
{
        if (!imagedisk_file) return 0;
        fclose(imagedisk_file);
		return 1;
}

uint8_t readsector(uint32_t sector, uint8_t *buffer)
{
		uint32_t where;
        if (!imagedisk_file) return 0;
		where = (sector * SECTORSIZE);
		if (where > (imagedisk_size-SECTORSIZE)) return 0;
        fseek(imagedisk_file, where, SEEK_SET);
        fread(buffer, 1, SECTORSIZE, imagedisk_file);
		return 1;
}

uint8_t loadmbr(void)
{
		int i;
        uint8_t can_read;
		can_read = readsector(0, MBR);
		if (!can_read) return 0;
		mbr = (mbr_t*)MBR;
        for (i=0;i<4;i++)
        {
                if (mbr->partition[i].status & PARTITION_ACTIVE) active_partition = i;
        }
		return 1;
}

uint8_t loadbootsector(void)
{
        unsigned long lba_start;
        uint8_t can_read;
		if (!main_partition) return 0;
		lba_start = main_partition->lba_start;
		can_read = readsector(lba_start, BOOTSECTOR);
		if (!can_read) return 0;
        bootsector = (fat32_boot_sector_t*)BOOTSECTOR;	
		return 1;
}

uint8_t loadrootentries(void)
{
        unsigned char sector[SECTORSIZE];
        int next_dir_sector = 1;
        unsigned long addr = 0;
		uint8_t can_read;
		int next_dir_entry;
		int dir_entry_address;
		unsigned char dir_entry_data[32];
		char volume_str[11];
		char filename[12];
		char filename_s[11];
		
		if (!main_partition) return 0;
		if (!bootsector) return 0;

		addr = getrootlbaaddress();
		
		if (addr == 0) return 0;		
		 
        while (next_dir_sector != 0)
        {
                can_read = readsector(addr, sector);
				if (!can_read) return 0;
                if (sector[0] == 0x00)
                {
                 next_dir_sector = 0;
                 break;
                }
                next_dir_entry = 1;
                dir_entry_address = 0;
                while (next_dir_entry != 0)
                {
                        if (dir_entry_address >= SECTORSIZE)
                        {
                         next_dir_entry = 0;
                         break;
                        }
                        memcpy(dir_entry_data,&sector[dir_entry_address],32);
                        fat32_file_entry_t *file_entry;
                        file_entry = (fat32_file_entry_t*)dir_entry_data;
                        if (file_entry->name[0] != 0)
                        {
                                if (file_entry->attribute == 0x08) // Volume ID
                                {
                                        memcpy(volume_str, file_entry->name, 11);
                                        volume_str[11] = '\0';
                                        strtrm(volume_id, volume_str);
                                }
                                else if (file_entry->attribute != 0x0f) // if not lfn
                                {
                                       memcpy(filename_s,file_entry->name,11);
                                       filename_s[11] = '\0';
                                       strfilenamedot8e3s11(filename_s, filename);
                                }
                        }
                        else
                        {
                           next_dir_entry = 0;
                           next_dir_sector = 0;
                           break;
                        }
                        dir_entry_address += 32;
                }
                addr++;
        }
		return 1;
}

uint8_t isfat32type(void)
{
		if (!main_partition) return 0;
        switch (main_partition->type)
        {
                case PARTITION_FAT32:
                case PARTITION_FAT32_LBA:
                {
                     return 1;
                }
                break;
        }
		return 0;
}

uint8_t getpartitiontype(void)
{
        uint8_t partition_type;
		if (!main_partition) return 0;
		partition_type = main_partition->type;
		return partition_type;
}

uint32_t getpartitionsize(void)
{
        uint32_t partition_type;
		if (!main_partition) return 0;
		partition_type = (main_partition->sector_count * SECTORSIZE);
		return partition_type;
}

uint32_t getpartitionstart(void)
{
        uint32_t partition_start;
		if (!main_partition) return 0;
		partition_start = (main_partition->lba_start * SECTORSIZE);
		return partition_start;
}

uint32_t getpartitionlbastart(void)
{
        uint32_t lba_start;
		if (!main_partition) return 0;
		lba_start = main_partition->lba_start;
		return lba_start;
}

uint32_t getclusterstart(void)
{
        uint32_t lba_cluster;
		if (!main_partition) return 0;
		if (!bootsector) return 0;
		lba_cluster = (main_partition->lba_start + bootsector->reserved_sector_count) + 
					  (bootsector->table_count * bootsector->sectors_per_fat);
		return lba_cluster;
}

uint32_t getrootlbaaddress(void)
{
		uint32_t root_lba_address;
		if (!main_partition) return 0;
		if (!bootsector) return 0;
		root_lba_address = (main_partition->lba_start + bootsector->reserved_sector_count) + 
							(bootsector->table_count * bootsector->sectors_per_fat) + 
							((bootsector->root_cluster-2) * bootsector->sector_per_cluster);
		return root_lba_address;
}

uint32_t getrootaddress(void)
{
		uint32_t root_lba_address;
		uint32_t root_address;
		if (!main_partition) return 0;
		if (!bootsector) return 0;
		root_lba_address = getrootlbaaddress();
		root_address = (root_lba_address * SECTORSIZE);
		return root_address;
}

void getoemname(char *oem_name)
{
		char oem_name_s[16];
		if (!main_partition) return;
		if (!bootsector) return;
		memset(oem_name_s, 0, 8);
		memcpy(oem_name_s, bootsector->oem_name, 8);
		strtrm(oem_name, oem_name_s);		
		oem_name[8] = '\0';
}

void getvolumelabel(char *volume_label)
{
		char volume_label_s[16];
		if (!main_partition) return;
		if (!bootsector) return;
		memset(volume_label_s, 0, 11);
		memcpy(volume_label_s, bootsector->volume_label, 11);
		strtrm(volume_label, volume_label_s);
		volume_label[11] = '\0';
}

void getfattypelabel(char *fat_type_label)
{
		char fat_type_label_s[16];
		if (!main_partition) return;
		if (!bootsector) return;
		memset(fat_type_label_s, 0, 8);
		memcpy(fat_type_label_s, bootsector->fat_type_label, 8);
		strtrm(fat_type_label, fat_type_label_s);
		fat_type_label[8] = '\0';
}

void strfilenamedot8e3s11(char source_filename[11], char destination_filename[12])
{
        int n = 1;
        int i = 0;
        int j = 0;
        int k = 0;
        int a = 0;
        int b = 0;
        int has_dot = 0;
        char tmpfn[11];
        char fn[12];
        memset(fn,0,12);
        memset(tmpfn,0,11);
        while (n != 0)
        {
          if (i > 11) n = 0;
          else
          {
                if (source_filename[i] != ' ')
                {
                        tmpfn[j] = source_filename[i];
                        j++;
                }
                else
                {
                        if (i < 10)
                        {
                           if (source_filename[i+1] != ' ')
                           {
                              tmpfn[j] = '.';
                              j++;
                           }
                        }
                }
                i++;
          }
        }                 
        tmpfn[j] = '\0';
        tmpfn[11] = '\0';
        while (k < 11)
        {
                if (tmpfn[k] == '.') has_dot = 1;
                k++;
        }
        while (a < 11)
        {
                if (has_dot == 0)
                {
                  if (a == 8)
                  {
                        fn[b] = '.';
                        b++;
                  }
                  fn[b] = tmpfn[a];
                  b++;
                }
                else
                {
                  fn[b] = tmpfn[a];
                  b++;
                }
                a++;
        }
        fn[b] = '\0';
        fn[12] = '\0';
        strcpy(destination_filename,fn);
}

void strtrm(char *s1, char *s2)
{
    int i,j;
    int l;
    int n = 0;
    int e = 0;
    char c;
    j = 0;
    l = strlen(s2);
    for(i=0;i<l;i++)
    {
        if (s2[i] != ' ')
        {
            e = i;
        }
    }
    while(*s2)
    {
        c = *s2++;
        if (c != ' ')
        {
            n = 1;
        }
        if (n == 1)
        {
            if (j <= e)
            {
                *s1++ = c;
            }
        }
        j++;
    }
    *s1++ = '\0';
}

int main(int argc, char *argv[])
{
	uint8_t image_intialized;
	char imagefilename[256];
	char partition_type_name[16];
	uint32_t imagefilesize;
	uint8_t partition_type;
	uint8_t has_lba;
	uint32_t partition_size;
	uint32_t partition_start;
	uint32_t partition_lba_start;
	uint32_t cluster_start;
	uint32_t root_lba_address;
	uint32_t root_address;
	char oem_name[16];
	char volume_label[16];
	char fat_type_label[16];
	
	if (argc > 1)
	{
		strcpy(imagefilename, argv[1]);
		if (fileexists(imagefilename))
		{
			imagefilesize = filesize(imagefilename);
			active_partition = 0;
			partition_type = 0;
			has_lba = 0;
			partition_lba_start = 0;
			partition_start = 0;
			partition_size = 0;
			cluster_start = 0;
			root_lba_address = 0;
			root_address = 0;
			if (imagefilesize >= SECTORSIZE)
			{
				image_intialized = initimagedisk(imagefilename);
				if (image_intialized)
				{
					if (isfat32type())
					{
						if (loadbootsector())
						{
							root_lba_address = getrootlbaaddress();
							if (root_lba_address != 0)
							{
								if (loadrootentries())
								{
									printf("Image Disk File Name: %s\n", imagefilename);
									printf("Image Disk File Size: %u\n", imagefilesize);
									printf("Partition Entry: %u\n", active_partition);
									partition_type = getpartitiontype();							
									switch(partition_type)
									{
										case PARTITION_FAT32:
										{
											strcpy(partition_type_name, "FAT32");
											has_lba = 0;
										};
										break;
										case PARTITION_FAT32_LBA:
										{
											strcpy(partition_type_name, "FAT32");
											has_lba = 1;
										};
										break;								
									};					
									partition_size = getpartitionsize();
									partition_start = getpartitionstart();
									partition_lba_start = getpartitionlbastart();
									cluster_start = getclusterstart();
									
									root_address = getrootaddress();
									getoemname(oem_name);
									getvolumelabel(volume_label);
									getfattypelabel(fat_type_label);
									printf("Partition Type Name: %s\n", partition_type_name);
									printf("Partition Type Code: 0x%02X (%d)\n", partition_type, partition_type);
									printf("Partition Size: %u\n", partition_size);
									printf("Partition Start: %u\n", partition_start);
									printf("Partition LBA Start: 0x%02X (%d)\n", partition_lba_start, partition_lba_start);
									printf("Has LBA: %u\n", has_lba);
									printf("OEM Name: %s\n", oem_name);
									printf("Volume Label: %s\n", volume_label);
									printf("Volume ID: %s\n", volume_id);
									printf("Volume Code: 0x%X\n", bootsector->volume_id);
									printf("Root Address: 0x%X (%d)\n", root_address, root_address);
									printf("Root LBA Address: 0x%X (%d)\n", root_lba_address, root_lba_address);
									printf("FAT Type Label: %s\n", fat_type_label);
									printf("Drive Number: 0x%02X\n", bootsector->drive_number);
									printf("Root Entry Count: %u\n", bootsector->root_entry_count);
									printf("Bytes Per Sector: %u\n", bootsector->bytes_per_sector);
									printf("Sector Per Cluster: %u\n", bootsector->sector_per_cluster);
									printf("Sectors Per Track: %u\n", bootsector->sectors_per_track);
									printf("Sectors Per FAT: %u\n", bootsector->sectors_per_fat);
									printf("Table Count / Number of FAT: %u\n", bootsector->table_count);
									printf("Reserved Sectors Count %u\n", bootsector->reserved_sector_count);
									printf("Root Cluster: %u\n", bootsector->root_cluster);
									printf("Cluster Start: %u\n", cluster_start);
									printf("Boot Signature: 0x%02X (%d)\n", bootsector->boot_signature, bootsector->boot_signature);
									printf("BIOS Signature Code: 0x%02X (%d)\n", bootsector->signature, bootsector->signature);
									printf("\n");
								}
								else
								{
									printf("Cannot read root entries in image disk file '%s'.\n", imagefilename);
								}
							}
							else
							{
								printf("Cannot read root address in image disk file '%s'.\n", imagefilename);
							}
						}
						else 
						{
							printf("Cannot read boot sector in image disk file '%s'.\n", imagefilename);
						}
					}
					else
					{
						printf("Cannot find FAT32 partition in image disk file '%s'.\n", imagefilename);
					}
					uninitimagedisk();
				}
				else
				{
					printf("Error while load image disk file '%s'.\n", imagefilename);
				}
			}
			else
			{
				printf("This image disk file is not valid.\n");
			}
		}
		else
		{
			printf("Cannot find image disk file '%s'.\n", imagefilename);			
		}
	}
	else
	{
		printf("FAT32 Information\n");
		printf("   Created by Mario Freire\n");
		printf("\n");
		printf("Usage: fat32info [image-file]\n");
		printf("Example: fat32info harddisk.img\n");
		printf("\n");
	}
	return 0;
}