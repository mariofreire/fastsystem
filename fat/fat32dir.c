#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define SECTORSIZE 512

#define TRUE 1
#define FALSE 0
#define true TRUE
#define false FALSE

#define PARTITION_ACTIVE 0x80
#define PARTITION_INACTIVE 0x00

#define MAX_PARTITION 4

#define BOOT_SIGNATURE 0xAA55

#define PARTITION_FAT32 0x0B
#define PARTITION_FAT32_LBA 0x0C

#define MBR_BOOTSTRAP_SIZE 0x1BE
#define FAT32_BOOTSTRAP_SIZE 0x1A4

#define FAT_ENTRY_SIZE 32

#define F_ATTR_NORMAL 0x00
#define F_ATTR_RDONLY 0x01
#define F_ATTR_HIDDEN 0x02
#define F_ATTR_SYSTEM 0x04
#define F_ATTR_VOLMID 0x08
#define F_ATTR_DIRECT 0x10
#define F_ATTR_ARCHVE 0x20
#define F_ATTR_LNGFNM 0x0F

#define FILE_NAME_DELETED 0xE5
#define FILE_NAME_DIRECTORY 0x2E

typedef unsigned char bool;

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef unsigned long long QWORD;

#ifdef	WIN32
#define PATHSEPARATOR(c) ((c) == '\\' || (c) == '/')
#else	//WIN32
#define PATHSEPARATOR(c) ((c) == '/')
#endif	//WIN32

#define UCHAR8A(value) ((BYTE)(value))
#define UCHAR8B(value) ((BYTE)((value)>> 8))
#define UCHAR8C(value) ((BYTE)((value)>>16))
#define UCHAR8D(value) ((BYTE)((value)>>24))
#define UINT16(a,b) ((DWORD)((BYTE)(a)|((BYTE)(b)<<8)))
#define UINT32(a,b,c,d) ((DWORD)((BYTE)(a)|((BYTE)(b)<<8)|((BYTE)(c)<<16)|((BYTE)(d)<<24)))
#define USHORT16(a,b) ((DWORD)(((DWORD)(a)<<16)|((WORD)(b))))

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
	
	uint32_t fat_size_32;
	uint16_t flags;
	uint16_t version;
	uint32_t root_cluster;
	uint16_t fs_info;
	uint16_t backup_boot_sector;
	uint8_t reserved_0[12];
	uint8_t drive_number;
	uint8_t reserved_1;
	uint8_t boot_signature;
	uint32_t volume_id;
	uint8_t volume_label[11];
	uint8_t type[8];
	uint8_t bootstrap[FAT32_BOOTSTRAP_SIZE];
	uint16_t signature;	
} fat32_t;

typedef struct
{
	char name[11];
	uint8_t attribute;
	uint8_t reserved;
	uint8_t creation_time_tenth;
	uint16_t creation_time;
	uint16_t creation_date;
	uint16_t last_date;
	uint16_t first_cluster_hi;
	uint16_t write_time;
	uint16_t write_date;
	uint16_t first_cluster_lo;
	uint32_t size;
} file_entry_t;

#pragma pack (pop)

FILE *imagedisk_file;
uint32_t imagedisk_size;
char imagedisk_name[1024];
uint8_t MBR[SECTORSIZE];
mbr_t *mbr;
int active_partition;
partition_entry_t* partition;
partition_entry_t *main_partition;
uint8_t BOOT[SECTORSIZE];
fat32_t *fat32;
uint8_t bootstrap[FAT32_BOOTSTRAP_SIZE];

int filesize(char *filename);
bool fileexists(char *filename);
bool initimagedisk(char *filename);
bool uninitimagedisk(void);
uint32_t sectortobytes(uint32_t sector);
bool readsector(uint32_t sector, uint8_t *buffer);
bool loadmbr(void);
bool hasactive(void);
bool isfat32type(void);
bool hasfat32lba(void);
uint32_t getrootdirsector(void);
uint32_t getrootdirsectorscount(void);
uint32_t getdatasector(void);
uint32_t getdatasectorcount(void);
uint32_t getclustercount(void);
uint32_t getsectornumber(uint32_t sector);
uint32_t getentryoffset(uint32_t sector);
uint32_t getfirstsectorofcluster(uint32_t cluster);
uint32_t getrootdirsectorstart(void);
uint32_t getrootdircluster(void);
file_entry_t* getfileentryofcluster(uint32_t cluster);
file_entry_t* getfileentryofsector(uint32_t sector);
uint32_t listrootdir(void);
void strfilenamedot8e3s11(char *source_filename, char *destination_filename);
void strtrm(char *s1, char *s2);
char *getshortfilename(char *filename11);

int filesize(char *filename)
{
	FILE *fp;
	int op, lp;	
	fp = fopen(filename, "rb");
	if (!fp) return 0;	
	op = ftell(fp);
	fseek(fp, 0, SEEK_END);
	lp = ftell(fp);
	fseek(fp, op, SEEK_SET);	
	fclose(fp);	
	return lp;
}

bool fileexists(char *filename)
{
	FILE *fp;
	fp = fopen(filename, "rb");
	if (!fp) return false;	
	fclose(fp);
	return true;
}

bool initimagedisk(char *filename)
{
	if (fileexists(filename))
	{
		imagedisk_size = filesize(filename);
		imagedisk_file = fopen(filename, "rb");
		if (!imagedisk_file) return false;
		
		if (loadmbr())
		{
			partition = (partition_entry_t*)mbr->partition;
			if (hasactive())
			{
				main_partition = (partition_entry_t*)&partition[active_partition];
			}
		}
		else
		{
			uninitimagedisk();
			return false;
		}
	}
	else
	{
		return false;
	}
	return true;
}

bool uninitimagedisk(void)
{
	if (!imagedisk_file) return false;
	fclose(imagedisk_file);
	return true;
}

uint32_t sectortobytes(uint32_t sector)
{
	return (sector * SECTORSIZE);
}

bool readsector(uint32_t sector, uint8_t *buffer)
{
	uint32_t where;
	if (!imagedisk_file) return false;
	where = sectortobytes(sector);
	if (where > (imagedisk_size-SECTORSIZE)) return false;
	fseek(imagedisk_file, where, SEEK_SET);
	if (fread(buffer, SECTORSIZE, 1, imagedisk_file) == 0) return false;
	return true;
}

bool loadmbr(void)
{
	int i;
	if (!imagedisk_file) return false;
	if (!readsector(0, MBR)) return false;
	mbr = (mbr_t*)MBR;
	if (mbr->signature != BOOT_SIGNATURE) return false;	
	active_partition = -1;	
	for(i=0;i<4;i++)
	{
		if (mbr->partition[i].flag & PARTITION_ACTIVE)
		{
			active_partition = i;
		}
	}	
	return true;
}

bool hasactive(void)
{
	if (active_partition >= MAX_PARTITION)
	{
		return false;
	}
	if (active_partition != -1)
	{
		return true;
	}
	return false;
}

bool isfat32type(void)
{
	bool has_fat32;
	if (!imagedisk_file) return false;
	if (!hasactive()) return false;
	has_fat32 = false;
	switch(main_partition->type)
	{
		case PARTITION_FAT32:
		case PARTITION_FAT32_LBA:
		{
			has_fat32 = true;
		};
		break;
	}
	return has_fat32;
}

bool hasfat32lba(void)
{
	bool has_lba;
	if (!imagedisk_file) return false;
	if (!hasactive()) return false;
	has_lba = false;
	switch(main_partition->type)
	{
		case PARTITION_FAT32_LBA:
		{
			has_lba = true;
		};
		break;
	}
	return has_lba;
}

bool loadfat32(void)
{
	if (!imagedisk_file) return false;
	if (!hasactive()) return false;
	if (!isfat32type()) return false;	
	readsector(main_partition->lba_start, BOOT);
	fat32 = (fat32_t*)BOOT;
	if (fat32->signature != BOOT_SIGNATURE) return false;
	memcpy(bootstrap, fat32->bootstrap, FAT32_BOOTSTRAP_SIZE);	
	return true;
}

uint32_t getrootdirsector(void)
{
	uint32_t start;
	uint32_t sectors;
	uint32_t root_dir;
	if (!imagedisk_file) return 0;
	if (!hasactive()) return 0;
	if (!isfat32type()) return 0;
	start = main_partition->lba_start + fat32->reserved_sectors_count;
	sectors = fat32->fat_size_32 * fat32->number_fats;
	root_dir = start + sectors;
	return root_dir;
}

uint32_t getrootdirsectorscount(void)
{
	uint32_t sectors;
	uint32_t entries;
	uint32_t bytes;
	if (!imagedisk_file) return 0;
	if (!hasactive()) return 0;
	if (!isfat32type()) return 0;
	bytes = fat32->bytes_per_sector;
	entries = fat32->root_entries_count;
	sectors = (FAT_ENTRY_SIZE * entries + bytes - 1) / bytes;
	return sectors;
}

uint32_t getdatasector(void)
{
	uint32_t start;
	uint32_t root_start;
	uint32_t root_count;
	if (!imagedisk_file) return 0;
	if (!hasactive()) return 0;
	if (!isfat32type()) return 0;
	root_start = getrootdirsector();
	root_count = getrootdirsectorscount();
	start = (root_start + root_count);
	return start;
}

uint32_t getdatasectorcount(void)
{
	uint32_t sectors;
	uint32_t start;
	uint32_t total;
	if (!imagedisk_file) return 0;
	if (!hasactive()) return 0;
	if (!isfat32type()) return 0;
	start = getdatasector();
	total = fat32->total_sectors_32;
	sectors = (total - start);
	return sectors;
}

uint32_t getclustercount(void)
{
	uint32_t clusters;
	uint32_t sectors;
	uint32_t sector_per_cluster;
	if (!imagedisk_file) return 0;
	if (!hasactive()) return 0;
	if (!isfat32type()) return 0;
	sectors = getdatasectorcount();
	sector_per_cluster = fat32->sector_per_cluster;
	clusters = (sectors / sector_per_cluster);
	return clusters;
}

uint32_t getsectornumber(uint32_t sector)
{
	uint32_t sector_number;
	uint32_t reserved_sectors;
	uint32_t bytes_per_sector;
	if (!imagedisk_file) return 0;
	if (!hasactive()) return 0;
	if (!isfat32type()) return 0;
	reserved_sectors = fat32->reserved_sectors_count;
	bytes_per_sector = fat32->bytes_per_sector;
	sector_number = (reserved_sectors + (sector * 4 / bytes_per_sector));
	return sector_number;
}

uint32_t getentryoffset(uint32_t sector)
{
	uint32_t offset;
	uint32_t bytes_per_sector;
	if (!imagedisk_file) return 0;
	if (!hasactive()) return 0;
	if (!isfat32type()) return 0;
	bytes_per_sector = fat32->bytes_per_sector;
	offset = ((sector * 4) % bytes_per_sector);
	return offset;
}

uint32_t getfirstsectorofcluster(uint32_t cluster)
{
	uint32_t first_sector;
	uint32_t data_sector;
	uint32_t sector_per_cluster;
	if (!imagedisk_file) return 0;
	if (!hasactive()) return 0;
	if (!isfat32type()) return 0;
	sector_per_cluster = fat32->sector_per_cluster;
	data_sector = getdatasector();
	first_sector = (data_sector + (cluster-2) * sector_per_cluster);
	return first_sector;
}

uint32_t getrootdirsectorstart(void)
{
	uint32_t root_cluster;
	uint32_t root_sector;
	if (!imagedisk_file) return 0;
	if (!hasactive()) return 0;
	if (!isfat32type()) return 0;
	root_cluster = getrootdircluster();
	root_sector = getfirstsectorofcluster(root_cluster);
	return root_sector;
}

uint32_t getrootdircluster(void)
{
	uint32_t root_cluster;
	if (!imagedisk_file) return 0;
	if (!hasactive()) return 0;
	if (!isfat32type()) return 0;
	root_cluster = fat32->root_cluster;
	return root_cluster;
}

file_entry_t* getfileentryofcluster(uint32_t cluster)
{
	uint32_t first_sector;
	uint32_t entryoffset;
	uint32_t filecount;
	uint8_t sector[SECTORSIZE];
	uint8_t dir_entry_data[FAT_ENTRY_SIZE];
	file_entry_t file[16];
	file_entry_t *file_p;
	file_entry_t* entry;
	if (!imagedisk_file) return NULL;
	if (!hasactive()) return NULL;
	if (!isfat32type()) return NULL;
	first_sector = getfirstsectorofcluster(cluster);
	if (!readsector(first_sector, sector)) return NULL;
	filecount = 0;
	while (filecount < 16)
	{
		entryoffset = (filecount*FAT_ENTRY_SIZE);
		memcpy(&dir_entry_data[0], &sector[entryoffset], FAT_ENTRY_SIZE);
		entry = (file_entry_t*)dir_entry_data;
		file[filecount] = *entry;
		filecount++;
	}
	file_p = &file[0];
	return file_p;
}

file_entry_t* getfileentryofsector(uint32_t sector)
{
	uint32_t entryoffset;
	uint32_t filecount;
	uint8_t first_sector[SECTORSIZE];
	uint8_t dir_entry_data[FAT_ENTRY_SIZE];
	file_entry_t file[16];
	file_entry_t *file_p;
	file_entry_t* entry;
	if (!imagedisk_file) return NULL;
	if (!hasactive()) return NULL;
	if (!isfat32type()) return NULL;
	if (!readsector(sector, first_sector)) return NULL;
	filecount = 0;
	while (filecount < 16)
	{
		entryoffset = (filecount*FAT_ENTRY_SIZE);
		memcpy(dir_entry_data, &first_sector[entryoffset], FAT_ENTRY_SIZE);
		entry = (file_entry_t*)dir_entry_data;
		file[filecount] = *entry;
		filecount++;
	}
	file_p = &file[0];
	return file_p;
}

uint32_t listrootdir(void)
{
	uint32_t root_sector;
	//uint32_t root_cluster;
	uint32_t filecount;
	uint32_t entrycount;
	uint32_t totalfiles;
	int q;
	file_entry_t* entries;
	char filename[12];
	if (!imagedisk_file) return 0;
	if (!hasactive()) return 0;
	if (!isfat32type()) return 0;	
	q = 0;
	filecount = 0;
	entrycount = 0;
	totalfiles = 0;
	//root_cluster = getrootdircluster();
	//root_sector = getfirstsectorofcluster(root_cluster);
	root_sector = getrootdirsector();
	while(q == 0)
	{
		entries = getfileentryofsector(root_sector+entrycount);
		if (entries == NULL)
		{
			q = 1;
			break;
		}
		filecount = 0;
		while (filecount < 16)
		{			
			if (entries[filecount].name[0] == 0)
			{
				q = 1;
				break;
			}
			if (((uint8_t)entries[filecount].name[0] != FILE_NAME_DELETED) && 
			    (entries[filecount].attribute != F_ATTR_LNGFNM) && 
			    (entries[filecount].attribute != F_ATTR_VOLMID))
			{
				totalfiles++;
				strcpy(filename, getshortfilename(entries[filecount].name));
				printf("%s\n", filename);
			}
			filecount++;
		}
		entrycount++;
	}
	return totalfiles;
}

uint32_t getfilefirstcluster(file_entry_t entry)
{
	uint16_t cluster_hi;
	uint16_t cluster_lo;
	uint32_t cluster;
	if (!imagedisk_file) return false;
	if (!hasactive()) return false;
	if (!isfat32type()) return false;	
	cluster_hi = entry.first_cluster_hi;
	cluster_lo = entry.first_cluster_lo;
	cluster = USHORT16(cluster_hi, cluster_lo);
	return cluster;
}

void strfilenamedot8e3s11(char *source_filename, char *destination_filename)
{
        int n = 1;
        int i = 0;
        int j = 0;
        int k = 0;
        int a = 0;
        int b = 0;
        int has_dot = 0;
        char tmpfn[12];
        char fn[12];
        memset(fn,0,12);
        memset(tmpfn,0,12);
        while (n != 0)
        {
          if (i > 10) n = 0;
          else
          {
                if (source_filename[i] != ' ')
                {
                        tmpfn[j] = source_filename[i];
                        j++;
                }
                else
                {
                        if (i < 9)
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
					    if ((tmpfn[a] != ' ') && (tmpfn[a] != 0))
						{
							fn[b] = '.';
							b++;
						}
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

char *getshortfilename(char *filename11)
{
	char filename[12];
	char filename_s[12];
	char *fn;	
	memcpy(filename_s, filename11, 12);
	filename_s[12] = '\0';
	strfilenamedot8e3s11(filename_s, filename);
	filename[12] = '\0';	
	fn = (char*)filename;	
	return fn;
}

int main(int argc, char *argv[])
{
	file_entry_t *root_dir;
	file_entry_t *first_file;
	uint32_t files_count;
	int i;
	
	if (argc > 1)
	{
		strcpy(imagedisk_name, argv[1]);
		if (fileexists(imagedisk_name))
		{
			if (initimagedisk(imagedisk_name))
			{
				if (hasactive())
				{
					if (isfat32type())
					{
						if (loadfat32())
						{
							printf("Root Address 0x%08X\n", sectortobytes(getrootdirsector()));
							printf("----------------------------------------------------------\n");
							files_count = listrootdir();
							printf("----------------------------------------------------------\n");
							printf("Total Files: %u\n", files_count);
							if (files_count > 0)
							{
								i = 0;
								root_dir = getfileentryofsector(getrootdirsector());
								first_file = &root_dir[0];
								while((first_file->attribute == F_ATTR_LNGFNM) || (first_file->attribute == F_ATTR_VOLMID) || ((uint8_t)first_file->name[0] == FILE_NAME_DELETED))
								{
									i++;
									first_file = &root_dir[i];
								}
								printf("----------------------------------------------------------\n");
								printf("File '%s' at 0x%08X has %u bytes\n", getshortfilename(first_file->name), sectortobytes(getfirstsectorofcluster(getfilefirstcluster(*first_file))), first_file->size);
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
		printf("FAT32 List Directory\n");
		printf("   Created by Mario Freire\n");
		printf("\n");
		printf("Usage: fat32dir [image-file]\n");
		printf("Example: fat32dir harddisk.img\n");
		printf("\n");
	}
	return 0;
}
