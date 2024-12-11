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
#define FAT32_BOOTSTRAP_SIZE 0x1A4
//#define FAT16_BOOTSTRAP_SIZE 0x1C0
#define FAT16_BOOTSTRAP_SIZE 0x1C

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
} fat_bpb1_t;

typedef union
{	
	struct
	{
		uint8_t drive_number;
		uint8_t reserved;
		uint8_t boot_signature;
		uint32_t volume_id;
		uint8_t volume_label[11];
		uint8_t type[8];
		uint8_t bootstrap[FAT16_BOOTSTRAP_SIZE];
	} fat16;	
	struct
	{
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
	} fat32;	
} fat_bpb2_t;

typedef struct
{
	fat_bpb1_t bpb1;
	fat_bpb2_t bpb2;
} fat_bpb_t;

typedef struct
{
	uint8_t jump_opcode;
	uint8_t jump_boot;
	uint8_t jump_boot2;
	uint8_t oem_name[8];
	fat_bpb_t bpb;
	uint8_t bootstrap[FAT32_BOOTSTRAP_SIZE];
	uint16_t signature;	
} fat_t;

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

typedef struct
{
  char name1[11];
  unsigned char attribute;
  char name2[20];
} file_lfn_entry_t;

typedef struct
{
    char path[256];
} path_t;

typedef struct
{
path_t path[32];
int pathcount;
} path_sub_t;

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
fat_t *fat;
uint8_t bootstrap[FAT32_BOOTSTRAP_SIZE];

int filesize(char *filename);
bool fileexists(char *filename);
bool initimagedisk(char *filename);
bool uninitimagedisk(void);
uint32_t sectortobytes(uint32_t sector);
bool readsector(uint32_t sector, uint8_t *buffer);
bool loadmbr(void);
bool hasactive(void);
bool isfat16type(void);
bool hasfat16lba(void);
bool isfat32type(void);
bool hasfat32lba(void);
bool isfattype(void);
bool hasfatlba(void);
bool loadfat(void);
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
uint8_t getlongfilename(char *filename, uint32_t sector);
uint32_t listdir(uint32_t sector);
uint32_t getfilefirstcluster(file_entry_t entry);
void strfilenamedot8e3s11(char *source_filename, char *destination_filename);
void strtrm(char *s1, char *s2);
char *strupr(char *str);
char *strlwr(char *str);
char *getshortfilename(char *filename11);
path_sub_t getpath(const char *p);
file_entry_t* findfileinsector(uint32_t sector, char *filename);

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
	int mbr_inactive;
	fat_t *no_mbr;
	unsigned char no_mbr_fat_type;
	if (!imagedisk_file) return false;
	if (!readsector(0, MBR)) return false;
	mbr = (mbr_t*)MBR;
	if (mbr->signature != BOOT_SIGNATURE) return false;
	active_partition = -1;
	mbr_inactive = 0;	
	no_mbr_fat_type = 0;
	no_mbr = (fat_t*)MBR;
	if (no_mbr->signature == BOOT_SIGNATURE)
	{
		if (strncmp(no_mbr->oem_name, "FASTSYS1", 8) == 0)
		{
			if (strncmp(no_mbr->bpb.bpb2.fat16.type, "FAT16   ", 8) == 0)
			{
				no_mbr_fat_type = PARTITION_FAT16_LBA;
				mbr_inactive = 1;
			}
			if (strncmp(no_mbr->bpb.bpb2.fat32.type, "FAT32   ", 8) == 0)
			{
				no_mbr_fat_type = PARTITION_FAT32_LBA;
				mbr_inactive = 1;
			}
		}
	}
	if (mbr_inactive == 0)
	{	
		for(i=0;i<4;i++)
		{
			if (mbr->partition[i].flag & PARTITION_ACTIVE)
			{
				active_partition = i;
			}
		}
	}
	else
	{
		memset(MBR, 0, SECTORSIZE);
		mbr = (mbr_t*)MBR;
		mbr->signature = BOOT_SIGNATURE;
		mbr->partition[0].flag = PARTITION_ACTIVE;
		mbr->partition[0].type = no_mbr_fat_type;
		mbr->partition[0].lba_start = 0;
		mbr->partition[0].lba_end = 0;
		active_partition = 0;
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

bool isfat16type(void)
{
	bool has_fat16;
	if (!imagedisk_file) return false;
	if (!hasactive()) return false;
	has_fat16 = false;
	switch(main_partition->type)
	{
		case PARTITION_FAT16:
		case PARTITION_FAT16_LBA:
		{
			has_fat16 = true;
		};
		break;
	}
	return has_fat16;
}

bool hasfat16lba(void)
{
	bool has_lba;
	if (!imagedisk_file) return false;
	if (!hasactive()) return false;
	has_lba = false;
	switch(main_partition->type)
	{
		case PARTITION_FAT16_LBA:
		{
			has_lba = true;
		};
		break;
	}
	return has_lba;
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

bool isfattype(void)
{
	bool has_fat;
	if (!imagedisk_file) return false;
	if (!hasactive()) return false;
	has_fat = false;
	switch(main_partition->type)
	{
		case PARTITION_FAT16:
		case PARTITION_FAT16_LBA:
		case PARTITION_FAT32:
		case PARTITION_FAT32_LBA:
		{
			has_fat = true;
		};
		break;
	}
	return has_fat;
}

bool hasfatlba(void)
{
	bool has_lba;
	if (!imagedisk_file) return false;
	if (!hasactive()) return false;
	has_lba = false;
	switch(main_partition->type)
	{
		case PARTITION_FAT16_LBA:
		case PARTITION_FAT32_LBA:
		{
			has_lba = true;
		};
		break;
	}
	return has_lba;
}

bool loadfat(void)
{
	if (!imagedisk_file) return false;
	if (!hasactive()) return false;
	if (!isfattype()) return false;	
	readsector(main_partition->lba_start, BOOT);
	fat = (fat_t*)BOOT;
	if (fat->signature != BOOT_SIGNATURE) return false;
	memcpy(bootstrap, fat->bootstrap, FAT32_BOOTSTRAP_SIZE);	
	return true;
}

uint32_t getrootdirsector(void)
{
	uint32_t start;
	uint32_t sectors;
	uint32_t root_dir;
	uint32_t fat_size;
	if (!imagedisk_file) return 0;
	if (!hasactive()) return 0;
	if (!isfattype()) return 0;
	start = main_partition->lba_start + fat->bpb.bpb1.reserved_sectors_count;
	if (isfat16type())
	{
		fat_size = fat->bpb.bpb1.fat_size_16;
	}
	else if (isfat32type())
	{
		fat_size = fat->bpb.bpb2.fat32.fat_size_32;
	}
	sectors = fat_size * fat->bpb.bpb1.number_fats;
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
	if (!isfattype()) return 0;
	bytes = fat->bpb.bpb1.bytes_per_sector;
	entries = fat->bpb.bpb1.root_entries_count;
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
	if (!isfattype()) return 0;
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
	if (!isfattype()) return 0;
	start = getdatasector();	
	if (isfat16type())
	{
		total = fat->bpb.bpb1.total_sectors_16;
	}
	else if (isfat32type())
	{
		total = fat->bpb.bpb1.total_sectors_32;
	}
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
	if (!isfattype()) return 0;
	sectors = getdatasectorcount();
	sector_per_cluster = fat->bpb.bpb1.sector_per_cluster;
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
	if (!isfattype()) return 0;
	reserved_sectors = fat->bpb.bpb1.reserved_sectors_count;
	bytes_per_sector = fat->bpb.bpb1.bytes_per_sector;
	if (isfat16type())
	{
		sector_number = (reserved_sectors + (sector * 2 / bytes_per_sector));
	}
	else if (isfat32type())
	{
		sector_number = (reserved_sectors + (sector * 4 / bytes_per_sector));
	}	
	return sector_number;
}

uint32_t getentryoffset(uint32_t sector)
{
	uint32_t offset;
	uint32_t bytes_per_sector;
	if (!imagedisk_file) return 0;
	if (!hasactive()) return 0;
	if (!isfattype()) return 0;
	bytes_per_sector = fat->bpb.bpb1.bytes_per_sector;	
	if (isfat16type())
	{
		offset = ((sector * 2) % bytes_per_sector);
	}
	else if (isfat32type())
	{
		offset = ((sector * 4) % bytes_per_sector);
	}
	return offset;
}

uint32_t getfirstsectorofcluster(uint32_t cluster)
{
	uint32_t first_sector;
	uint32_t data_sector;
	uint32_t sector_per_cluster;
	if (!imagedisk_file) return 0;
	if (!hasactive()) return 0;
	if (!isfattype()) return 0;
	sector_per_cluster = fat->bpb.bpb1.sector_per_cluster;
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
	if (!isfattype()) return 0;
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
	root_cluster = fat->bpb.bpb2.fat32.root_cluster;
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
	if (!isfattype()) return NULL;
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

void strcatb(char* s1, char* s2)
{
	int i, j;
    int rlen = strlen(s1);
    int llen = strlen(s2);
    for(i=rlen;i>=0;i--)
    {
        s1[i+llen] = s1[i];
    }
    for(j=0;j<llen;j++)
    {
        s1[j] = s2[j];
    }
}

uint8_t getlongfilename(char *filename, uint32_t sector)
{
	uint32_t filecount;
	uint32_t entrycount;
	uint32_t totalfiles;
	uint32_t fsize;
	uint32_t fcluster;
	uint32_t fsector;
	uint32_t fwhere;
	char ftype[8];
	int q;
	int d;
	int j;
	int has_lfn = 0;
	char shortfilename[12];
	char longfilename[1024];
	path_sub_t path_sub;
	file_entry_t* entries;
	file_entry_t *find_file;
	if (!imagedisk_file) return 0;
	if (!hasactive()) return 0;
	if (!isfattype()) return 0;	
	d = 0;
	q = 0;
	filecount = 0;
	entrycount = 0;
	path_sub = getpath(strupr(filename));
	strcpy(longfilename, "");
	while(q == 0)
	{
		entries = getfileentryofsector(sector+entrycount);
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
			    (entries[filecount].attribute != F_ATTR_VOLMID))
			{
				if (entries[filecount].attribute != F_ATTR_LNGFNM)
				{
					if ((entries[filecount].name[0] == '.') && (entries[filecount].name[1] == ' ') && (entries[filecount].attribute & F_ATTR_DIRECT))
					{
						d++;
					}
					if (d > 1)
					{
						q = 1;
						break;
					}
					strcpy(shortfilename, getshortfilename(entries[filecount].name));
					if (strcmp(strlwr(shortfilename), strlwr(filename)) == 0)
					{
						if ((has_lfn) && (strlen(longfilename) > 0))
						{
							strcpy(filename, longfilename);
							strcpy(longfilename, "");
							has_lfn = 0;						
							return 1;
						}
						else
						{
							strcpy(longfilename, "");
							has_lfn = 0;			
							return 0;
						}
					}
					else
					{
						strcpy(longfilename, "");
					}
				}
				else
				{
					char filename_s1[11];
					char filename_s2[20];
					char lfn_s[1024];
					file_lfn_entry_t *file_entry_lfn = (file_lfn_entry_t*)&entries[filecount];
					memcpy(filename_s1, file_entry_lfn->name1, 11);
					filename_s1[11] = '\0';
					int a = 1;
					int b = 0;
					while(a < 11)
					{
						if ((filename_s1[a] != 0x00) && ((char)filename_s1[a] != (char)0xFF))
						{
							lfn_s[b] = filename_s1[a];
							b++;
						}
						a += 2;
					}
					memcpy(filename_s2, file_entry_lfn->name2, 20);
					filename_s2[19] = '\0';
					a = 0;
					while (a < 20)
					{
						if (filename_s2[a] != 0x00)
						{
							if ((char)filename_s2[a] != (char)0xFF)
							{
								lfn_s[b] = filename_s2[a];
								b++;
							}
							else
							{
								lfn_s[b] = '\0';
								break;
							}
						}
						a += 2;
					}
					lfn_s[b] = '\0';
					strcatb(longfilename, lfn_s);
					has_lfn = 1;
				}
			}
			filecount++;
		}
		entrycount++;
	}
	return 0;	
}

uint32_t listdir(uint32_t sector)
{
	uint32_t filecount;
	uint32_t entrycount;
	uint32_t totalfiles;
	uint32_t fsize;
	uint32_t fcluster;
	uint32_t fsector;
	uint32_t fwhere;
	char ftype[8];
	int q;
	int d;
	int j;
	int has_lfn = 0;
	char shortfilename[13];
	char longfilename[1024];
	path_sub_t path_sub;
	file_entry_t* entries;
	file_entry_t *find_file;
	char filename[1024];
	if (!imagedisk_file) return 0;
	if (!hasactive()) return 0;
	if (!isfattype()) return 0;	
	d = 0;
	q = 0;
	filecount = 0;
	entrycount = 0;
	totalfiles = 0;
	path_sub = getpath(strupr(filename));
	while(q == 0)
	{
		entries = getfileentryofsector(sector+entrycount);
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
			    (entries[filecount].attribute != F_ATTR_VOLMID))
			{
				if (entries[filecount].attribute != F_ATTR_LNGFNM)
				{
					if ((entries[filecount].name[0] == '.') && (entries[filecount].name[1] == ' ') && (entries[filecount].attribute & F_ATTR_DIRECT))
					{
						d++;
					}
					if (d > 1)
					{
						q = 1;
						break;
					}
					totalfiles++;
					strcpy(shortfilename, getshortfilename(entries[filecount].name));
					strcpy(longfilename, shortfilename);
					if (has_lfn)
					{
						if (getlongfilename(longfilename, sector))
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
					fsize = entries[filecount].size;
					fcluster = getfilefirstcluster(entries[filecount]);
					fsector = getfirstsectorofcluster(fcluster);
					fwhere = sectortobytes(fsector);
										
					if (entries[filecount].attribute & F_ATTR_DIRECT)
					{
						strcpy(ftype, "<DIR>");
						printf("%-32s\t%-12s\t\t\t%-10u\t%-10u\t0x%08X\n", filename, ftype, fcluster, fsector, fwhere);
						//printf("%-12s\t%-12s\t\t\t%-10u\t%-10u\t0x%08X\n", filename, ftype, fcluster, fsector, fwhere);
					}
					else
					{
						strcpy(ftype, "");
						printf("%-32s\t%-12s\t%-10u\t%-10u\t%-10u\t0x%08X\n", filename, ftype, fsize, fcluster, fsector, fwhere);
						//printf("%-12s\t%-12s\t%-10u\t%-10u\t%-10u\t0x%08X\n", filename, ftype, fsize, fcluster, fsector, fwhere);
					}
				}
				else
				{
					has_lfn = 1;
				}
			}
			filecount++;
		}
		entrycount++;
	}
	return totalfiles;
}

file_entry_t* findfileinsector(uint32_t sector, char *filename)
{
	uint32_t filecount;
	uint32_t entrycount;
	uint32_t totalfiles;
	uint32_t fsize;
	uint32_t fcluster;
	uint32_t fsector;
	uint32_t fwhere;
	char ftype[8];
	int q;
	int d;
	int j;
	int has_lfn = 0;
	path_sub_t path_sub;
	file_entry_t* entries;
	file_entry_t* active_file;
	file_entry_t *find_file;
	char shortfilename[13];
	char longfilename[1024];
	char filename_s[1024];
	if (!imagedisk_file) return 0;
	if (!hasactive()) return 0;
	if (!isfattype()) return 0;	
	d = 0;
	q = 0;
	filecount = 0;
	entrycount = 0;
	totalfiles = 0;
	/*
	path_sub = getpath(strupr(filename));
	for(j=0;j<path_sub.pathcount;j++)
	{
		if (path_sub.path[j].path[0] != 0)
		{
			//printf("File[%d] '%s' located at 0x%08X\n", j, path_sub.path[j].path, sector);
			//return findfileinsector(sector, path_sub.path[j].path);
		}
	}
	*/
	while(q == 0)
	{
		entries = getfileentryofsector(sector+entrycount);
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
			    (entries[filecount].attribute != F_ATTR_VOLMID))
			{
				if (entries[filecount].attribute != F_ATTR_LNGFNM)
				{
					if ((entries[filecount].name[0] == '.') && (entries[filecount].name[1] == ' ') && (entries[filecount].attribute & F_ATTR_DIRECT))
					{
						d++;
					}
					if (d > 1)
					{
						q = 1;
						break;
					}
					totalfiles++;
					strcpy(shortfilename, getshortfilename(entries[filecount].name));
					strcpy(longfilename, shortfilename);
					if (has_lfn)
					{
						if (getlongfilename(longfilename, sector))
						{
							strcpy(filename_s, longfilename);
						}
						else
						{
							strcpy(filename_s, shortfilename);
						}
						has_lfn = 0;
					}
					else
					{
						strcpy(filename_s, shortfilename);
					}
					fsize = entries[filecount].size;
					fcluster = getfilefirstcluster(entries[filecount]);
					fsector = getfirstsectorofcluster(fcluster);
					fwhere = sectortobytes(fsector);
					if (strcmp(strlwr(filename), strlwr(filename_s)) == 0)
					{
						q = 1;
						active_file = (file_entry_t*)&entries[filecount];
						return active_file;
					}
				}
				else
				{
					has_lfn = 1;
				}
			}
			filecount++;
		}
		entrycount++;
	}
	return NULL;
}

uint32_t getfilefirstcluster(file_entry_t entry)
{
	uint16_t cluster_hi;
	uint16_t cluster_lo;
	uint32_t cluster;
	if (!imagedisk_file) return false;
	if (!hasactive()) return false;
	if (!isfattype()) return false;	
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


char *strupr(char *str)
{
  unsigned char *p = (unsigned char *)str;
  while (*p) {
     *p = toupper((unsigned char)*p);
      p++;
  }
  return str;
}

char *strlwr(char *str)
{
  unsigned char *p = (unsigned char *)str;
  while (*p) {
     *p = tolower((unsigned char)*p);
      p++;
  }
  return str;
}

path_sub_t getpath(const char *p)
{
    path_sub_t ps;
    int i;
    int l;
    i = 0;
    l = 0;
    ps.pathcount = 0;
    while(*p)
    {
        if (ps.pathcount >= 30)
        {
            break;
        }
        if (*p != '/')
        {
            ps.path[ps.pathcount].path[l++] = *p;
        }
        if (i > 0)
        {
            if (*p == '/')
            {
                ps.pathcount++;
                l = 0;
            }
        }
        *p++;
        i++;
    }
    ps.path[ps.pathcount].path[l++] = '\0';
    ps.pathcount++;
    ps.path[ps.pathcount].path[0] = '\0';
    return ps;
}


int main(int argc, char *argv[])
{
	file_entry_t *root_dir;
	file_entry_t *first_file;
	file_entry_t *find_file;
	uint32_t files_count;
	uint32_t root_sector;
	uint32_t sector;
	uint32_t fsector;
	uint32_t lsector;
	uint8_t is_directory = 0;
	int i;
	int j;
	int has_lfn = 0;
	bool detail_first_file;
	char* column[6] = {"Name", "Type", "Size", "Cluster", "Sector", "Position"};
	path_sub_t path_sub;
	char shortfilename[13];
	char longfilename[1024];
	char filename[1024];
	
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
							root_sector = getrootdirsector();
							if (argc >= 4)
							{
								for(int optindex=2;optindex<argc-1;optindex++)
								{
									if (strcmp(argv[optindex], "-d") == 0)
									{
										detail_first_file = true;
									}
									else if (strcmp(argv[optindex], "-s") == 0)
									{
										if (isdigit(argv[optindex+1][0]))
										{
											sector = atol(argv[optindex+1]);
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
											printf("fatdir: invalid sector: \"%c\"\n", argv[optindex+1][0]);
											sector = root_sector;
										}
									}
									else
									{
										if (argv[optindex][0] == '-')
										{
											printf("fatdir: invalid option: \"%c\"\n", argv[optindex][1]);
										}
										sector = root_sector;
									}									
								}
							}
							else
							{
								sector = root_sector;
								lsector = sector;
								if (argc == 3)
								{
									path_sub = getpath(strupr(argv[2]));
									for(j=0;j<path_sub.pathcount;j++)
									{
										if (path_sub.path[j].path[0] != 0)
										{
											find_file = findfileinsector(sector, path_sub.path[j].path);
											if (find_file != NULL)
											{
												if (find_file->attribute & F_ATTR_DIRECT)
												{
													sector = getfirstsectorofcluster(getfilefirstcluster(*find_file));
													strcpy(shortfilename, getshortfilename(find_file->name));
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
													is_directory = 1;
													printf("Directory[%d] '%s' located at 0x%08X\n", j, filename, sectortobytes(sector));
													// printf("Directory[%d] '%s' located at 0x%08X\n", j, getshortfilename(find_file->name), sectortobytes(sector));														
												}												
											}
										}
									}
									/*
									find_file = findfileinsector(sector, strupr(argv[2]));
									if (find_file != NULL)
									{
										printf("File '%s' located at 0x%08X\n", getshortfilename(find_file->name), sectortobytes(getfirstsectorofcluster(getfilefirstcluster(*find_file))));
										if (find_file->attribute & F_ATTR_DIRECT)
										{
											listdir(getfirstsectorofcluster(getfilefirstcluster(*find_file)));
										}
									}
									*/
									/*
									path_sub = getpath(strupr(argv[2]));
									for(j=0;j<path_sub.pathcount;j++)
									{
										if (path_sub.path[j].path[0] != 0)
										{
											find_file = findfileinsector(sector, path_sub.path[j].path);
											if (find_file != NULL)
											{
												printf("File[%d] '%s' located at 0x%08X\n", j, getshortfilename(find_file->name), sectortobytes(getfirstsectorofcluster(getfilefirstcluster(*find_file))));
											}
										}
									}
									*/
								}
							}
							printf("Root Address 0x%08X\n", sectortobytes(getrootdirsector()));
							printf("-------------------------------------------------------------------------------------------------------------------\n");
							//printf("------------------------------------------------------------------------------------------\n");
							printf("%-32s\t%-10s\t%-10s\t%-10s\t%-10s\t%-10s\n", column[0], column[1], column[2], column[3], column[4], column[5]);
							//printf("%-12s\t%-10s\t%-10s\t%-10s\t%-10s\t%-10s\n", column[0], column[1], column[2], column[3], column[4], column[5]);
							printf("-------------------------------------------------------------------------------------------------------------------\n");
							//printf("------------------------------------------------------------------------------------------\n");
							files_count = listdir(sector);
							printf("-------------------------------------------------------------------------------------------------------------------\n");
							//printf("------------------------------------------------------------------------------------------\n");
							printf("%-32s\t%-10s\t%-10s\t%-10s\t%-10s\t%-10s\n", column[0], column[1], column[2], column[3], column[4], column[5]);
							//printf("%-12s\t%-10s\t%-10s\t%-10s\t%-10s\t%-10s\n", column[0], column[1], column[2], column[3], column[4], column[5]);
							printf("-------------------------------------------------------------------------------------------------------------------\n");
							//printf("------------------------------------------------------------------------------------------\n");
							printf("Total: %u\n", files_count);
							if (files_count > 0)
							{
								i = 0;
								root_dir = getfileentryofsector(getrootdirsector());
								first_file = &root_dir[0];
								while((first_file->attribute == F_ATTR_LNGFNM) || (first_file->attribute == F_ATTR_VOLMID) || ((uint8_t)first_file->name[0] == FILE_NAME_DELETED))
								{
									if (first_file->attribute == F_ATTR_LNGFNM) has_lfn = 1;
									i++;
									first_file = &root_dir[i];
								}
								if (detail_first_file)
								{
									printf("------------------------------------------------------------------------------------------\n");
									fsector = getfirstsectorofcluster(getfilefirstcluster(*first_file));
									strcpy(shortfilename, getshortfilename(first_file->name));
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
										printf("Directory '%s' at 0x%08X\n", filename, sectortobytes(fsector));
									}
									else
									{
										printf("File '%s' at 0x%08X has %u bytes\n", filename, sectortobytes(fsector), first_file->size);
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
