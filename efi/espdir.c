#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

#define SECTORSIZE 512

#define PARTITION_ACTIVE 0x80
#define PARTITION_INACTIVE 0x00

#define MAX_PARTITION 4

#define BOOT_SIGNATURE 0xAA55

#define PARTITION_FAT16 0x06
#define PARTITION_FAT16_LBA 0x0E
#define PARTITION_FAT32 0x0B
#define PARTITION_FAT32_LBA 0x0C

#define PARTITION_UNKNOWN 0x00
#define PARTITION_EXTENDED 0x05
#define PARTITION_EXTENDED_LBA 0x0F

#define PARTITION_GPT_PROTECTIVE 0xEE


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

#ifdef	WIN32
#define PATHSEPARATOR(c) ((c) == '\\' || (c) == '/')
#else	//WIN32
#define PATHSEPARATOR(c) ((c) == '/')
#endif	//WIN32

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef unsigned long long QWORD;

#define UCHAR8A(value) ((BYTE)(value))
#define UCHAR8B(value) ((BYTE)((value)>> 8))
#define UCHAR8C(value) ((BYTE)((value)>>16))
#define UCHAR8D(value) ((BYTE)((value)>>24))
#define UINT16(a,b) ((DWORD)((BYTE)(a)|((BYTE)(b)<<8)))
#define UINT32(a,b,c,d) ((DWORD)((BYTE)(a)|((BYTE)(b)<<8)|((BYTE)(c)<<16)|((BYTE)(d)<<24)))
#define USHORT16(a,b) ((DWORD)(((DWORD)(a)<<16)|((WORD)(b))))


char appfilename[] = "test1.exe";

#pragma pack (push, 1)

typedef struct
{
	char signature[8];
	unsigned long revision;
	unsigned long header_size;
	unsigned long crc32_header;
	unsigned long reserved;
	unsigned long long my_lba;
	unsigned long long alternate_lba;
	unsigned long long first_usable_lba;
	unsigned long long last_usable_lba;
	unsigned char disk_guid[16];
	unsigned long long partition_entry_lba;
	unsigned long num_partition_entries;
	unsigned long partition_entry_size;
	unsigned long crc32_partitions;
} gpt_header_t;

typedef struct
{
	unsigned char type_guid[16];
	unsigned char unique_guid[16];
	unsigned long long lba_start;
	unsigned long long lba_end;
	unsigned long long attributes;
	unsigned short name[36];
} gpt_entry_t;

typedef struct
{
	unsigned char head;
	unsigned char sector;
	unsigned char cylinder;
} chs_t;

typedef struct
{
	unsigned char flag;
	chs_t chs_start;
	unsigned char type;
	chs_t chs_end;
	unsigned long lba_start;
	unsigned long lba_end;
} partition_entry_t;

typedef struct
{
	unsigned char bootstrap[MBR_BOOTSTRAP_SIZE];
	partition_entry_t partition[4];
	unsigned short signature;
} mbr_t;

typedef struct
{
	unsigned short bytes_per_sector;
	unsigned char sector_per_cluster;
	unsigned short reserved_sectors_count;
	unsigned char number_fats;
	unsigned short root_entries_count;
	unsigned short total_sectors_16;
	unsigned char media;
	unsigned short fat_size_16;
	unsigned short sectors_per_track;
	unsigned short number_heads;
	unsigned long hidden_sectors;
	unsigned long total_sectors_32;
} fat_bpb1_t;

typedef union
{
	struct
	{
		unsigned char drive_number;
		unsigned char reserved;
		unsigned char boot_signature;
		unsigned long volume_id;
		unsigned char volume_label[11];
		unsigned char type[8];
		unsigned char bootstrap[FAT16_BOOTSTRAP_SIZE];
	} fat16;
	struct
	{
		unsigned long fat_size_32;
		unsigned short flags;
		unsigned short version;
		unsigned long root_cluster;
		unsigned short fs_info;
		unsigned short backup_boot_sector;
		unsigned char reserved_0[12];
		unsigned char drive_number;
		unsigned char reserved_1;
		unsigned char boot_signature;
		unsigned long volume_id;
		unsigned char volume_label[11];
		unsigned char type[8];
	} fat32;
} fat_bpb2_t;

typedef struct
{
	fat_bpb1_t bpb1;
	fat_bpb2_t bpb2;
} fat_bpb_t;

typedef struct
{
	unsigned char jump_opcode;
	unsigned char jump_boot;
	unsigned char jump_boot2;
	unsigned char oem_name[8];
	fat_bpb_t bpb;
	unsigned char bootstrap[FAT32_BOOTSTRAP_SIZE];
	unsigned short signature;
} fat_t;

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

typedef struct
{
	char name[12];
	unsigned short attribute;
	unsigned char creation_time_tenth;
	unsigned short creation_time;
	unsigned short creation_date;
	unsigned short last_date;
	unsigned short write_time;
	unsigned short write_date;
	unsigned long first_cluster;
	unsigned long size;
} file_dir_t;

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

mbr_t* mbr;

int active_partition=-1;
partition_entry_t* partition;
partition_entry_t *main_partition;

fat_t *fat;

file_entry_t file_dir_sector[16];

int mbr_loaded=0;
int esp_loaded=0;

int esp_found = 0;

gpt_entry_t* gpt_main_partition;

void remap_mbr(void);
void remap_boot(void);

int has_gpt=0;
int has_efi_support=1;

int fat_loaded=0;

gpt_header_t *gpt_header;
gpt_entry_t *gpt_entry;

unsigned char bootstrap[FAT32_BOOTSTRAP_SIZE];

unsigned char root_sector[SECTORSIZE];

unsigned char mbr_sector[SECTORSIZE];
unsigned char boot_sector[SECTORSIZE];
unsigned char disk_address_packet[SECTORSIZE];
unsigned char gpt_header_ptr[SECTORSIZE];
unsigned char gpt_entry_ptr[16384];

const uint8_t efi_system_guid[16] = {
    0x28, 0x73, 0x2A, 0xC1, 0x1F, 0xF8, 0xD2, 0x11,
    0xBA, 0x4B, 0x00, 0xA0, 0xC9, 0x3E, 0xC9, 0x3B
};

unsigned char loadmbr(void);
unsigned char loadgpt(void);
bool uninitimagedisk(void);
bool has_partition_active(void);
char *getshortfilename(char *filename11);
void strfilenamedot8e3s11(char *source_filename, char *destination_filename);
uint32_t getfilefirstcluster(file_entry_t entry);
uint32_t getrootlbaaddress(void);

unsigned long root_sector_start;


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

void print(const char* s)
{
        while(*s)
        {
            putchar(*s++);
        }
}

void printk(const char *msg, ...)
{
	int slen = strlen(msg)*4+1;
	char msgBuf[1024];
	va_list va_alist;

	if (!msg) return;

	msgBuf[slen - 1] = '\0';
	msgBuf[1024 - 1] = '\0';
	va_start(va_alist, msg);
	vsprintf(msgBuf, msg, va_alist);
	va_end(va_alist);

	print(msgBuf);
}

void error(unsigned long exception_code)
{
	printk("Error: Access violation at %s: Exception(0x%x).\n", appfilename, exception_code);
}

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
		imagedisk_file = fopen(filename, "r+b");
		if (!imagedisk_file) return false;

		if (loadmbr())
		{
			partition = (partition_entry_t*)mbr->partition;
			if ((has_efi_support == 1) && (has_gpt == 1))
			{
				if (loadgpt() == 0)
				{
					uninitimagedisk();
					return false;
				}
			}
			else
			{
				if (has_partition_active())
				{
					main_partition = (partition_entry_t*)&partition[active_partition];
				}
				else
				{
					uninitimagedisk();
					return false;
				}
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

unsigned long sectortobytes(unsigned long sector)
{
	return (sector * SECTORSIZE);
}

unsigned long bytestosector(unsigned long bytes)
{
	return (bytes / SECTORSIZE);
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

bool writesector(uint32_t sector, uint8_t *buffer)
{
	uint32_t where;
	if (!imagedisk_file) return false;
	where = sectortobytes(sector);
	if (where > (imagedisk_size-SECTORSIZE)) return false;
	fseek(imagedisk_file, where, SEEK_SET);
	if (fwrite(buffer, SECTORSIZE, 1, imagedisk_file) == 0) return false;
	return true;
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

int is_print(int c)
{
    return c >= 0x20 && c <= 0x7E;
}

void dump_hex_address(unsigned long offset, const void *data, size_t size) {
    const unsigned char *buffer = (const unsigned char *)data;
    int i, j;

    for (i = 0; i < size; i += 16) {
        printk("%08X: ", (unsigned int)offset+i);
        for (j = 0; j < 16; j++) {
            if (i + j < size)
                printk("%02X ", buffer[i + j]);
            else
                printk("   ");	
        }		
		printk(" ");			
		for (int j = 0; j < 16; j++) {
			if (i + j < size)
				printk("%c", is_print(buffer[i + j]) ? buffer[i + j] : '.');
			else
				printk(" ");
		}
        printk("\n");
    }
}

unsigned char loadmbr(void)
{
	int i;
	int mbr_inactive;
	fat_t *no_mbr;
	unsigned char no_mbr_fat_type;
	mbr_loaded = 0;
	if (!readsector(0, mbr_sector)) return 0;

	mbr = (mbr_t*)mbr_sector;
	if (mbr->signature != BOOT_SIGNATURE) return 0;

	active_partition = -1;
	mbr_inactive = 0;
	no_mbr_fat_type = 0;
	no_mbr = (fat_t*)mbr_sector;

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
		if (has_efi_support)
		{
			if (mbr->partition[0].type == PARTITION_GPT_PROTECTIVE)
			{
				has_gpt = 1;
			}
		}
	}
	else
	{
		memset(mbr_sector, 0, SECTORSIZE);
		mbr = (mbr_t*)mbr_sector;
		mbr->signature = BOOT_SIGNATURE;
		mbr->partition[0].flag = PARTITION_ACTIVE;
		mbr->partition[0].type = no_mbr_fat_type;
		mbr->partition[0].lba_start = 0;
		mbr->partition[0].lba_end = 0;
		active_partition = 0;
	}
	mbr_loaded = 1;
	return 1;
}

bool has_partition_active(void)
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

bool has_esp_partition(void)
{
	if (has_efi_support == 0) return false;
	if (has_gpt == 0)
	{
		return false;
	}
	else
	{
		if (has_gpt == 1)
		{
			if (esp_found == 1)
			{
				if (esp_loaded == 1)
				{
					return true;
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	return false;
}

void remap_mbr(void)
{
	mbr = (mbr_t*)mbr_sector;
	partition = (partition_entry_t*)mbr->partition;
	if (mbr->signature != BOOT_SIGNATURE)
	{
		error((unsigned long)mbr);
	}
	if (has_partition_active())
	{
		main_partition = (partition_entry_t*)&partition[active_partition];
	}
	else
	{
		if (has_gpt == 0)
		{
			error((unsigned long)partition);
		}
	}
}

unsigned char loadgpt(void)
{
	int i, j;
	if (mbr_loaded == 0) return 0;
	if (has_efi_support == 0) return 0;
	if (has_gpt == 0) return 0;	
	if (!readsector(1, gpt_header_ptr)) return 0;
	gpt_header = (gpt_header_t*)gpt_header_ptr;	
	if (memcmp(gpt_header->signature, "EFI PART", 8) != 0) return 0;
	unsigned long long entry_lba = gpt_header->partition_entry_lba;	
	unsigned long entry_count = gpt_header->num_partition_entries;
	unsigned long entries_size = entry_count*gpt_header->partition_entry_size;
	unsigned long entries_sectors = entries_size/SECTORSIZE;
	for(i=0;i<entries_sectors;i++)
	{
		if (!readsector(entry_lba + i, gpt_entry_ptr+(i * SECTORSIZE))) return 0;
	}
	gpt_entry = (gpt_entry_t*)&gpt_entry_ptr[0];
    for(j=0;j<entry_count;j++)
	{
		if (memcmp(gpt_entry[j].type_guid, efi_system_guid, 16) == 0) 
		{
			gpt_main_partition = (gpt_entry_t*)(&gpt_entry[j]);
			esp_found = 1;
			esp_loaded = 1;
			break;
		}
	}
	if (esp_found == 0) return 0;	
	return 1;
}

unsigned char isfat16type(void)
{
	unsigned char has_fat16;
	if (has_esp_partition())
	{
		if (fat_loaded == 1)
		{
			if (strncmp(fat->bpb.bpb2.fat16.type, "FAT16   ", 8) == 0)
			{
				has_fat16 = 1;
			}
		}
		else
		{
			has_fat16 = 0;
		}
	}
	else
	{
		if (!has_partition_active()) return 0;
		has_fat16 = 0;
		switch(main_partition->type)
		{
			case PARTITION_FAT16:
			case PARTITION_FAT16_LBA:
			{
				has_fat16 = 1;
			};
			break;
		}
	}
	return has_fat16;
}

unsigned char isfat32type(void)
{
	unsigned char has_fat32;
	if (has_esp_partition())
	{
		if (fat_loaded == 1)
		{
			if (strncmp(fat->bpb.bpb2.fat32.type, "FAT32   ", 8) == 0)
			{
				has_fat32 = 1;
			}			
		}
		else
		{
			has_fat32 = 0;
		}		
	}
	else
	{
		if (!has_partition_active()) return false;
		has_fat32 = 0;
		switch(main_partition->type)
		{
			case PARTITION_FAT32:
			case PARTITION_FAT32_LBA:
			{
				has_fat32 = 1;
			};
			break;
		}		
	}
	return has_fat32;
}

unsigned char isfattype(void)
{
	unsigned char has_fat;
	if (has_esp_partition())
	{
		if (fat_loaded == 1)
		{
			if (strncmp(fat->bpb.bpb2.fat16.type, "FAT16   ", 8) == 0)
			{
				has_fat = 1;
			}
			if (strncmp(fat->bpb.bpb2.fat32.type, "FAT32   ", 8) == 0)
			{
				has_fat = 1;
			}			
		}
		else
		{
			has_fat = 1;
		}
	}
	else
	{
		if (!has_partition_active()) return 0;
		has_fat = 0;
		switch(main_partition->type)
		{
			case PARTITION_FAT16:
			case PARTITION_FAT16_LBA:
			case PARTITION_FAT32:
			case PARTITION_FAT32_LBA:
			{
				has_fat = 1;
			};
			break;
		}
	}
	return has_fat;
}

unsigned char hasfatlba(void)
{
	unsigned char has_lba;
	if (has_esp_partition())
	{
		if (fat_loaded == 1)
		{
			if (strncmp(fat->bpb.bpb2.fat16.type, "FAT16   ", 8) == 0)
			{
				has_lba = 1;
			}
			if (strncmp(fat->bpb.bpb2.fat32.type, "FAT32   ", 8) == 0)
			{
				has_lba = 1;
			}			
		}
		else
		{
			has_lba = 0;
		}
	}
	else
	{
		if (!has_partition_active()) return 0;
		has_lba = 0;
		switch(main_partition->type)
		{
			case PARTITION_FAT16_LBA:
			case PARTITION_FAT32_LBA:
			{
				has_lba = 1;
			};
			break;
		}
	}
	return has_lba;
}

unsigned char loadfat(void)
{
	unsigned long sector;
	if (has_esp_partition())
	{
		if (!isfattype()) return 0;
		sector = gpt_main_partition->lba_start;
	}
	else
	{
		if (!has_partition_active()) return 0;
		if (!isfattype()) return 0;
		sector = main_partition->lba_start;
	}
	readsector(sector, boot_sector);
	fat = (fat_t*)boot_sector;
	if (fat->signature != BOOT_SIGNATURE) return 0;
	memcpy(bootstrap, fat->bootstrap, FAT32_BOOTSTRAP_SIZE);
	remap_boot();
	fat_loaded = 1;
	return 1;
}

void remap_fat(void)
{
	fat = (fat_t*)boot_sector;
	if (fat->signature != BOOT_SIGNATURE)
	{
		error((unsigned long)fat);
	}
}

void remap_boot(void)
{
	remap_mbr();
	remap_fat();
}

unsigned long getrootdirsector(void)
{
	unsigned long start;
	unsigned long sectors;
	unsigned long root_dir;
	unsigned long fat_size;
	if (has_esp_partition())
	{
		if (!isfattype()) return 0;		
		start = gpt_main_partition->lba_start + fat->bpb.bpb1.reserved_sectors_count;
	}
	else
	{
		if (!has_partition_active()) return 0;
		if (!isfattype()) return 0;
		start = main_partition->lba_start + fat->bpb.bpb1.reserved_sectors_count;		
	}
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
	// if (has_esp_partition()) return getrootlbaaddress();
	return root_dir;
}

unsigned long getrootdirsectorscount(void)
{
	unsigned long sectors;
	unsigned long entries;
	unsigned long bytes;
	if (has_esp_partition())
	{
		if (!isfattype()) return 0;		
	}
	else
	{
		if (!has_partition_active()) return 0;
		if (!isfattype()) return 0;
	}
	bytes = fat->bpb.bpb1.bytes_per_sector;
	entries = fat->bpb.bpb1.root_entries_count;
	sectors = (FAT_ENTRY_SIZE * entries + bytes - 1) / bytes;
	return sectors;
}

unsigned long getdatasector(void)
{
	unsigned long start;
	unsigned long root_start;
	unsigned long root_count;
	if (has_esp_partition())
	{
		if (!isfattype()) return 0;		
	}
	else
	{
		if (!has_partition_active()) return 0;
		if (!isfattype()) return 0;
	}
	root_start = getrootdirsector();
	root_count = getrootdirsectorscount();
	start = (root_start + root_count);
	return start;
}

unsigned long getfirstsectorofcluster(unsigned long cluster)
{
	unsigned long first_sector;
	unsigned long data_sector;
	unsigned long sector_per_cluster;
	remap_boot();
	if (has_esp_partition())
	{
		if (!isfattype()) return 0;		
	}
	else
	{
		if (!has_partition_active()) return 0;
		if (!isfattype()) return 0;
	}
	sector_per_cluster = fat->bpb.bpb1.sector_per_cluster;
	data_sector = getdatasector();
	first_sector = (data_sector + (cluster-2) * sector_per_cluster);
	return first_sector;
}

unsigned long getclusterstart_fat16(void)
{
        unsigned long lba_cluster;
		uint32_t lba_start;
		if (has_esp_partition())
		{
			if (!isfattype()) return 0;		
			lba_start = gpt_main_partition->lba_start;
		}
		else
		{
			if (!main_partition) return 0;
			if (!fat) return 0;
			lba_start = main_partition->lba_start;
		}
		lba_cluster = (lba_start + fat->bpb.bpb1.reserved_sectors_count) + 
					  (fat->bpb.bpb1.number_fats * fat->bpb.bpb1.fat_size_16);
		return lba_cluster;
}

unsigned long getclusterstart_fat32(void)
{
        unsigned long lba_cluster;
		uint32_t lba_start;
		if (has_esp_partition())
		{
			if (!isfattype()) return 0;		
			lba_start = gpt_main_partition->lba_start;
		}
		else
		{
			if (!main_partition) return 0;
			if (!fat) return 0;
			lba_start = main_partition->lba_start;
		}
		lba_cluster = (lba_start + fat->bpb.bpb1.reserved_sectors_count) + 
					  (fat->bpb.bpb1.number_fats * fat->bpb.bpb2.fat32.fat_size_32);
		return lba_cluster;
}

unsigned long getclusterstart(void)
{
	if (isfat16type())
	{
		return getclusterstart_fat16();
	}
	else
	{
		if (isfat32type())
		{
			return getclusterstart_fat32();
		}		
		else
		{
			return 0;
		}
	}	
}

unsigned long getrootdircluster(void)
{
	unsigned long root_cluster;
	if (!has_esp_partition())
	{
		if (!main_partition) return 0;
	}
	if (!isfat32type()) return 0;
	root_cluster = fat->bpb.bpb2.fat32.root_cluster;
	return root_cluster;
}

uint32_t getrootlbaaddress(void)
{
		uint32_t root_lba_address;
		uint32_t lba_start;
		uint32_t root_cluster;
		uint32_t cluster_start;
		remap_boot();
		if (has_esp_partition())
		{
			if (!isfattype()) return 0;		
			lba_start = gpt_main_partition->lba_start;
		}
		else
		{
			if (!main_partition) return 0;
			if (!fat) return 0;
			lba_start = main_partition->lba_start;
		}
		cluster_start = getclusterstart();
		root_cluster = getrootdircluster();
		root_lba_address = cluster_start;
		if (isfat32type()) root_lba_address += ((root_cluster-2) * fat->bpb.bpb1.sector_per_cluster);
		return root_lba_address;
}

uint32_t getrootaddress(void)
{
		uint32_t root_lba_address;
		uint32_t root_address;
		remap_boot();
		if (has_esp_partition())
		{
			if (!isfattype()) return 0;		
		}
		else
		{
			if (!main_partition) return 0;
			if (!fat) return 0;
		}
		root_lba_address = getrootlbaaddress();
		root_address = (root_lba_address * SECTORSIZE);
		return root_address;
}

file_entry_t* getfileentryofcluster(uint32_t cluster)
{
	uint32_t first_sector;
	uint32_t entryoffset;
	uint32_t filecount;
	uint8_t sector[SECTORSIZE];
	uint8_t dir_entry_data[FAT_ENTRY_SIZE];
	file_entry_t *file;
	file_entry_t *file_p;
	file_entry_t* entry;
	if (!imagedisk_file) return NULL;
	remap_boot();
	if (has_esp_partition())
	{
		if (!isfattype()) return 0;		
	}
	else
	{
		if (!main_partition) return 0;
		if (!fat) return 0;
	}
	first_sector = getfirstsectorofcluster(cluster);
	if (!readsector(first_sector, sector)) return NULL;
	filecount = 0;
	file = (file_entry_t*)malloc(sizeof(file_entry_t)*16);
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
	file_entry_t *file;
	file_entry_t *file_p;
	file_entry_t* entry;
	if (!imagedisk_file) return NULL;
	remap_boot();
	if (has_esp_partition())
	{
		if (!isfattype()) return 0;		
	}
	else
	{
		if (!main_partition) return 0;
		if (!fat) return 0;
	}
	if (!readsector(sector, first_sector)) return NULL;	
	filecount = 0;
	file = (file_entry_t*)malloc(sizeof(file_entry_t)*16);
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
	//if (!hasactive()) return 0;
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
		free(entries);
	}
	return 0;	
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
	//if (!hasactive()) return 0;
	if (!isfattype()) return 0;	
	d = 0;
	q = 0;
	filecount = 0;
	entrycount = 0;
	totalfiles = 0;
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
		free(entries);
	}
	return NULL;
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
	int has_lfn = 0;
	int q;
	file_entry_t* entries;
	char shortfilename[13];
	char longfilename[1024];
	char filename[1024];
	if (!imagedisk_file) return 0;
	remap_boot();
	if (has_esp_partition())
	{
		if (!isfattype()) return 0;		
	}
	else
	{
		if (!main_partition) return 0;
		if (!fat) return 0;
	}
	q = 0;
	filecount = 0;
	entrycount = 0;
	totalfiles = 0;
	char* column[6] = {"Name", "Type/Size", "Cluster", "Sector", "Position"};
	printk("Root Address 0x%08X\n", sectortobytes(getrootdirsector()));
	printk("---------------------------------------------------------------------------------------------------\n");
	printk("%-32s\t%-10s\t%-10s\t%-10s\t%-10s\n", column[0], column[1], column[2], column[3], column[4]);
	printk("---------------------------------------------------------------------------------------------------\n");
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
						printk("%-32s\t%-12s\t%-10u\t%-10u\t0x%08X\n", filename, ftype, fcluster, fsector, fwhere);
					}
					else
					{
						strcpy(ftype, "");
						printk("%-32s\t%-10u\t%-10u\t%-10u\t0x%08X\n", filename, fsize, fcluster, fsector, fwhere);
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
		free(entries);
	}
	return totalfiles;
}

uint32_t getfilefirstcluster(file_entry_t entry)
{
	uint16_t cluster_hi;
	uint16_t cluster_lo;
	uint32_t cluster;
	if (!imagedisk_file) return false;
	remap_boot();
	if (has_esp_partition())
	{
		if (!isfattype()) return 0;		
	}
	else
	{
		if (!main_partition) return 0;
		if (!fat) return 0;
	}
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
        fn[11] = '\0';
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
	filename_s[11] = '\0';
	strfilenamedot8e3s11(filename_s, filename);
	filename[11] = '\0';	
	fn = (char*)filename;	
	return fn;
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
				if (has_esp_partition())
				{
					printk("EFI System Partition found:\n");
					printk("  Starting LBA: %d\n", (unsigned long long)gpt_main_partition->lba_start);
					printk("  Ending LBA: %d\n", (unsigned long long)gpt_main_partition->lba_end);
					printk("  Attributes: 0x%X\n", (unsigned long long)gpt_main_partition->attributes);
					if (isfattype())
					{
						if (loadfat())
						{
							root_sector_start = getrootlbaaddress();
						}
						else
						{
							error((unsigned long)boot_sector);
						}
					}
					else
					{
						error((unsigned long)gpt_entry_ptr);
					}
				}
				else
				{
					if (has_partition_active())
					{
						if (isfattype())
						{
							if (loadfat())
							{
								root_sector_start = getrootdirsector();
							}
							else
							{
								error((unsigned long)boot_sector);
							}
						}
						else
						{
							error((unsigned long)mbr_sector);
						}
					}	
					else
					{
						error((unsigned long)mbr_sector);
					}
				}
				
				if (has_esp_partition())
				{
					printk("\n");
					if (isfat32type()) 
					{
						detail_first_file = false;
						root_sector = getrootdirsector();
						printk("FAT32 filesystem detected in EFI partition.\n");
						printk("  Root Sector: %d\n", root_sector_start);
						printk("\n");
						printk("---------------------------------------------------------------------------------------------------\n");
						printk("ROOT DIRECTORY: /\n");
						printk("---------------------------------------------------------------------------------------------------\n");
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
										printf("espdir: invalid sector: \"%c\"\n", argv[optindex+1][0]);
										sector = root_sector;
									}
								}
								else
								{
									if (argv[optindex][0] == '-')
									{
										printf("espdir: invalid option: \"%c\"\n", argv[optindex][1]);
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
												strfilenamedot8e3s11(find_file->name, shortfilename);
												//strcpy(shortfilename, getshortfilename(find_file->name));
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
											}												
										}
									}
								}								
							}
						}
						listdir(sector);
						printk("---------------------------------------------------------------------------------------------------\n");
					}
					else 
					{
						printk("Unknown filesystem in EFI partition.\n");
					}
				}
				else
				{
					printk("EFI System Partition not found.\n");
				}
				uninitimagedisk();
			}
        	else
        	{
            	printk("Error\n");
        	}
		}
	}
	else
	{
		printf("EFI System Partition List Directory\n");
		printf("   Created by Mario Freire\n");
		printf("\n");
		printf("Usage: espdir [image-file]\n");
		printf("Example: espdir harddisk.img\n");
		printf("\n");
	}
	return 0;
}

