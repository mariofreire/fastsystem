#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define USHORT16(a,b) ((unsigned long)(((unsigned long)(a)<<16)|((unsigned short)(b))))

#pragma pack(push, 1)

typedef struct
{
	uint8_t jump_opcode;
	uint8_t jump_boot;
	uint8_t jump_boot2;
	uint8_t oem_name[8];
	uint16_t bytes_per_sector;
	uint8_t sector_per_cluster;
	uint16_t reserved_sectors_count;
	uint8_t number_fats;
	uint16_t root_entries_count;
	uint16_t total_sectors_16;
	uint8_t media;
	uint16_t fat_size_16;
	uint16_t sectors_per_track;
	uint16_t number_heads;
	uint32_t hidden_sectors;
	uint32_t total_sectors_32;
	
	uint8_t drive_number;
	uint8_t reserved;
	uint8_t boot_signature;
	uint32_t volume_id;
	uint8_t volume_label[11];
	uint8_t type[8];
	uint8_t bootstrap[0x1C0];
	uint16_t signature;	
} fat16_t;

typedef struct
{
	char name[11];
	unsigned char attribute;
	unsigned char reserved;
	unsigned char creation_time_tenth;
	unsigned short creation_time;
	unsigned short creation_date;
	unsigned short last_date;
	unsigned short first_cluster_hi;
	unsigned short write_time;
	unsigned short write_date;
	unsigned short first_cluster_lo;
	unsigned long size;
} file_entry_t;

#pragma pack(pop)


unsigned long lba_start = 0;

const char *imagefiledisk = "hdd19.img";

uint32_t getclusterstart(fat16_t *fat)
{
		uint32_t lba_cluster;
		if (!fat) return 0;
		lba_cluster = (lba_start + fat->reserved_sectors_count) + 
					  (fat->number_fats * fat->fat_size_16);
		return lba_cluster;
}

unsigned char readsector(unsigned long sector, unsigned long count, unsigned char *buffer)
{
	FILE *fp = fopen(imagefiledisk, "rb");
	if (!fp) return 0;	
	fseek(fp, (512*sector), SEEK_SET);
	fread(buffer, 512*count, 1, fp);	
	fclose(fp);	
	return 1;
}

unsigned long getclustercount2(fat16_t *bootsector) 
{
    unsigned long reservedSectors = bootsector->reserved_sectors_count;
    unsigned long bytesPerSector = bootsector->bytes_per_sector;
    unsigned long sectorsPerCluster = bootsector->sector_per_cluster;
	unsigned long totalSectors = bootsector->total_sectors_16;

    unsigned long fatSize = totalSectors - reservedSectors;
    unsigned long clusterCount = fatSize / sectorsPerCluster;

    return clusterCount;
}

unsigned long getrootdirsector(fat16_t *fat)
{
	unsigned long start;
	unsigned long sectors;
	unsigned long root_dir;
	unsigned long fat_size;
	if (!fat) return 0;
	start = lba_start + fat->reserved_sectors_count;
	fat_size = fat->fat_size_16;
	sectors = fat_size * fat->number_fats;
	root_dir = start + sectors;
	return root_dir;
}

unsigned long getrootdirsectorscount(fat16_t *fat)
{
	unsigned long sectors;
	unsigned long entries;
	unsigned long bytes;
	if (!fat) return 0;
	bytes = fat->bytes_per_sector;
	entries = fat->root_entries_count;
	sectors = (32 * entries + bytes - 1) / bytes;
	return sectors;
}

unsigned long getdatasector(fat16_t *fat)
{
	unsigned long start;
	unsigned long root_start;
	unsigned long root_count;
	if (!fat) return 0;
	root_start = getrootdirsector(fat);
	root_count = getrootdirsectorscount(fat);
	start = (root_start + root_count);
	return start;
}


unsigned long getdatasectorcount(fat16_t *fat)
{
	unsigned long sectors;
	unsigned long start;
	unsigned long total;
	if (!fat) return 0;
	start = getdatasector(fat);
	/*
	if (isfat16type())
	{
		total = fat->bpb.bpb1.total_sectors_16;
	}
	else if (isfat32type())
	{
		total = fat->bpb.bpb1.total_sectors_32;
	}
	*/
	total = fat->total_sectors_16;
	sectors = (total - start);
	return sectors;
}



unsigned long getclustercount(fat16_t *fat)
{
	unsigned long clusters;
	unsigned long sectors;
	unsigned long sector_per_cluster;
	if (!fat) return 0;
	sectors = getdatasectorcount(fat);
	sector_per_cluster = fat->sector_per_cluster;
	clusters = (sectors / sector_per_cluster);
	return clusters;
}

unsigned long getsectornumber(fat16_t *fat, unsigned long sector)
{
	unsigned long sector_number;
	unsigned long reserved_sectors;
	unsigned long bytes_per_sector;
	if (!fat) return 0;
	reserved_sectors = fat->reserved_sectors_count;
	bytes_per_sector = fat->bytes_per_sector;
	sector_number = (reserved_sectors + (sector * 2 / bytes_per_sector));
	return sector_number;
}

unsigned long getentryoffset(fat16_t *fat, unsigned long sector)
{
	unsigned long offset;
	unsigned long bytes_per_sector;
	if (!fat) return 0;
	bytes_per_sector = fat->bytes_per_sector;
	offset = ((sector * 2) % bytes_per_sector);
	return offset;
}


unsigned long getfirstsectorofcluster(fat16_t *fat, unsigned long cluster)
{
	unsigned long first_sector;
	unsigned long data_sector;
	unsigned long sector_per_cluster;
	if (!fat) return 0;
	sector_per_cluster = fat->sector_per_cluster;
	data_sector = getdatasector(fat);
	first_sector = (data_sector + (cluster-2) * sector_per_cluster);
	return first_sector;
}

unsigned long getrootdircluster(fat16_t *fat)
{
	unsigned long root_cluster;
	if (!fat) return 0;
	root_cluster = 0;
	return root_cluster;
}

unsigned long getrootdirsectorstart(fat16_t *fat)
{
	unsigned long root_cluster;
	unsigned long root_sector_start;
	if (!fat) return 0;
	root_cluster = getrootdircluster(fat);
	root_sector_start = getfirstsectorofcluster(fat, root_cluster);
	return root_sector_start;
}

unsigned long filesizeondisk(unsigned long size)
{
	unsigned long i,szd=0;
	for(i=0;i<=size;i+=512)
	{
		szd += 512;
	}
	return szd;
}

unsigned long getfilefirstcluster(file_entry_t entry)
{
	unsigned short cluster_hi;
	unsigned short cluster_lo;
	unsigned long cluster;
	cluster_hi = entry.first_cluster_hi;
	cluster_lo = entry.first_cluster_lo;
	cluster = USHORT16(cluster_hi, cluster_lo);
	return cluster;
}

int _fileexists(const char *filename)
{
	FILE *fp;
	fp = fopen(filename, "rb");
	if (!fp) return 0;
	fclose(fp);
	return 1;
}

int _filesize(const char *filename)
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

unsigned long getfilesize(fat16_t *fat, unsigned long sector, unsigned long bytes_per_sector)
{
	int total_size = 0;
	int filesize = 0;
	int sectcount = 0;
	int sector_count = 0;
	int offset = 0;
	int fsector = 0;
	int fcluster = 0;
	char *data = (char*)malloc(bytes_per_sector);
	char filename[32];
	int q = 0;
	int max_size = _filesize(imagefiledisk) / bytes_per_sector;
	while (q == 0)
	{
		sector_count = sector+sectcount;
		if (sector_count >= max_size)
		{
			q = 1;
			break;
		}
		if (!readsector(sector_count, 1, data))
		{
			q = 1;
			break;
		}
		if (data == NULL)
		{
			q = 1;
			break;			
		}
		if (data[0] == 0) 
		{
			q = 1;
			break;
		}
		else
		{
			offset = 0;
			while(offset < bytes_per_sector)
			{
				void *filep = (void*)data+offset;
				file_entry_t *file = (file_entry_t*)filep;
				if (file == NULL)
				{
					q = 1;
					break;
				}
				if (file->name[0] == 0)
				{
					q = 1;
					break;					
				}
				if (((file->attribute != 0x0F) && (file->attribute != 0x08)) && 
				    (file->name[0] != 0xE5) && (file->name[0] != 0x2E) && 
					(file->name[0] != 0x00) && (file->name[0] != 0xFF))
				{
					memset(filename, 0, 32);
					memcpy(filename, file->name, 11);
					if (filename[0] == 0)
					{
						q = 1;
						break;						
					}
					filesize = file->size;
					total_size += filesize;
					fcluster = getfilefirstcluster(*file);
					fsector = getfirstsectorofcluster(fat, fcluster);	
					if (fsector >= max_size)
					{
						q = 1;
						break;
					}
					//printf("0x%08X\n", (int)fcluster);
					if (fcluster == 0xFFFF)
					{
						q = 1;
						break;
					}
					if (file->attribute == 0x10)
					{
						total_size += getfilesize(fat, fsector, bytes_per_sector);
					}
					//printf("%s\t%d\n", filename, (int)filesize);					
				}
				//printf("offset: %d\n", (int)offset);
				offset += 32;
			}
		}
		//printf("sector: %d\n", (int)sectcount);
		sectcount++;
		sector_count = sector+sectcount;
		if (sector_count >= max_size)
		{
			q = 1;
			break;
		}
	}	
	free(data);
	return total_size;
}

int main() {
	char *buffer;
	char bootsector[512];
	fat16_t *fat;
	if (!readsector(lba_start, 1, bootsector)) return 0;
	
	fat = (fat16_t*)bootsector;

    uint32_t datastart = getclusterstart(fat);
    uint32_t bytes_per_sector = fat->bytes_per_sector;
    uint32_t reserved_sector_count = fat->reserved_sectors_count;
    uint32_t total_sectors = fat->total_sectors_16;
	if (total_sectors == 0) total_sectors = fat->total_sectors_32;
    uint32_t number_of_fat = fat->number_fats;
    uint32_t sector_per_track = fat->sectors_per_track;
	
	uint32_t a = (datastart*bytes_per_sector);
    uint32_t b = (total_sectors*bytes_per_sector)-a;
    uint32_t c = (sector_per_track/number_of_fat)*bytes_per_sector;

	uint32_t d = b+c;
	uint32_t f = 0;
	
	uint32_t rootdir = (datastart);
	
   
    uint32_t cluster_count = getclustercount(fat);
	
	buffer = (char*)malloc(512*cluster_count);
	if (!readsector(lba_start+1, 128, buffer)) 
	{
		if (buffer) free(buffer);
		return 0;
	}
	
	uint32_t free_cluster = 0;
	
	for(int i=0;i<128*512;i+=2)
	{
		if ((buffer[i] == 0) && (buffer[i+1] == 0))
		{
			free_cluster++;
		}
	}
	unsigned long x = ((lba_start*512)-free_cluster)-reserved_sector_count;
	free_cluster -= x;
	f = free_cluster;
	f *= 512;
	free(buffer);
	
    /*
    int totalSectors = 819756;
    int sectorsPerFat = 201;
    int sectorsPerCluster = 16;
    int reservedSectorsCount = 1;
    int bytesPerSector = 512;
    
    int totalSize = totalSectors * bytesPerSector;
    
    int freeSpace = totalSize - (sectorsPerFat * bytesPerSector);
    */
	int total_size = getfilesize(fat, rootdir, bytes_per_sector);
	//total_size /= 10;
    //printf("\n");
	//f = (((total_sectors-rootdir)+((lba_start/2))) * bytes_per_sector) - filesizeondisk(total_size);
	//f /= 10;
	f = (total_sectors * bytes_per_sector) - filesizeondisk(total_size);
	free_cluster = f / bytes_per_sector;
	int total_space = total_sectors*bytes_per_sector;
	
    printf("Root Address %u\n", datastart);
    printf("Free Cluster %u\n", free_cluster);
    printf("Cluster Count %u\n", cluster_count);
    printf("Total sectors %u\n", total_sectors);
    printf("Total space has value %u bytes.\n", total_space);
    printf("Free space has value %u bytes.\n", f);
    printf("Used space has value %u bytes.\n", total_size);
    
    // Read sectors in img
    
    return 0;
}
