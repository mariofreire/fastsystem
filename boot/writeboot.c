#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#define SECTORSIZE 512

#define TRUE 1
#define FALSE 0
#define true TRUE
#define false FALSE

#define PARTITION_ACTIVE 0x80
#define PARTITION_INACTIVE 0x00

#define MAX_PARTITION 4

#define BOOT_SIGNATURE 0xAA55

#define PARTITION_FAT16 0x06
#define PARTITION_FAT16_LBA 0x0E
#define PARTITION_FAT32 0x0B
#define PARTITION_FAT32_LBA 0x0C

#define MBR_BOOTSTRAP_SIZE 0x1BE
#define FAT32_BOOTSTRAP_BPB_START 0x0B
#define FAT32_BOOTSTRAP_BPB_END 0x47
#define FAT16_BOOTSTRAP_BPB_START 0x0B
#define FAT16_BOOTSTRAP_BPB_END 0x2B

#define FAT32_BOOTSTRAP_BPB_SIZE 0x3C
#define FAT32_BOOTSTRAP_OUT_BPB_SIZE 0x1B9
#define FAT16_BOOTSTRAP_BPB_SIZE 0x20
#define FAT16_BOOTSTRAP_OUT_BPB_SIZE 0x1D5


#define UINT16(a,b) ((unsigned long)((unsigned char)(a)|((unsigned char)(b)<<8)))

#pragma pack (push, 1)

typedef struct
{
	uint8_t head;
	uint8_t sector;
	uint8_t cylinder;
} chs_t;

typedef struct
{
	uint8_t flag;
	chs_t chs_start;
	uint8_t type;
	chs_t chs_end;	
	uint32_t lba_start;
	uint32_t lba_end;
} partition_entry_t;

typedef struct
{
	uint8_t bootstrap[MBR_BOOTSTRAP_SIZE];
	partition_entry_t partition[4];
	uint16_t signature;
} mbr_t;

#pragma pack (pop)

uint8_t MBR[SECTORSIZE];
mbr_t *mbr;
partition_entry_t* partition;
partition_entry_t *main_partition;
int active_partition;

uint32_t lba_start;
uint32_t bootsector_start;

uint8_t show_offset_dumphex;

int fileexists(char *filename)
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

int readsector(char *filename, uint32_t sector, uint8_t *buffer)
{
	FILE *fp;
	uint32_t offset;
	uint32_t size;
	size = filesize(filename);
	offset = (sector * SECTORSIZE);
	if (offset > (size-SECTORSIZE)) return 0;
	fp = fopen(filename, "rb");
	if (!fp) return 0;
	fseek(fp, offset, SEEK_SET);
	if (!fread(buffer, SECTORSIZE, 1, fp))
	{
		fclose(fp);
		return 0;
	}
	fclose(fp);
	return 1;
}

int writesector(char *filename, uint32_t sector, uint8_t *buffer)
{
	FILE *fp;
	uint32_t offset;
	uint32_t size;
	size = filesize(filename);
	offset = (sector * SECTORSIZE);
	if (offset > (size-SECTORSIZE)) return 0;
	fp = fopen(filename, "r+b");
	if (!fp) return 0;
	fseek(fp, offset, SEEK_SET);
	if (!fwrite(buffer, SECTORSIZE, 1, fp))
	{
		fclose(fp);
		return 0;
	}
	fclose(fp);
	return 1;
}

void dumphex(uint8_t *buffer, int size, uint8_t show_offset, uint32_t offset_start)
{
	int i,j;
	i = 0;
	j = 0;
	while (i < size)
	{
		if (j == 16)
		{
			printf("\n");
			if (show_offset)
			{
				printf("%08X: ", offset_start+i);
			}
			j = 0;
		}
		else if (j == 0)
		{
			if (show_offset)
			{
				printf("%08X: ", offset_start+i);
			}
		}
		printf("%02X ", buffer[i]);
		i++;
		j++;
	}
	printf("\n");
}

int updateboot(char *imagefilename, char *bootfilename)
{
	FILE *image_w_fp;
	int boot_sz = filesize(bootfilename);
	int image_sz = filesize(imagefilename);
	uint8_t bootsector[SECTORSIZE];
	uint8_t imagebootsector[SECTORSIZE];
	int i,j,k;
	int has_error;
	int sector_start,sector_end;
	unsigned short bootsignature;
	lba_start = 0;
	bootsector_start = 0;
	has_error = 0;
	active_partition = -1;
	if (image_sz <= SECTORSIZE)
	{
		return 0;
	}
	if (boot_sz != SECTORSIZE)
	{
		return 0;
	}
	else
	{
		if (!readsector(bootfilename, 0, bootsector)) return 0;
		if (!readsector(imagefilename, 0, MBR)) return 0;
		mbr = (mbr_t*)MBR;
		partition = (partition_entry_t*)mbr->partition;
		if (mbr->signature != BOOT_SIGNATURE) return 0;
		bootsignature = UINT16(bootsector[SECTORSIZE-2], bootsector[SECTORSIZE-1]);
		if (bootsignature != BOOT_SIGNATURE) return 0;
		for(i=0;i<4;i++)
		{
			if (partition[i].flag & PARTITION_ACTIVE)
			{
				active_partition = i;
			}
		}
		if (active_partition != -1)
		{
			main_partition = (partition_entry_t*)&partition[active_partition];
			if (((main_partition->type == PARTITION_FAT32) || (main_partition->type == PARTITION_FAT32_LBA)) 
			|| ((main_partition->type == PARTITION_FAT16) || (main_partition->type == PARTITION_FAT16_LBA)))
			{
				lba_start = main_partition->lba_start;
				if (lba_start == 0)
				{
					has_error++;
				}
			}
			else
			{
				has_error++;	
			}
		}
		else
		{
			has_error++;
		}
		if (has_error > 0)
		{
			return 0;
		}
		else
		{
			bootsector_start = (lba_start*SECTORSIZE);
			if (!readsector(imagefilename, lba_start, imagebootsector)) return 0;			
			if (show_offset_dumphex == 1)
			{
				printf("Boot Sector in the boot sector file: %s\n", bootfilename);
				dumphex(bootsector, SECTORSIZE, 1, 0);
				printf("\n");
				printf("Boot Sector in the image disk file: %s\n", imagefilename);
			}			
			if ((main_partition->type == PARTITION_FAT32) || (main_partition->type == PARTITION_FAT32_LBA))
			{
				sector_start = FAT32_BOOTSTRAP_BPB_START;
				sector_end = FAT32_BOOTSTRAP_BPB_END;
			}
			else
			{
				sector_start = FAT16_BOOTSTRAP_BPB_START;
				sector_end = FAT16_BOOTSTRAP_BPB_END;				
			}
			for(j=0;j<sector_start;j++)
			{
				imagebootsector[j] = bootsector[j];
			}
			for(k=sector_end;k<(SECTORSIZE-2);k++)
			{
				imagebootsector[k] = bootsector[k];
			}			
			if (show_offset_dumphex == 1)
			{
				dumphex(imagebootsector, SECTORSIZE, 1, bootsector_start);
				printf("\n");
			}
			if (!writesector(imagefilename, lba_start, imagebootsector)) return 0;
		}
	}
	return 1;
}

int main(int argc, char *argv[])
{
	char imagefilename[256];
	char bootfilename[256];
	if (argc > 2)
	{
		strcpy(imagefilename, argv[1]);
		strcpy(bootfilename, argv[2]);
		if (argc == 4)
		{
			if ((strcmp(argv[3], "--dumphex") == 0) || (strcmp(argv[3], "--dumphex") == 0))
			{
				show_offset_dumphex = 1;
			}
			else
			{
				show_offset_dumphex = 0;
			}
		}
		else
		{
			show_offset_dumphex = 0;
		}
		if (fileexists(imagefilename))
		{
			if (fileexists(bootfilename))
			{
				if (updateboot(imagefilename, bootfilename))
				{
					printf("Writted %d bytes in boot sector of the image file.\n", filesize(bootfilename));
					printf("Recorded Sector successfully.\n");
				}
				else
				{
					printf("Cannot write boot sector in the image file '%s'.\n", imagefilename);
				}
			}
			else
			{
				printf("Cannot find boot file '%s'.\n", bootfilename);
			}
		}
		else
		{
			printf("Cannot find image file '%s'.\n", imagefilename);			
		}
	}
	else
	{
		printf("Write Boot Sector File To Boot Sector\n");
		printf("   Created by Mario Freire\n");
		printf("\n");
		printf("Usage: writeboot [image-file] [boot-sector-file] [option]\n");
		printf("Example: writeboot harddisk.img boot.bin\n");
		printf("         writeboot harddisk.img boot.bin --dumphex\n");
		printf("\n");
	}
	return 0;
}