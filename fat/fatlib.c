#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <limits.h>
#include "fatlib.h"

#define HIGH16(a) ((unsigned short)(((a)>>16)&0xFFFF))
#define LOW16(a) ((unsigned short)((a)&0xFFFF))

FILE *imagedisk_file = NULL;
uint64_t imagedisk_size = 0;
uint8_t MBR[SECTORSIZE];
uint8_t BOOT[SECTORSIZE];
mbr_t *mbr = NULL;
fat_t *fat = NULL;
int active_partition = -1;
partition_entry_t *partition = NULL;
partition_entry_t *main_partition = NULL;

char *strupr1(const char *s)
{
	size_t len;
	char *dest = (char*)s;
	char *s1;
	const char c = ('a' - 'A');
	if (s == NULL)
	{
		return 0;
	}
	len = strlen(s);
	if (len > 0)
	{
		for (s1=(char*)s;*s1;s1++)
		{
			*s1 = (((*s1 >= 'a') && (*s1 <= 'z')) ? (*s1 - c) : *s1);
		}
	}
	return dest;
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

const char *basename1(const char *path)
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

uint64_t filesize(const char *filename)
{
    FILE *fp;
    off_t size;

    fp = fopen(filename, "rb");
    if (fp == NULL)
        return 0;

    if (fseeko(fp, 0, SEEK_END) != 0)
    {
        fclose(fp);
        return 0;
    }

    size = ftello(fp);
    fclose(fp);

    if (size < 0)
        return 0;

    return (uint64_t)size;
}

bool fileexists(const char *filename)
{
    FILE *fp = fopen(filename, "rb");

    if (fp == NULL)
        return false;

    fclose(fp);
    return true;
}

uint64_t sectortobytes(uint32_t sector)
{
    return (uint64_t)sector * SECTORSIZE;
}

uint32_t bytestosector(uint64_t bytes)
{
    return (uint32_t)(bytes / SECTORSIZE);
}

uint64_t filesizeondisk(uint64_t size)
{
	uint64_t i,szd=0;
	for(i=0;i<=size;i+=512)
	{
		szd += 512;
	}
	return (uint64_t)szd;
}

bool readsector(uint32_t sector, uint8_t *buffer)
{
    uint64_t offset;

    if (imagedisk_file == NULL || buffer == NULL)
        return false;

    offset = sectortobytes(sector);

    if (offset > imagedisk_size)
        return false;

    if (SECTORSIZE > imagedisk_size - offset)
        return false;

    if (fseeko(imagedisk_file, (off_t)offset, SEEK_SET) != 0)
        return false;

    return fread(buffer, 1, SECTORSIZE, imagedisk_file) == SECTORSIZE;
}

bool writesector(uint32_t sector, const uint8_t *buffer)
{
    uint64_t offset;

    if (imagedisk_file == NULL || buffer == NULL)
        return false;

    offset = sectortobytes(sector);

    if (offset > imagedisk_size)
        return false;

    if (SECTORSIZE > imagedisk_size - offset)
        return false;

    if (fseeko(imagedisk_file, (off_t)offset, SEEK_SET) != 0)
        return false;

    if (fwrite(buffer, 1, SECTORSIZE, imagedisk_file) != SECTORSIZE)
        return false;

    if (fflush(imagedisk_file) != 0)
        return false;

    return true;
}

bool loadmbr(void)
{
    int i;

    if (imagedisk_file == NULL)
        return false;

    if (!readsector(0, MBR))
        return false;

    mbr = (mbr_t *)MBR;

    if (mbr->signature != BOOT_SIGNATURE)
        return false;

    active_partition = -1;

    for (i = 0; i < MAX_PARTITION; ++i)
    {
        if (mbr->partition[i].flag == PARTITION_ACTIVE)
        {
            if (active_partition == -1)
                active_partition = i;
        }
    }

    if (active_partition == -1)
    {
        fat_t *boot = (fat_t *)MBR;
        bool is_fat16 = false;
        bool is_fat32 = false;

        if (memcmp(boot->oem_name, "FASTSYS1", 8) == 0)
        {
            is_fat16 = memcmp(boot->bpb.bpb2.fat16.type, "FAT16   ", 8) == 0;
            is_fat32 = memcmp(boot->bpb.bpb2.fat32.type, "FAT32   ", 8) == 0;

            memset(MBR, 0, sizeof(MBR));
            mbr = (mbr_t *)MBR;
            mbr->signature = BOOT_SIGNATURE;
            mbr->partition[0].flag = PARTITION_ACTIVE;
            mbr->partition[0].lba_start = 0;

            if (is_fat16)
            {
                mbr->partition[0].type = PARTITION_FAT16_LBA;
                active_partition = 0;
            }
            else if (is_fat32)
            {
                mbr->partition[0].type = PARTITION_FAT32_LBA;
                active_partition = 0;
            }
        }
    }

    return true;
}

bool initimagedisk(const char *filename)
{
    if (!fileexists(filename))
        return false;

    imagedisk_size = filesize(filename);

    if (imagedisk_size < SECTORSIZE)
        return false;

    imagedisk_file = fopen(filename, "r+b");

    if (imagedisk_file == NULL)
        return false;

    if (!loadmbr())
    {
        fclose(imagedisk_file);
        imagedisk_file = NULL;
        return false;
    }

    partition = mbr->partition;

    if (!hasactive())
    {
        fclose(imagedisk_file);
        imagedisk_file = NULL;
        return false;
    }

    main_partition = &partition[active_partition];

    if (!haspartition())
    {
        fclose(imagedisk_file);
        imagedisk_file = NULL;
        return false;
    }

    return true;
}

bool uninitimagedisk(void)
{
    bool result = true;

    if (imagedisk_file != NULL)
    {
        if (fclose(imagedisk_file) != 0)
            result = false;

        imagedisk_file = NULL;
    }

    mbr = NULL;
    fat = NULL;
    partition = NULL;
    main_partition = NULL;
    active_partition = -1;

    return result;
}

bool hasactive(void)
{
    return active_partition >= 0 && active_partition < MAX_PARTITION;
}

bool haspartition(void)
{
    return main_partition != NULL;
}

bool isfat16type(void)
{
    if (!hasactive())
        return false;

    switch (main_partition->type)
    {
        case PARTITION_FAT16_LESS_32MB:
        case PARTITION_FAT16:
        case PARTITION_FAT16_LBA:
            return true;
        default:
            return false;
    }
}

bool isfat32type(void)
{
    if (!hasactive())
        return false;

    switch (main_partition->type)
    {
        case PARTITION_FAT32:
        case PARTITION_FAT32_LBA:
            return true;
        default:
            return false;
    }
}

bool hasfat16lba(void)
{
    return hasactive() && main_partition->type == PARTITION_FAT16_LBA;
}

bool hasfat32lba(void)
{
    return hasactive() && main_partition->type == PARTITION_FAT32_LBA;
}

bool isfattype(void)
{
    return isfat16type() || isfat32type();
}

bool hasfatlba(void)
{
    return hasfat16lba() || hasfat32lba();
}

bool loadfat(void)
{
    if (imagedisk_file == NULL)
        return false;

    if (!hasactive() || !isfattype())
        return false;

    if (!readsector(main_partition->lba_start, BOOT))
        return false;

    fat = (fat_t *)BOOT;

    if (fat->signature != BOOT_SIGNATURE)
        return false;

    if (fat->bpb.bpb1.bytes_per_sector != SECTORSIZE)
        return false;

    if (fat->bpb.bpb1.sector_per_cluster == 0)
        return false;

    if (fat->bpb.bpb1.number_fats == 0)
        return false;

    if (isfat16type() && fat->bpb.bpb1.fat_size_16 == 0)
        return false;

    if (isfat32type() && fat->bpb.bpb2.fat32.fat_size_32 == 0)
        return false;

    return true;
}

uint32_t getfatsize(void)
{
    if (!fat)
        return 0;

    if (isfat16type())
        return fat->bpb.bpb1.fat_size_16;

    if (isfat32type())
        return fat->bpb.bpb2.fat32.fat_size_32;

    return 0;
}

uint32_t getrootdirsector(void)
{
    uint32_t fat_start;
    uint32_t fat_size;
    uint32_t root_dir;

    if (!imagedisk_file || !hasactive() || !isfattype())
        return 0;

    fat_start = main_partition->lba_start + fat->bpb.bpb1.reserved_sectors_count;
    fat_size = getfatsize();
    root_dir = fat_start + ((uint32_t)fat->bpb.bpb1.number_fats * fat_size);

    return root_dir;
}

uint32_t getrootdirsectorscount(void)
{
    uint32_t entries;
    uint32_t bytes;

    if (!imagedisk_file || !hasactive() || !isfat16type())
        return 0;

    bytes = fat->bpb.bpb1.bytes_per_sector;
    entries = fat->bpb.bpb1.root_entries_count;

    return ((uint32_t)FAT_ENTRY_SIZE * entries + bytes - 1) / bytes;
}

uint32_t getfirstdatasector(void)
{
    uint32_t fat_size;
    uint32_t first_data;

    if (!imagedisk_file || !hasactive() || !isfattype())
        return 0;

    fat_size = getfatsize();

    first_data = main_partition->lba_start + fat->bpb.bpb1.reserved_sectors_count + ((uint32_t)fat->bpb.bpb1.number_fats * fat_size);

    if (isfat16type())
        first_data += getrootdirsectorscount();

    return first_data;
}

uint32_t getdatasector(void)
{
    return getfirstdatasector();
}

uint32_t getdatasectorcount(void)
{
    uint32_t total;
    uint32_t first_data_relative;

    if (!imagedisk_file || !hasactive() || !isfattype())
        return 0;

    total = fat->bpb.bpb1.total_sectors_16;

    if (total == 0)
        total = fat->bpb.bpb1.total_sectors_32;

    first_data_relative = fat->bpb.bpb1.reserved_sectors_count + ((uint32_t)fat->bpb.bpb1.number_fats * getfatsize());

    if (isfat16type())
        first_data_relative += getrootdirsectorscount();

    if (total <= first_data_relative)
        return 0;

    return total - first_data_relative;
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

uint32_t getnextcluster(uint32_t cluster)
{
    uint8_t sector[SECTORSIZE];
    uint32_t fat_sector;
    uint32_t offset;

	if (!imagedisk_file) return 0;
	if (!hasactive()) return 0;
	if (!isfattype()) return 0;

    fat_sector = main_partition->lba_start + getsectornumber(cluster);
    offset = getentryoffset(cluster);

    if (!readsector(fat_sector, sector))
        return 0;

    if (isfat16type())
    {
        return (*(uint16_t *)(sector + offset));
    }
    else
    {
        return (*(uint32_t *)(sector + offset)) & 0x0FFFFFFF;
    }
}

bool islastcluster(uint32_t cluster)
{
	if (!imagedisk_file) return 0;
	if (!hasactive()) return 0;
	if (!isfattype()) return 0;

    if (isfat16type())
        return cluster >= 0xFFF8;

    return cluster >= 0x0FFFFFF8;
}

uint32_t getclustercount(void)
{
    uint32_t sectors;
    uint32_t sectors_per_cluster;

    if (!imagedisk_file || !hasactive() || !isfattype())
        return 0;

    sectors = getdatasectorcount();
    sectors_per_cluster = fat->bpb.bpb1.sector_per_cluster;

    if (sectors_per_cluster == 0)
        return 0;

    return sectors / sectors_per_cluster;
}

uint32_t clustertosector(uint32_t cluster)
{
    uint32_t first_data;
    uint32_t sectors_per_cluster;

    if (!imagedisk_file || !hasactive() || !isfattype())
        return 0;

    if (cluster < 2)
        return 0;

    first_data = getfirstdatasector();
    sectors_per_cluster = fat->bpb.bpb1.sector_per_cluster;

    return first_data + (cluster - 2U) * sectors_per_cluster;
}

uint32_t getclusterfromsector(uint32_t sector)
{
    uint32_t first_data;
    uint32_t relative_sector;
    uint32_t sectors_per_cluster;

    if (!imagedisk_file || !hasactive() || !isfattype())
        return 0;

    first_data = getfirstdatasector();

    if (sector < first_data)
        return 0;

    relative_sector = sector - first_data;
    sectors_per_cluster = fat->bpb.bpb1.sector_per_cluster;

    if (sectors_per_cluster == 0)
        return 0;

    return (relative_sector / sectors_per_cluster) + 2U;
}

uint32_t getfirstsectorofcluster(uint32_t cluster)
{
    return clustertosector(cluster);
}

uint32_t getrootdircluster(void)
{
    if (!imagedisk_file || !hasactive() || !isfat32type())
        return 0;

    return fat->bpb.bpb2.fat32.root_cluster;
}

uint32_t getrootdirsectorstart(void)
{
    if (!imagedisk_file || !hasactive() || !isfattype())
        return 0;

    if (isfat16type())
        return getrootdirsector();

    if (isfat32type())
        return clustertosector(getrootdircluster());

    return 0;
}

uint32_t getfatentrysize(void)
{
    if (isfat16type())
        return 2;

    if (isfat32type())
        return 4;

    return 0;
}

uint32_t getfatsector(uint32_t cluster)
{
    uint32_t entry_size;
    uint32_t entries_per_sector;

    if (!imagedisk_file || !hasactive() || !isfattype())
        return 0;

    entry_size = getfatentrysize();

    if (entry_size == 0)
        return 0;

    entries_per_sector = fat->bpb.bpb1.bytes_per_sector / entry_size;

    if (entries_per_sector == 0)
        return 0;

    return main_partition->lba_start + fat->bpb.bpb1.reserved_sectors_count + (cluster / entries_per_sector);
}

uint32_t getfatentryoffset(uint32_t cluster)
{
    uint32_t entry_size;
    uint32_t entries_per_sector;

    if (!imagedisk_file || !hasactive() || !isfattype())
        return 0;

    entry_size = getfatentrysize();

    if (entry_size == 0)
        return 0;

    entries_per_sector = fat->bpb.bpb1.bytes_per_sector / entry_size;

    if (entries_per_sector == 0)
        return 0;

    return (cluster % entries_per_sector) * entry_size;
}

file_entry_t* getfileentryofcluster(uint32_t cluster)
{
    static file_entry_t file[16];
    
	uint32_t first_sector;
	uint32_t entryoffset;
	uint32_t filecount;
	uint8_t sector[SECTORSIZE];
	uint8_t dir_entry_data[FAT_ENTRY_SIZE];
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

	return file;
}

file_entry_t* getfileentryofsector(uint32_t sector)
{
    static file_entry_t file[16];

    uint32_t entryoffset;
    uint32_t filecount;
    uint8_t first_sector[SECTORSIZE];
    uint8_t dir_entry_data[FAT_ENTRY_SIZE];
    file_entry_t *entry;

    if (!imagedisk_file) return NULL;
    if (!hasactive()) return NULL;
    if (!isfattype()) return NULL;
    if (!readsector(sector, first_sector)) return NULL;

    for (filecount = 0; filecount < 16; filecount++)
    {
        entryoffset = filecount * FAT_ENTRY_SIZE;
        memcpy(dir_entry_data, &first_sector[entryoffset], FAT_ENTRY_SIZE);
        entry = (file_entry_t *)dir_entry_data;
        file[filecount] = *entry;
    }

    return file;
}

bool getfileentriesofsector(uint32_t sector, file_entry_t entries[16])
{
    uint8_t buffer[SECTORSIZE];

    if (!readsector(sector, buffer))
        return false;

    memcpy(entries, buffer, sizeof(file_entry_t) * 16);

    return true;
}

unsigned long readcluster(unsigned long cluster)
{
    uint8_t buffer[SECTORSIZE];
    uint32_t sector;
    uint32_t offset;

    if (!imagedisk_file || !hasactive() || !isfattype())
        return 0;

    if (cluster < 2)
        return 0;

    sector = getfatsector((uint32_t)cluster);
    offset = getfatentryoffset((uint32_t)cluster);

    if (sector == 0 || offset >= SECTORSIZE)
        return 0;

    if (!readsector(sector, buffer))
        return 0;

    if (isfat16type())
    {
        uint16_t value;

        if (offset + sizeof(value) > SECTORSIZE)
            return 0;

        memcpy(&value, buffer + offset, sizeof(value));
        return (unsigned long)(value & FAT16_CHAIN_MASK);
    }

    if (isfat32type())
    {
        uint32_t value;

        if (offset + sizeof(value) > SECTORSIZE)
            return 0;

        memcpy(&value, buffer + offset, sizeof(value));
        return (unsigned long)(value & FAT32_CHAIN_MASK);
    }

    return 0;
}

bool writecluster(unsigned long cluster, unsigned long value)
{
    uint32_t fat_size;
    uint32_t entry_size;
    uint32_t sector;
    uint32_t offset;
    uint32_t i;

    if (!imagedisk_file || !hasactive() || !isfattype())
        return false;

    if (cluster < 2)
        return false;

    entry_size = getfatentrysize();
    if (entry_size == 0)
        return false;

    fat_size = getfatsize();
    if (fat_size == 0)
        return false;

    if (isfat16type())
        value &= FAT16_CHAIN_MASK;
    else
        value &= FAT32_CHAIN_MASK;

    sector = getfatsector((uint32_t)cluster);
    offset = getfatentryoffset((uint32_t)cluster);

    if (sector == 0 || offset + entry_size > SECTORSIZE)
        return false;

    for (i = 0; i < fat->bpb.bpb1.number_fats; ++i)
    {
        uint32_t fat_sector = sector + i * fat_size;
        uint8_t buffer[SECTORSIZE];

        if (!readsector(fat_sector, buffer))
            return false;

        if (isfat16type())
        {
            uint16_t v = (uint16_t)value;
            memcpy(buffer + offset, &v, sizeof(v));
        }
        else
        {
            uint32_t old_value;
            uint32_t new_value;

            memcpy(&old_value, buffer + offset, sizeof(old_value));
            new_value = (old_value & 0xF0000000U) | ((uint32_t)value & FAT32_CHAIN_MASK);
            memcpy(buffer + offset, &new_value, sizeof(new_value));
        }

        if (!writesector(fat_sector, buffer))
            return false;
    }

    return true;
}

uint32_t fat_eoc_mark(void)
{
    if (isfat16type())
        return FAT16_EOC_MARK;

    if (isfat32type())
        return FAT32_EOC_MARK;

    return 0;
}

uint32_t fat_max_cluster(void)
{
    uint32_t cluster_count = getclustercount();

    if (cluster_count == 0)
        return 0;

    if (cluster_count == UINT32_MAX)
        return UINT32_MAX;

    return cluster_count + 1U;
}

bool is_valid_data_cluster(uint32_t cluster)
{
    uint32_t max_cluster = fat_max_cluster();
    return cluster >= 2 && cluster <= max_cluster;
}

bool is_eoc(uint32_t cluster)
{
    uint32_t eoc = fat_eoc_mark();

    if (eoc == 0)
        return false;

    return cluster >= eoc;
}

unsigned long neededcluster(uint64_t size)
{
    uint64_t cluster_size;
    uint64_t count;
    uint32_t sectors_per_cluster;
    uint32_t bytes_per_sector;
    uint32_t max_clusters;

    if (!imagedisk_file || !hasactive() || !isfattype())
        return 0;

    sectors_per_cluster = fat->bpb.bpb1.sector_per_cluster;
    bytes_per_sector = fat->bpb.bpb1.bytes_per_sector;

    if (sectors_per_cluster == 0 || bytes_per_sector == 0)
        return 0;

    cluster_size = (uint64_t)sectors_per_cluster * bytes_per_sector;

    if (cluster_size == 0 || size == 0)
        return 0;

    count = (size + cluster_size - 1) / cluster_size;
    max_clusters = getclustercount();

    if (max_clusters == 0 || count > max_clusters || count > UINT32_MAX)
        return 0;

    return (unsigned long)count;
}

bool freechain(uint32_t first_cluster)
{
    uint32_t cluster;
    uint32_t max_cluster;
    uint32_t iterations = 0;

    if (!imagedisk_file || !hasactive() || !isfattype())
        return false;

    if (first_cluster == 0)
        return true;

    if (!is_valid_data_cluster(first_cluster))
        return false;

    max_cluster = fat_max_cluster();
    cluster = first_cluster;

    while (cluster >= 2 && cluster <= max_cluster)
    {
        uint32_t next;

        ++iterations;

        if (iterations > max_cluster)
            return false;

        next = (uint32_t)readcluster(cluster);

        if (!writecluster(cluster, 0))
            return false;

        if (next == 0 || is_eoc(next))
            return true;

        if (next < 2 || next > max_cluster)
            return false;

        cluster = next;
    }

    return false;
}

unsigned long allocchain(unsigned long count)
{
    uint32_t first = 0;
    uint32_t previous = 0;
    uint32_t cluster;
    uint32_t max_cluster;
    uint32_t eoc_mark;
    uint32_t found = 0;

    if (!imagedisk_file || !hasactive() || !isfattype() || count == 0)
        return 0;

    max_cluster = fat_max_cluster();
    eoc_mark = fat_eoc_mark();

    if (max_cluster < 2 || eoc_mark == 0)
        return 0;

    if ((uint64_t)count > getclustercount())
        return 0;

    for (cluster = 2; cluster <= max_cluster && found < count; ++cluster)
    {
        uint32_t value = (uint32_t)readcluster(cluster);

        if (value != 0)
            continue;

        if (!writecluster(cluster, eoc_mark))
        {
            if (first != 0)
                freechain(first);
            return 0;
        }

        if (first == 0)
            first = cluster;

        if (previous != 0)
        {
            if (!writecluster(previous, cluster))
            {
                freechain(first);
                return 0;
            }
        }

        previous = cluster;
        ++found;
    }

    if (found != count)
    {
        if (first != 0)
            freechain(first);
        return 0;
    }

    return first;
}

uint32_t chain_length(uint32_t first_cluster)
{
    uint32_t cluster;
    uint32_t count = 0;
    uint32_t max_cluster;

    if (first_cluster == 0 || !is_valid_data_cluster(first_cluster))
        return 0;

    max_cluster = fat_max_cluster();
    cluster = first_cluster;

    while (cluster >= 2 && cluster <= max_cluster)
    {
        uint32_t next;

        ++count;

        if (count > max_cluster)
            return 0;

        next = (uint32_t)readcluster(cluster);

        if (next == 0)
            return 0;

        if (is_eoc(next))
            return count;

        if (next < 2 || next > max_cluster)
            return 0;

        cluster = next;
    }

    return 0;
}

uint32_t get_nth_cluster(uint32_t first_cluster, uint32_t index)
{
    uint32_t cluster;
    uint32_t i;
    uint32_t max_cluster;

    if (first_cluster < 2 || !is_valid_data_cluster(first_cluster))
        return 0;

    max_cluster = fat_max_cluster();
    cluster = first_cluster;

    for (i = 0; i < index; ++i)
    {
        uint32_t next;

        if (cluster < 2 || cluster > max_cluster)
            return 0;

        next = (uint32_t)readcluster(cluster);

        if (next < 2 || is_eoc(next))
            return 0;

        cluster = next;
    }

    return cluster;
}

bool resizechain(uint32_t *first_cluster, uint32_t required)
{
    uint32_t current_count;
    uint32_t cluster;
    uint32_t next;
    uint32_t extra;

    if (!first_cluster)
        return false;

    if (required == 0)
    {
        if (*first_cluster != 0)
        {
            if (!freechain(*first_cluster))
                return false;

            *first_cluster = 0;
        }

        return true;
    }

    if (*first_cluster == 0)
    {
        *first_cluster = (uint32_t)allocchain(required);
        return *first_cluster != 0;
    }

    current_count = chain_length(*first_cluster);

    if (current_count == 0)
        return false;

    if (current_count == required)
        return true;

    if (current_count > required)
    {
        cluster = get_nth_cluster(*first_cluster, required - 1);

        if (cluster < 2)
            return false;

        next = (uint32_t)readcluster(cluster);

        if (next >= 2 && !is_eoc(next))
        {
            if (!writecluster(cluster, fat_eoc_mark()))
                return false;

            if (!freechain(next))
                return false;
        }

        return true;
    }

    extra = required - current_count;
    cluster = get_nth_cluster(*first_cluster, current_count - 1);

    if (cluster < 2)
        return false;

    next = (uint32_t)allocchain(extra);

    if (next == 0)
        return false;

    if (!writecluster(cluster, next))
    {
        freechain(next);
        return false;
    }

    return true;
}

uint32_t getfilefirstcluster(const file_entry_t *entry)
{
    uint32_t cluster;

    if (!entry)
        return 0;

    cluster = entry->first_cluster_lo;

    if (isfat32type())
        cluster |= ((uint32_t)entry->first_cluster_hi << 16);

    return cluster;
}

void strfilenamedos(const char *source, uint8_t destination[11])
{
    size_t i;
    size_t pos = 0;
    bool extension = false;

    memset(destination, ' ', 11);

    if (!source)
        return;

    for (i = 0; source[i] != '\0'; ++i)
    {
        unsigned char c = (unsigned char)source[i];

        if (c == '.')
        {
            if (!extension)
            {
                extension = true;
                pos = 8;
            }

            continue;
        }

        if (c >= 0x80)
            c = '_';
        else
            c = (unsigned char)toupper(c);

        if (!((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || strchr("_^$~!#%&-{}()@'`", c) != NULL))
            c = '_';

        if (pos < 11)
            destination[pos++] = c;
    }
}

bool getshortfilename(const char *name11, char *output, size_t output_size)
{
    size_t i;
    size_t pos = 0;

    if (!name11 || !output || output_size == 0)
        return false;

    for (i = 0; i < 8; ++i)
    {
        if (name11[i] != ' ')
        {
            if (pos + 1 >= output_size)
                return false;

            output[pos++] = name11[i];
        }
    }

    while (pos > 0 && output[pos - 1] == ' ')
        --pos;

    if (name11[8] != ' ')
    {
        if (pos + 1 >= output_size)
            return false;

        output[pos++] = '.';

        for (i = 8; i < 11; ++i)
        {
            if (name11[i] != ' ')
            {
                if (pos + 1 >= output_size)
                    return false;

                output[pos++] = name11[i];
            }
        }
    }

    output[pos] = '\0';
    return true;
}

bool names_equal(const char *a, const char *b)
{
    while (*a && *b)
    {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b))
            return false;

        ++a;
        ++b;
    }

    return *a == '\0' && *b == '\0';
}

uint8_t lfn_checksum(const uint8_t *name)
{
    uint8_t sum = 0;
    int i;

    for (i = 0; i < 11; ++i)
        sum = (uint8_t)(((sum & 1) ? 0x80 : 0) + (sum >> 1) + name[i]);

    return sum;
}

size_t utf8_to_utf16(const char *src, uint16_t *dst, size_t max_units)
{
    size_t i = 0;
    size_t out = 0;

    if (!src || !dst || max_units == 0)
        return 0;

    while (src[i] != '\0')
    {
        uint32_t cp;
        unsigned char c = (unsigned char)src[i];

        if (c < 0x80)
        {
            cp = c;
            i++;
        }
        else if ((c & 0xE0) == 0xC0)
        {
            unsigned char c1 = (unsigned char)src[i + 1];

            if (c1 == '\0' || (c1 & 0xC0) != 0x80)
                return 0;

            cp = ((uint32_t)(c & 0x1F) << 6) | (c1 & 0x3F);

            if (cp < 0x80)
                return 0;

            i += 2;
        }
        else if ((c & 0xF0) == 0xE0)
        {
            unsigned char c1 = (unsigned char)src[i + 1];
            unsigned char c2 = (unsigned char)src[i + 2];

            if (c1 == '\0' || c2 == '\0')
                return 0;

            if ((c1 & 0xC0) != 0x80 || (c2 & 0xC0) != 0x80)
                return 0;

            cp = ((uint32_t)(c & 0x0F) << 12) | ((uint32_t)(c1 & 0x3F) << 6) | (c2 & 0x3F);

            if (cp < 0x800 || (cp >= 0xD800 && cp <= 0xDFFF))
                return 0;

            i += 3;
        }
        else if ((c & 0xF8) == 0xF0)
        {
            unsigned char c1 = (unsigned char)src[i + 1];
            unsigned char c2 = (unsigned char)src[i + 2];
            unsigned char c3 = (unsigned char)src[i + 3];

            if (c1 == '\0' || c2 == '\0' || c3 == '\0')
                return 0;

            if ((c1 & 0xC0) != 0x80 || (c2 & 0xC0) != 0x80 || (c3 & 0xC0) != 0x80)
                return 0;

            cp = ((uint32_t)(c & 0x07) << 18) | ((uint32_t)(c1 & 0x3F) << 12) | ((uint32_t)(c2 & 0x3F) << 6) | (c3 & 0x3F);

            if (cp < 0x10000 || cp > 0x10FFFF)
                return 0;

            i += 4;
        }
        else
        {
            return 0;
        }

        if (cp <= 0xFFFF)
        {
            if (out >= max_units)
                return 0;

            dst[out++] = (uint16_t)cp;
        }
        else
        {
            uint32_t v = cp - 0x10000;

            if (out + 2 > max_units)
                return 0;

            dst[out++] = (uint16_t)(0xD800 + (v >> 10));
            dst[out++] = (uint16_t)(0xDC00 + (v & 0x3FF));
        }
    }

    return out;
}

size_t lfn_entry_count(size_t utf16_length)
{
    if (utf16_length == 0)
        return 0;

    return (utf16_length + 12) / 13;
}

bool utf16_to_utf8(const uint16_t *src, size_t count, char *output, size_t output_size)
{
    size_t i;
    size_t pos = 0;

    if (!src || !output || output_size == 0)
        return false;

    for (i = 0; i < count; ++i)
    {
        uint32_t cp = src[i];

        if (cp == 0x0000 || cp == 0xFFFF)
            break;

        if (cp >= 0xD800 && cp <= 0xDBFF)
        {
            uint32_t low;

            if (i + 1 >= count)
                return false;

            low = src[++i];

            if (low < 0xDC00 || low > 0xDFFF)
                return false;

            cp = 0x10000 + (((cp - 0xD800) << 10) | (low - 0xDC00));

            if (pos + 4 >= output_size)
                return false;

            output[pos++] = (char)(0xF0 | (cp >> 18));
            output[pos++] = (char)(0x80 | ((cp >> 12) & 0x3F));
            output[pos++] = (char)(0x80 | ((cp >> 6) & 0x3F));
            output[pos++] = (char)(0x80 | (cp & 0x3F));
        }
        else if (cp < 0x80)
        {
            if (pos + 1 >= output_size)
                return false;

            output[pos++] = (char)cp;
        }
        else if (cp < 0x800)
        {
            if (pos + 2 >= output_size)
                return false;

            output[pos++] = (char)(0xC0 | (cp >> 6));
            output[pos++] = (char)(0x80 | (cp & 0x3F));
        }
        else
        {
            if (pos + 3 >= output_size)
                return false;

            output[pos++] = (char)(0xE0 | (cp >> 12));
            output[pos++] = (char)(0x80 | ((cp >> 6) & 0x3F));
            output[pos++] = (char)(0x80 | (cp & 0x3F));
        }
    }

    output[pos] = '\0';
    return pos != 0;
}

bool get_lfn_name(const uint8_t *raw_entries, size_t entry_count, char *output, size_t output_size)
{
    uint16_t chars[LFN_MAX_CHARS];
    size_t char_count = 0;
    size_t i;
    uint8_t max_order = 0;

    if (!raw_entries || !output || output_size == 0)
        return false;

    memset(chars, 0xFF, sizeof(chars));

    for (i = 0; i < entry_count; ++i)
    {
        lfn_entry_t entry;
        uint8_t order;
        size_t base;

        memcpy(&entry, raw_entries + i * FAT_ENTRY_SIZE, sizeof(entry));

        if (entry.attribute != F_ATTR_LNGFNM)
            return false;

        order = entry.order & 0x1F;

        if (order == 0 || order > LFN_MAX_ENTRIES)
            return false;

        if (entry.order & LFN_LAST)
            max_order = order;

        base = (size_t)(order - 1) * 13;

        memcpy(chars + base, entry.name1, sizeof(entry.name1));
        memcpy(chars + base + 5, entry.name2, sizeof(entry.name2));
        memcpy(chars + base + 11, entry.name3, sizeof(entry.name3));
    }

    if (max_order == 0 || max_order > entry_count)
        return false;

    char_count = (size_t)max_order * 13;

    if (char_count > LFN_MAX_CHARS)
        char_count = LFN_MAX_CHARS;

    return utf16_to_utf8(chars, char_count, output, output_size);
}

bool findfileinsectorfilenumber(uint32_t directory_start, const char *filename, found_file_t *result)
{
    uint32_t current_cluster = 0;
    bool is_root_fat16 = false;

    if (!filename || !result)
        return false;

    memset(result, 0, sizeof(*result));

    if (isfat16type() && directory_start == getrootdirsector())
    {
        is_root_fat16 = true;
    }
    else
    {
        current_cluster = getclusterfromsector(directory_start);

        if (isfat32type() && directory_start == getrootdirsectorstart())
            current_cluster = getrootdircluster();

        if (current_cluster < 2)
            return false;
    }

    for (;;)
    {
        uint32_t first_sector;
        uint32_t sector_count;
        uint32_t sector_index;

        if (is_root_fat16)
        {
            first_sector = directory_start;
            sector_count = getrootdirsectorscount();
        }
        else
        {
            first_sector = clustertosector(current_cluster);
            sector_count = fat->bpb.bpb1.sector_per_cluster;
        }

        for (sector_index = 0; sector_index < sector_count; ++sector_index)
        {
            uint8_t buffer[SECTORSIZE];
            uint32_t entry_index;

            if (!readsector(first_sector + sector_index, buffer))
                return false;

            for (entry_index = 0; entry_index < SECTORSIZE / FAT_ENTRY_SIZE; ++entry_index)
            {
                uint8_t *raw = buffer + entry_index * FAT_ENTRY_SIZE;
                file_entry_t current;

                if (raw[0] == 0x00)
                    return false;

                if (raw[0] == FILE_NAME_DELETED)
                    continue;

                if (raw[11] == F_ATTR_LNGFNM)
                    continue;

                memcpy(&current, raw, sizeof(current));

                if (current.attribute & F_ATTR_VOLMID)
                    continue;

                {
                    char shortname[32];

                    if (getshortfilename(current.name, shortname, sizeof(shortname)) && names_equal(filename, shortname))
                    {
                        result->entry = current;
                        result->sector = first_sector + sector_index;
                        result->offset = entry_index * FAT_ENTRY_SIZE;
                        return true;
                    }
                }

                if (entry_index > 0)
                {
                    size_t lfn_count = 0;
                    uint32_t scan = entry_index;

                    while (scan > 0)
                    {
                        uint8_t *previous = buffer + (scan - 1) * FAT_ENTRY_SIZE;

                        if (previous[11] != F_ATTR_LNGFNM)
                            break;

                        ++lfn_count;
                        --scan;

                        if (lfn_count >= LFN_MAX_ENTRIES)
                            break;
                    }

                    if (lfn_count > 0)
                    {
                        uint8_t lfn_raw[LFN_MAX_ENTRIES * FAT_ENTRY_SIZE];
                        char longname[MAX_FILENAME_LENGTH];
                        size_t j;
                        uint8_t checksum;

                        for (j = 0; j < lfn_count; ++j)
                            memcpy(lfn_raw + j * FAT_ENTRY_SIZE, buffer + (entry_index - lfn_count + j) * FAT_ENTRY_SIZE, FAT_ENTRY_SIZE);

                        checksum = lfn_checksum((const uint8_t *)current.name);

                        {
                            bool checksum_ok = true;

                            for (j = 0; j < lfn_count; ++j)
                            {
                                lfn_entry_t le;

                                memcpy(&le, lfn_raw + j * FAT_ENTRY_SIZE, sizeof(le));

                                if (le.checksum != checksum)
                                {
                                    checksum_ok = false;
                                    break;
                                }
                            }

                            if (checksum_ok && get_lfn_name(lfn_raw, lfn_count, longname, sizeof(longname)) && names_equal(filename, longname))
                            {
                                result->entry = current;
                                result->sector = first_sector + sector_index;
                                result->offset = entry_index * FAT_ENTRY_SIZE;
                                return true;
                            }
                        }
                    }
                }
            }
        }

        if (is_root_fat16)
            break;

        {
            uint32_t next = (uint32_t)readcluster(current_cluster);

            if (is_eoc(next))
                break;

            if (next < 2 || !is_valid_data_cluster(next))
                break;

            current_cluster = next;
        }
    }

    return false;
}

path_sub_t getpath(const char *path)
{
    path_sub_t result;
    size_t length = 0;

    memset(&result, 0, sizeof(result));

    if (!path)
        return result;

    while (*path != '\0')
    {
        char c = *path++;

        if (c == '/' || c == '\\')
        {
            if (length > 0)
            {
                result.path[result.pathcount].path[length] = '\0';
                result.pathcount++;
                length = 0;

                if (result.pathcount >= MAX_PATH_COMPONENTS)
                    break;
            }

            continue;
        }

        if (length < sizeof(result.path[0].path) - 1)
            result.path[result.pathcount].path[length++] = c;
    }

    if (length > 0 && result.pathcount < MAX_PATH_COMPONENTS)
    {
        result.path[result.pathcount].path[length] = '\0';
        result.pathcount++;
    }

    return result;
}

bool find_path_directory(const path_sub_t *path, int component_count, uint32_t *directory_sector)
{
    int i;
    uint32_t current_sector;

    if (!path || !directory_sector)
        return false;

    current_sector = getrootdirsectorstart();

    if (current_sector == 0)
        return false;

    for (i = 0; i < component_count; ++i)
    {
        found_file_t found;

        if (!findfileinsectorfilenumber(current_sector, path->path[i].path, &found))
            return false;

        if (!(found.entry.attribute & F_ATTR_DIRECT))
            return false;

        {
            uint32_t cluster = getfilefirstcluster(&found.entry);

            if (cluster < 2)
                return false;

            current_sector = clustertosector(cluster);
        }
    }

    *directory_sector = current_sector;
    return true;
}

uint8_t getlongfilename(char *filename, uint32_t sector)
{
    uint8_t buffer[SECTORSIZE];
    uint8_t previous_buffer[SECTORSIZE];
    uint32_t entry_index;
    uint32_t lfn_count = 0;
    uint32_t scan_index;
    uint8_t lfn_raw[LFN_MAX_ENTRIES * FAT_ENTRY_SIZE];
    file_entry_t file_entry;
    uint8_t checksum;
    uint32_t first_lfn_sector;
    uint32_t first_lfn_index;
    uint32_t i;
    char fn[MAX_FILENAME_LENGTH];
    if (!filename)
        return 0;
    memset(fn, 0, sizeof(fn));
    strcpy(fn, filename);        
    filename[0] = '\0';
    if (!imagedisk_file || !hasactive() || !isfattype())
        return 0;
    if (!readsector(sector, buffer))
        return 0;
    for (entry_index = 0; entry_index < SECTORSIZE / FAT_ENTRY_SIZE; ++entry_index)
    {
        uint8_t *raw = buffer + entry_index * FAT_ENTRY_SIZE;
        if (raw[0] == 0x00)
            return 0;
        if (raw[0] == FILE_NAME_DELETED)
            continue;
        if (raw[11] == F_ATTR_LNGFNM)
            continue;
        memcpy(&file_entry, raw, sizeof(file_entry));
        if (file_entry.attribute & F_ATTR_VOLMID)
            continue;
        lfn_count = 0;
        scan_index = entry_index;
        while (scan_index > 0)
        {
            uint8_t *previous = buffer + (scan_index - 1) * FAT_ENTRY_SIZE;
            if (previous[11] != F_ATTR_LNGFNM)
                break;
            ++lfn_count;
            --scan_index;
            if (lfn_count >= LFN_MAX_ENTRIES)
                break;
        }
        first_lfn_sector = sector;
        first_lfn_index = scan_index;
        if (scan_index == 0 && lfn_count < LFN_MAX_ENTRIES)
        {
            uint32_t previous_sector;
            if (sector == 0)
                continue;
            previous_sector = sector - 1;
            if (!readsector(previous_sector, previous_buffer))
                continue;
            {
                int32_t previous_index = (SECTORSIZE / FAT_ENTRY_SIZE) - 1;
                while (previous_index >= 0)
                {
                    uint8_t *previous = previous_buffer + previous_index * FAT_ENTRY_SIZE;
                    if (previous[11] != F_ATTR_LNGFNM)
                        break;
                    ++lfn_count;
                    first_lfn_sector = previous_sector;
                    first_lfn_index = (uint32_t)previous_index;
                    if (lfn_count >= LFN_MAX_ENTRIES)
                        break;
                    --previous_index;
                }
            }
        }
        if (lfn_count == 0 || lfn_count > LFN_MAX_ENTRIES)
            continue;
        memset(lfn_raw, 0, sizeof(lfn_raw));
        if (first_lfn_sector == sector)
        {
            for (i = 0; i < lfn_count; ++i)
            {
                memcpy(lfn_raw + i * FAT_ENTRY_SIZE, buffer + (first_lfn_index + i) * FAT_ENTRY_SIZE, FAT_ENTRY_SIZE);
            }
        }
        else
        {
            uint32_t previous_entries = (SECTORSIZE / FAT_ENTRY_SIZE) - first_lfn_index;
            if (previous_entries > lfn_count)
                previous_entries = lfn_count;
            for (i = 0; i < previous_entries; ++i)
            {
                memcpy(lfn_raw + i * FAT_ENTRY_SIZE, previous_buffer + (first_lfn_index + i) * FAT_ENTRY_SIZE, FAT_ENTRY_SIZE);
            }
            for (i = previous_entries; i < lfn_count; ++i)
            {
                memcpy(lfn_raw + i * FAT_ENTRY_SIZE, buffer + (i - previous_entries) * FAT_ENTRY_SIZE, FAT_ENTRY_SIZE);
            }
        }
        checksum = lfn_checksum((const uint8_t *)file_entry.name);
        for (i = 0; i < lfn_count; ++i)
        {
            lfn_entry_t entry;
            memcpy(&entry, lfn_raw + i * FAT_ENTRY_SIZE, sizeof(entry));
            if (entry.attribute != F_ATTR_LNGFNM)
                break;
            if (entry.checksum != checksum)
                break;
        }
        if (i != lfn_count)
            continue;
        {
            lfn_entry_t first_entry;
            memcpy(&first_entry, lfn_raw, sizeof(first_entry));
            if (!(first_entry.order & LFN_LAST))
                continue;
            if ((first_entry.order & 0x1F) != lfn_count)
                continue;
        }
        if (get_lfn_name(lfn_raw, lfn_count, filename, MAX_FILENAME_LENGTH))
        {
        	char shortfilename[MAX_FILENAME_LENGTH];
        	memset(shortfilename, 0, sizeof(shortfilename));
            if (getshortfilename(file_entry.name, shortfilename, sizeof(shortfilename)))
            {
        		if (strncmp(shortfilename, fn, sizeof(shortfilename)) == 0) return 1;
        	}        	
        }
    }
    return 0;
}

file_entry_t *findfileinsector(uint32_t sector, char *filename)
{
    static file_entry_t result;
    uint8_t buffer[SECTORSIZE];
    uint8_t previous_buffer[SECTORSIZE];
    const uint32_t entries_per_sector = SECTORSIZE / FAT_ENTRY_SIZE;
    uint32_t entry_index;

    if (!imagedisk_file || !hasactive() || !isfattype())
        return NULL;

    if (!filename || filename[0] == '\0')
        return NULL;

    if (!readsector(sector, buffer))
        return NULL;

    for (entry_index = 0; entry_index < entries_per_sector; ++entry_index)
    {
        uint8_t *raw;
        file_entry_t entry;
        char shortname[MAX_FILENAME_LENGTH];

        raw = buffer + entry_index * FAT_ENTRY_SIZE;

        if (raw[0] == 0x00)
            break;

        if (raw[0] == FILE_NAME_DELETED)
            continue;

        if (raw[11] == F_ATTR_LNGFNM)
            continue;

        memcpy(&entry, raw, sizeof(entry));

        if (entry.attribute & F_ATTR_VOLMID)
            continue;

        memset(shortname, 0, sizeof(shortname));

        if (getshortfilename(entry.name, shortname, sizeof(shortname)))
        {
            if (names_equal(filename, shortname))
            {
                result = entry;
                return &result;
            }
        }

        {
            uint32_t lfn_count = 0;
            uint32_t scan_index = entry_index;

            while (scan_index > 0)
            {
                uint8_t *previous;

                previous = buffer + (scan_index - 1) * FAT_ENTRY_SIZE;

                if (previous[11] != F_ATTR_LNGFNM)
                    break;

                ++lfn_count;
                --scan_index;

                if (lfn_count >= LFN_MAX_ENTRIES)
                    break;
            }

            if (scan_index == 0 && lfn_count < LFN_MAX_ENTRIES && sector > 0)
            {
                if (readsector(sector - 1, previous_buffer))
                {
                    int32_t previous_index = (int32_t)entries_per_sector - 1;

                    while (previous_index >= 0)
                    {
                        uint8_t *previous;

                        previous = previous_buffer + (uint32_t)previous_index * FAT_ENTRY_SIZE;

                        if (previous[11] != F_ATTR_LNGFNM)
                            break;

                        ++lfn_count;
                        --previous_index;

                        if (lfn_count >= LFN_MAX_ENTRIES)
                            break;
                    }
                }
            }

            if (lfn_count > 0 && lfn_count <= LFN_MAX_ENTRIES)
            {
                uint8_t lfn_raw[LFN_MAX_ENTRIES * FAT_ENTRY_SIZE];
                char longname[MAX_FILENAME_LENGTH];
                uint8_t checksum;
                uint32_t first_lfn_sector;
                uint32_t first_lfn_index;
                uint32_t copied;
                uint32_t i;

                first_lfn_sector = sector;
                first_lfn_index = scan_index;

                if (scan_index == 0 && sector > 0 && lfn_count > 0)
                {
                    uint32_t current_count = 0;

                    for (i = 0; i < entry_index; ++i)
                    {
                        uint8_t *p = buffer + i * FAT_ENTRY_SIZE;

                        if (p[11] == F_ATTR_LNGFNM)
                            ++current_count;
                        else
                            current_count = 0;
                    }

                    current_count = 0;

                    for (i = 0; i < entry_index; ++i)
                    {
                        uint8_t *p = buffer + i * FAT_ENTRY_SIZE;

                        if (p[11] != F_ATTR_LNGFNM)
                            current_count = 0;
                        else
                            ++current_count;
                    }

                    if (current_count > 0 && current_count < lfn_count)
                    {
                        uint32_t previous_count = lfn_count - current_count;

                        if (readsector(sector - 1, previous_buffer))
                        {
                            memset(lfn_raw, 0, sizeof(lfn_raw));
                            copied = 0;

                            {
                                int32_t start = (int32_t)entries_per_sector - (int32_t)previous_count;

                                if (start < 0)
                                    start = 0;

                                for (i = 0; i < previous_count; ++i)
                                {
                                    memcpy(lfn_raw + copied * FAT_ENTRY_SIZE,
                                           previous_buffer + (uint32_t)(start + (int32_t)i) * FAT_ENTRY_SIZE,
                                           FAT_ENTRY_SIZE);
                                    ++copied;
                                }
                            }

                            for (i = 0; i < current_count && copied < lfn_count; ++i)
                            {
                                memcpy(lfn_raw + copied * FAT_ENTRY_SIZE,
                                       buffer + i * FAT_ENTRY_SIZE,
                                       FAT_ENTRY_SIZE);
                                ++copied;
                            }

                            checksum = lfn_checksum((const uint8_t *)entry.name);

                            {
                                bool valid = true;

                                for (i = 0; i < lfn_count; ++i)
                                {
                                    lfn_entry_t lfn;

                                    memcpy(&lfn, lfn_raw + i * FAT_ENTRY_SIZE, sizeof(lfn));

                                    if (lfn.attribute != F_ATTR_LNGFNM || lfn.checksum != checksum)
                                    {
                                        valid = false;
                                        break;
                                    }
                                }

                                if (valid)
                                {
                                    lfn_entry_t first;

                                    memcpy(&first, lfn_raw, sizeof(first));

                                    if ((first.order & LFN_LAST) && (first.order & 0x1F) == lfn_count)
                                    {
                                        if (get_lfn_name(lfn_raw, lfn_count, longname, sizeof(longname)))
                                        {
                                            if (names_equal(filename, longname))
                                            {
                                                result = entry;
                                                return &result;
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        continue;
                    }

                    first_lfn_sector = sector;
                    first_lfn_index = scan_index;
                }

                if (first_lfn_sector == sector)
                {
                    memset(lfn_raw, 0, sizeof(lfn_raw));

                    for (i = 0; i < lfn_count; ++i)
                    {
                        if (first_lfn_index + i >= entry_index)
                            break;

                        memcpy(lfn_raw + i * FAT_ENTRY_SIZE,
                               buffer + (first_lfn_index + i) * FAT_ENTRY_SIZE,
                               FAT_ENTRY_SIZE);
                    }

                    if (i != lfn_count)
                        continue;
                }
                else
                {
                    continue;
                }

                checksum = lfn_checksum((const uint8_t *)entry.name);

                {
                    bool valid = true;

                    for (i = 0; i < lfn_count; ++i)
                    {
                        lfn_entry_t lfn;

                        memcpy(&lfn, lfn_raw + i * FAT_ENTRY_SIZE, sizeof(lfn));

                        if (lfn.attribute != F_ATTR_LNGFNM || lfn.checksum != checksum)
                        {
                            valid = false;
                            break;
                        }
                    }

                    if (!valid)
                        continue;
                }

                {
                    lfn_entry_t first_lfn;

                    memcpy(&first_lfn, lfn_raw, sizeof(first_lfn));

                    if (!(first_lfn.order & LFN_LAST))
                        continue;

                    if ((first_lfn.order & 0x1F) != lfn_count)
                        continue;
                }

                if (get_lfn_name(lfn_raw, lfn_count, longname, sizeof(longname)))
                {
                    if (names_equal(filename, longname))
                    {
                        result = entry;
                        return &result;
                    }
                }
            }
        }
    }

    return NULL;
}

uint32_t listdir(uint32_t sector)
{
    uint32_t total_files = 0;
    uint32_t current_cluster = 0;
    bool is_root_fat16 = false;

    if (!imagedisk_file || !hasactive() || !isfattype())
        return 0;

    if (sector == 0)
        return 0;

    if (isfat16type() && sector == getrootdirsector())
    {
        is_root_fat16 = true;
    }
    else
    {
        current_cluster = getclusterfromsector(sector);

        if (isfat32type() && sector == getrootdirsectorstart())
            current_cluster = getrootdircluster();

        if (current_cluster < 2)
            return 0;
    }

    for (;;)
    {
        uint32_t first_sector;
        uint32_t sector_count;
        uint32_t sector_index;

        if (is_root_fat16)
        {
            first_sector = getrootdirsector();
            sector_count = getrootdirsectorscount();
        }
        else
        {
            first_sector = clustertosector(current_cluster);
            sector_count = fat->bpb.bpb1.sector_per_cluster;
        }

        if (first_sector == 0 || sector_count == 0)
            break;

        for (sector_index = 0; sector_index < sector_count; ++sector_index)
        {
            uint32_t current_sector = first_sector + sector_index;
            file_entry_t entries[16];
            uint32_t filecount;

            if (!getfileentriesofsector(current_sector, entries))
                return total_files;

            for (filecount = 0; filecount < 16; ++filecount)
            {
                file_entry_t *entry = &entries[filecount];

                char shortfilename[MAX_FILENAME_LENGTH];
                char longfilename[MAX_FILENAME_LENGTH];
                char filename[MAX_FILENAME_LENGTH];
                char ftype[16];

                uint32_t fsize;
                uint32_t fcluster;
                uint32_t fsector;
                uint64_t fwhere;

                bool has_lfn = false;

                if ((uint8_t)entry->name[0] == 0x00)
                    goto directory_done;

                if ((uint8_t)entry->name[0] == FILE_NAME_DELETED)
                    continue;

                if (entry->attribute == F_ATTR_LNGFNM)
                    continue;

                if (entry->attribute & F_ATTR_VOLMID)
                    continue;

                memset(shortfilename, 0, sizeof(shortfilename));

                if (!getshortfilename(entry->name, shortfilename, sizeof(shortfilename)))
                    continue;

                strcpy(longfilename, shortfilename);

                if (getlongfilename(longfilename, current_sector))
                {
                    strcpy(filename, longfilename);
                    has_lfn = true;
                }
                else
                {
                    strcpy(filename, shortfilename);
                    has_lfn = false;
                }

                (void)has_lfn;

                fsize = entry->size;
                fcluster = getfilefirstcluster(entry);

                if (fcluster != 0)
                    fsector = getfirstsectorofcluster(fcluster);
                else
                    fsector = 0;

                if (fsector != 0)
                    fwhere = sectortobytes(fsector);
                else
                    fwhere = 0;

                if (entry->attribute & F_ATTR_DIRECT)
                {
                    strcpy(ftype, "<DIR>");
                    printf("%-32s\t%-12s\t\t\t%-10u\t%-10u\t0x%08llX\n", filename, ftype, fcluster, fsector, (unsigned long long)fwhere);
                }
                else
                {
                    strcpy(ftype, "");
                    printf("%-32s\t%-12s\t%-10u\t%-10u\t%-10u\t0x%08llX\n", filename, ftype, fsize, fcluster, fsector, (unsigned long long)fwhere);
                }

                ++total_files;
            }
        }

        if (is_root_fat16)
            break;

        {
            uint32_t next_cluster;

            next_cluster = (uint32_t)readcluster(current_cluster);

            if (next_cluster == 0 || is_eoc(next_cluster))
                break;

            if (next_cluster < 2 || !is_valid_data_cluster(next_cluster))
                break;

            current_cluster = next_cluster;
        }
    }

directory_done:

    return total_files;
}

void set_current_datetime(file_entry_t *entry)
{
    time_t now;
    struct tm *t;

    if (!entry)
        return;

    now = time(NULL);
    t = localtime(&now);

    if (!t)
        return;

    {
        int year = t->tm_year + 1900;

        if (year < 1980)
            year = 1980;

        if (year > 2107)
            year = 2107;

        entry->creation_date = (uint16_t)(((year - 1980) << 9) | ((t->tm_mon + 1) << 5) | t->tm_mday);
        entry->write_date = entry->creation_date;
        entry->last_date = entry->creation_date;
        entry->creation_time = (uint16_t)((t->tm_hour << 11) | (t->tm_min << 5) | (t->tm_sec / 2));
        entry->write_time = entry->creation_time;
        entry->creation_time_tenth = (uint8_t)((t->tm_sec & 1) ? 100 : 0);
    }
}

bool write_lfn_entry(uint8_t *buffer, uint32_t offset, uint8_t order, const uint16_t *name, size_t name_length, uint8_t checksum)
{
    lfn_entry_t entry;
    size_t i;

    if (!buffer || !name || offset + FAT_ENTRY_SIZE > SECTORSIZE)
        return false;

    memset(&entry, 0xFF, sizeof(entry));

    entry.order = order;
    entry.attribute = F_ATTR_LNGFNM;
    entry.type = 0;
    entry.checksum = checksum;
    entry.first_cluster = 0;

    for (i = 0; i < 5; ++i)
        entry.name1[i] = 0xFFFF;

    for (i = 0; i < 6; ++i)
        entry.name2[i] = 0xFFFF;

    for (i = 0; i < 2; ++i)
        entry.name3[i] = 0xFFFF;

    for (i = 0; i < 5 && i < name_length; ++i)
        entry.name1[i] = name[i];

    for (i = 0; i < 6 && i + 5 < name_length; ++i)
        entry.name2[i] = name[i + 5];

    for (i = 0; i < 2 && i + 11 < name_length; ++i)
        entry.name3[i] = name[i + 11];

    if (name_length < 5)
        entry.name1[name_length] = 0x0000;
    else if (name_length < 11)
        entry.name2[name_length - 5] = 0x0000;
    else if (name_length < 13)
        entry.name3[name_length - 11] = 0x0000;

    memcpy(buffer + offset, &entry, sizeof(entry));
    return true;
}

bool write_file_directory_entries(uint32_t sector, uint32_t offset, const char *long_name, const uint8_t dos_name[11], uint32_t cluster, uint32_t file_size)
{
    uint16_t utf16_name[LFN_MAX_CHARS];
    size_t utf16_length;
    size_t lfn_count;
    size_t i;
    uint8_t buffer[SECTORSIZE];

    if (!long_name || !dos_name)
        return false;

    utf16_length = utf8_to_utf16(long_name, utf16_name, LFN_MAX_CHARS);

    if (utf16_length == 0 || utf16_length > LFN_MAX_CHARS)
        return false;

    lfn_count = lfn_entry_count(utf16_length);

    if (lfn_count == 0 || lfn_count > LFN_MAX_ENTRIES)
        return false;

    if (offset + (lfn_count + 1) * FAT_ENTRY_SIZE > SECTORSIZE)
        return false;

    if (!readsector(sector, buffer))
        return false;

    for (i = 0; i < lfn_count; ++i)
    {
        uint8_t order;
        size_t character_offset;
        size_t remaining;
        size_t count;

        order = (uint8_t)(lfn_count - i);

        if (i == 0)
            order |= LFN_LAST;

        character_offset = (lfn_count - 1 - i) * 13;

        if (character_offset >= utf16_length)
            count = 0;
        else
        {
            remaining = utf16_length - character_offset;
            count = remaining > 13 ? 13 : remaining;
        }

        if (!write_lfn_entry(buffer, offset + i * FAT_ENTRY_SIZE, order, utf16_name + character_offset, count, lfn_checksum(dos_name)))
            return false;
    }

    {
        file_entry_t entry;

        memset(&entry, 0, sizeof(entry));
        memcpy(entry.name, dos_name, 11);
        entry.attribute = F_ATTR_ARCHVE;
        entry.size = file_size;

        if (isfat32type())
            entry.first_cluster_hi = (uint16_t)(cluster >> 16);
        else
            entry.first_cluster_hi = 0;

        entry.first_cluster_lo = (uint16_t)(cluster & 0xFFFF);
        set_current_datetime(&entry);
        memcpy(buffer + offset + lfn_count * FAT_ENTRY_SIZE, &entry, sizeof(entry));
    }

    return writesector(sector, buffer);
}

bool findfreeslots_root_fat16(uint32_t required_entries, uint32_t *sector_found, uint32_t *offset_found)
{
    uint32_t s;
    uint32_t free_count = 0;
    uint32_t start_sector = 0;
    uint32_t start_offset = 0;

    if (!sector_found || !offset_found || required_entries == 0)
        return false;

    for (s = 0; s < getrootdirsectorscount(); ++s)
    {
        uint8_t buffer[SECTORSIZE];
        uint32_t offset;

        if (!readsector(getrootdirsector() + s, buffer))
            return false;

        for (offset = 0; offset + FAT_ENTRY_SIZE <= SECTORSIZE; offset += FAT_ENTRY_SIZE)
        {
            bool free_entry = buffer[offset] == 0x00 || buffer[offset] == FILE_NAME_DELETED;

            if (free_entry)
            {
                if (free_count == 0)
                {
                    start_sector = getrootdirsector() + s;
                    start_offset = offset;
                }

                ++free_count;

                if (free_count >= required_entries)
                {
                    *sector_found = start_sector;
                    *offset_found = start_offset;
                    return true;
                }
            }
            else
            {
                free_count = 0;
            }
        }
    }

    return false;
}

bool findfreeslots(uint32_t dir_cluster, uint32_t required_entries, uint32_t *sector_found, uint32_t *offset_found)
{
    uint32_t cluster;
    uint32_t j;
    uint32_t bytes_per_sector;
    uint32_t sectors_per_cluster;
    uint32_t max_cluster;

    if (!imagedisk_file || !hasactive() || !isfattype() || !sector_found || !offset_found || required_entries == 0)
        return false;

    if (isfat16type() && dir_cluster == 0)
        return false;

    bytes_per_sector = fat->bpb.bpb1.bytes_per_sector;
    sectors_per_cluster = fat->bpb.bpb1.sector_per_cluster;
    max_cluster = fat_max_cluster();
    cluster = dir_cluster;

    while (cluster >= 2 && cluster <= max_cluster)
    {
        uint32_t first_sector = clustertosector(cluster);

        if (first_sector == 0)
            return false;

        for (j = 0; j < sectors_per_cluster; ++j)
        {
            uint8_t buffer[SECTORSIZE];
            uint32_t offset;
            uint32_t free_count = 0;
            uint32_t start_offset = 0;

            if (!readsector(first_sector + j, buffer))
                return false;

            for (offset = 0; offset + FAT_ENTRY_SIZE <= bytes_per_sector; offset += FAT_ENTRY_SIZE)
            {
                bool free_entry = buffer[offset] == 0x00 || buffer[offset] == FILE_NAME_DELETED;

                if (free_entry)
                {
                    if (free_count == 0)
                        start_offset = offset;

                    ++free_count;

                    if (free_count >= required_entries)
                    {
                        *sector_found = first_sector + j;
                        *offset_found = start_offset;
                        return true;
                    }
                }
                else
                {
                    free_count = 0;
                }
            }
        }

        cluster = (uint32_t)readcluster(cluster);

        if (cluster == 0 || is_eoc(cluster))
            break;
    }

    return false;
}

bool write_file_data(FILE *source, uint64_t file_size, uint32_t first_cluster)
{
    uint32_t bytes_per_sector;
    uint32_t sectors_per_cluster;
    uint32_t cluster_size;
    uint8_t *buffer;
    uint64_t remaining;
    uint32_t cluster;
    uint32_t guard = 0;
    uint32_t max_cluster;

    if (!source)
        return false;

    if (file_size == 0)
        return first_cluster == 0;

    if (first_cluster < 2)
        return false;

    bytes_per_sector = fat->bpb.bpb1.bytes_per_sector;
    sectors_per_cluster = fat->bpb.bpb1.sector_per_cluster;
    cluster_size = bytes_per_sector * sectors_per_cluster;

    if (cluster_size == 0)
        return false;

    buffer = malloc(cluster_size);

    if (!buffer)
        return false;

    remaining = file_size;
    cluster = first_cluster;
    max_cluster = fat_max_cluster();

    while (remaining > 0)
    {
        uint32_t amount;
        uint32_t sector;
        uint32_t s;

        ++guard;

        if (guard > max_cluster)
        {
            free(buffer);
            return false;
        }

        if (cluster < 2 || cluster > max_cluster)
        {
            free(buffer);
            return false;
        }

        amount = remaining > cluster_size ? cluster_size : (uint32_t)remaining;

        memset(buffer, 0, cluster_size);

        if (fread(buffer, 1, amount, source) != amount)
        {
            free(buffer);
            return false;
        }

        sector = clustertosector(cluster);

        if (sector == 0)
        {
            free(buffer);
            return false;
        }

        for (s = 0; s < sectors_per_cluster; ++s)
        {
            if (!writesector(sector + s, buffer + s * bytes_per_sector))
            {
                free(buffer);
                return false;
            }
        }

        remaining -= amount;

        if (remaining > 0)
        {
            uint32_t next = (uint32_t)readcluster(cluster);

            if (next < 2 || is_eoc(next) || next > max_cluster)
            {
                free(buffer);
                return false;
            }

            cluster = next;
        }
    }

    free(buffer);
    return true;
}

file_entry_t* findfileindirectory(uint32_t sector, const char *filename)
{
    uint32_t current_cluster = 0;
    bool is_root_fat16 = false;

    if (!imagedisk_file || !hasactive() || !isfattype())
        return NULL;

    if (sector == 0)
        return NULL;

    if (isfat16type() && sector == getrootdirsector())
    {
        is_root_fat16 = true;
    }
    else
    {
        current_cluster = getclusterfromsector(sector);

        if (isfat32type() && sector == getrootdirsectorstart())
            current_cluster = getrootdircluster();

        if (current_cluster < 2)
            return 0;
    }

    for (;;)
    {
        uint32_t first_sector;
        uint32_t sector_count;
        uint32_t sector_index;

        if (is_root_fat16)
        {
            first_sector = getrootdirsector();
            sector_count = getrootdirsectorscount();
        }
        else
        {
            first_sector = clustertosector(current_cluster);
            sector_count = fat->bpb.bpb1.sector_per_cluster;
        }

        if (first_sector == 0 || sector_count == 0)
            break;

        for (sector_index = 0; sector_index < sector_count; ++sector_index)
        {
            uint32_t current_sector = first_sector + sector_index;
            file_entry_t entries[16];
            uint32_t filecount;

            if (!getfileentriesofsector(current_sector, entries))
                return NULL;

            for (filecount = 0; filecount < 16; ++filecount)
            {
                file_entry_t *entry = &entries[filecount];

                char shortfilename[MAX_FILENAME_LENGTH];
                char longfilename[MAX_FILENAME_LENGTH];
                char findfilename[MAX_FILENAME_LENGTH];
                char ftype[16];

                uint32_t fsize;
                uint32_t fcluster;
                uint32_t fsector;
                uint64_t fwhere;

                bool has_lfn = false;

                if ((uint8_t)entry->name[0] == 0x00)
                    goto directory_done;

                if ((uint8_t)entry->name[0] == FILE_NAME_DELETED)
                    continue;

                if (entry->attribute == F_ATTR_LNGFNM)
                    continue;

                if (entry->attribute & F_ATTR_VOLMID)
                    continue;

                memset(shortfilename, 0, sizeof(shortfilename));

                if (!getshortfilename(entry->name, shortfilename, sizeof(shortfilename)))
                    continue;

                strcpy(longfilename, shortfilename);

                if (getlongfilename(longfilename, current_sector))
                {
                    strcpy(findfilename, longfilename);
                    has_lfn = true;
                }
                else
                {
                    strcpy(findfilename, shortfilename);
                    has_lfn = false;
                }

                (void)has_lfn;

                fsize = entry->size;
                fcluster = getfilefirstcluster(entry);

                if (fcluster != 0)
                    fsector = getfirstsectorofcluster(fcluster);
                else
                    fsector = 0;

                if (fsector != 0)
                    fwhere = sectortobytes(fsector);
                else
                    fwhere = 0;
                    
                if (strcmp(filename, findfilename) == 0) {
                	return entry;
                }
            }
        }

        if (is_root_fat16)
            break;

        {
            uint32_t next_cluster;

            next_cluster = (uint32_t)readcluster(current_cluster);

            if (next_cluster == 0 || is_eoc(next_cluster))
                break;

            if (next_cluster < 2 || !is_valid_data_cluster(next_cluster))
                break;

            current_cluster = next_cluster;
        }
    }

directory_done:

    return NULL;
}

file_entry_t* findfileentry(uint32_t sector, const char *filename)
{
    if (!imagedisk_file || !hasactive() || !isfattype())
        return NULL;

    if (sector == 0)
        return NULL;

	file_entry_t *f = findfileinsector(sector, (char*)filename);
	if (!f) 
	{
		f = findfileindirectory(sector, (char*)filename);
		if (!f) 
		{
			char fn[1024];
			strcpy(fn, filename);
			strcpy(fn, strupr1(fn));
			f = findfileindirectory(sector, (char*)fn);
			if (!f) return NULL;
			else return f;
		}
		else return f;
	} else return f;
    return NULL;
}

file_entry_t* findfile(uint32_t sector, const char *filename)
{
    const char *file_name;
    const char *final_name;
    char *new_path;
    file_entry_t *f;
    file_entry_t *entry;
    uint32_t first_cluster;
    path_sub_t path;
    uint32_t root_cluster;
    uint32_t root_sector;
    uint32_t directory_sector;
    found_file_t existing;
    uint8_t dos_name[11];
    char fn[MAX_FILENAME_LENGTH];
    char filename_0[MAX_FILENAME_LENGTH];
    bool is_file = false;
    bool file_exists_on_disk = false;
    uint16_t utf16_name[LFN_MAX_CHARS];
    size_t utf16_length;
    size_t lfn_count;
    
    if (filename == NULL)
        return NULL;
    
    if (!imagedisk_file || !hasactive() || !isfattype())
        return NULL;

    if (sector == 0)
        return NULL;

    memset(fn, 0, sizeof(fn));
    strcpy(fn, filename);
    if ((strcmp(fn, "/") == 0) || (strcmp(fn, "") == 0) || (strcmp(fn, ".") == 0))
    {
    	strcpy(fn, "/.");
    }
    file_name = (char*)fn;
    strcpy(filename_0, basename1(file_name));
    root_cluster = getrootdircluster();
    root_sector = getrootdirsectorstart();
    path = getpath(file_name);

    if (path.pathcount == 0)
    {
        return NULL;
    }
    final_name = path.path[path.pathcount - 1].path;

    utf16_length = utf8_to_utf16(final_name, utf16_name, LFN_MAX_CHARS);
    if (utf16_length == 0)
    {
        return NULL;
    }
    if (utf16_length > LFN_MAX_CHARS)
    {
        return NULL;
    }

    lfn_count = lfn_entry_count(utf16_length);
    if (lfn_count == 0 || lfn_count > LFN_MAX_ENTRIES)
    {
        return NULL;
    }

    strfilenamedos(final_name, dos_name);

    first_cluster = 0;
    if (path.pathcount == 1)
    {
        directory_sector = root_sector;
    }
    
    if (strcmp(file_name, "/.") == 0) 
    {
    	file_entry_t e;
    	memset(&e, 0, sizeof(file_entry_t));
    	memset(&e, ' ', 11);
    	e.name[0] = '/';
    	e.attribute = F_ATTR_DIRECT;
    	e.first_cluster_hi = HIGH16(root_cluster);
    	e.first_cluster_lo = LOW16(root_cluster);
    	file_entry_t *root_entry = &e;
    	return root_entry;
    }
    
    if (find_path_directory(&path, path.pathcount - 1, &directory_sector))
    {
        new_path = (char*)path.path[path.pathcount-1].path;
        file_entry_t *f1 = findfileinsector(directory_sector, new_path);
        if (f1 == NULL)
        {
    		uint32_t newsector = directory_sector;
    		final_name = path.path[path.pathcount - 1].path;
    		strfilenamedos(final_name, dos_name);
			if (findfileinsectorfilenumber(directory_sector, final_name, &existing))
    			is_file = true;
    		if (is_file)
    		{
    			f = &existing.entry;
        		if (f == NULL)
        		{
            		return NULL;
        		}
        		else
        		{
        			first_cluster = getfilefirstcluster(f);
        			if (f->size != 0 && first_cluster < 2)
        			{
            			return NULL;
        			}

        			if (f->size == 0)
            			first_cluster = 0;
            		
            		if (f->attribute & F_ATTR_DIRECT)
            		{ 
            			is_file = false;
        				directory_sector = clustertosector(getfilefirstcluster(f)); 
            		}            			
        		}
        		file_exists_on_disk = true;
        	}
        	else
        	{
        		if (strcmp(file_name, "/.") == 0)
        		{
        			file_exists_on_disk = true;
        			is_file = false;
        		}
        	}
        }
        else
        {
        	first_cluster = getfilefirstcluster(f1);
        	if (f1->size != 0 && first_cluster < 2)
        	{
            	return NULL;
        	}
	
        	if (f1->size == 0)
            	first_cluster = 0;
            	
            f = f1;
            if (f->attribute & F_ATTR_DIRECT) 
            {
            	is_file = false;
            	directory_sector = clustertosector(getfilefirstcluster(f1));            	
            }
			else 
			{
				is_file = true;
			}
			file_exists_on_disk = true;
        }
    }
    else
    {
        if (strcmp(file_name, "/.") == 0)
        {
        	file_exists_on_disk = true;
        	is_file = false;
        }
        else
        {
    		is_file = false;
    		file_exists_on_disk = false;
    	}
    }
    
    if (file_exists_on_disk)
    {
    	if (directory_sector == 0) return NULL;
    	entry = findfileentry(directory_sector, filename);
    	if (entry == NULL) 
    	{
    		if (f != NULL) return f;
    		else return NULL;
    	}
    	return entry;
    }
    
    return NULL;
}

unsigned long getfilesize(const char *filename, unsigned long sector)
{
    if (!imagedisk_file || !hasactive() || !isfattype())
        return 0;

    if (sector == 0)
        return 0;

	file_entry_t *f = findfile(sector, (char*)filename);
	if (f != NULL) 
	{
		return f->size;
	}
	
	return 0;
}

unsigned long getfilesizeondisk(const char *filename, unsigned long sector)
{
    if (!imagedisk_file || !hasactive() || !isfattype())
        return 0;

    if (sector == 0)
        return 0;

	file_entry_t *f = findfile(sector, (char*)filename);
	if (!f) return 0;
	
	if (f->attribute & F_ATTR_DIRECT) return 0;
	if (f->size > 0)
	{
		unsigned long first_cluster = getfilefirstcluster(f);
		if (f->size != 0 && first_cluster < 2)
		{
			return 0;
		}
		unsigned char buffer[SECTORSIZE];
		unsigned long offset = 0;
		unsigned long cluster = first_cluster;
		unsigned long file_size = f->size;
		unsigned long remaining = file_size;
		unsigned long sectors_per_cluster = fat->bpb.bpb1.sector_per_cluster;
		unsigned long bytes_to_copy;
		unsigned long fsector;
		unsigned long sector_count;
		unsigned long sector_read=0;
		unsigned long first_sector = clustertosector(first_cluster);
		unsigned long i;
		unsigned long j;
        if (remaining == 0)
        {
            return 0;
        }
        else
        {
        	sector_count = (file_size + SECTORSIZE - 1) / SECTORSIZE;
			while (remaining > 0)
			{
    			fsector = clustertosector(cluster);		
    			if (cluster > 2)
    			{
    				for (i=0;i<((sectors_per_cluster) && (remaining>0));i++)
    				{
        				for(j=0;j<sectors_per_cluster;j++) 
        				{
        					sector_read++;		
        					bytes_to_copy = remaining <= SECTORSIZE
                     					? remaining
                     					: SECTORSIZE;			
        					offset += bytes_to_copy;
        					remaining -= bytes_to_copy;
        				}
    				}
    			}		
    			cluster = getnextcluster(cluster);
			}
			if (remaining == 0)
			{
				if (sector_read > 0) return (uint32_t)sectortobytes(sector_read);
			}
		}		
	}
	return 0;
}

unsigned char getfiledata(const char *filename, unsigned long sector, unsigned char *data)
{
    if (!imagedisk_file || !hasactive() || !isfattype())
        return 0;

    if (sector == 0)
        return 0;

	file_entry_t *f = findfile(sector, (char*)filename);
	if (!f) return 0;
	
	if (f->attribute & F_ATTR_DIRECT) return 0;
	if (f->size > 0)
	{
		unsigned long first_cluster = getfilefirstcluster(f);
		if (f->size != 0 && first_cluster < 2)
		{
			return 0;
		}
		unsigned char buffer[SECTORSIZE];
		unsigned long offset = 0;
		unsigned long cluster = first_cluster;
		unsigned long file_size = f->size;
		unsigned long remaining = file_size;
		unsigned long sectors_per_cluster = fat->bpb.bpb1.sector_per_cluster;
		unsigned long bytes_to_copy;
		unsigned long fsector;
		unsigned long sector_count;
		unsigned long sector_read=0;
		unsigned long first_sector = clustertosector(first_cluster);
		unsigned long i;
		unsigned long j;
        if (remaining == 0)
        {
            return 0;
        }
        else
        {
        	memset(data, 0, file_size);
        	sector_count = (file_size + SECTORSIZE - 1) / SECTORSIZE;
			while (remaining > 0)
			{
    			fsector = clustertosector(cluster);		
    			if (cluster > 2)
    			{
    				for (i=0;i<((sectors_per_cluster) && (remaining>0));i++)
    				{
        				for(j=0;j<sectors_per_cluster;j++) 
        				{
        					readsector(fsector + i + j, buffer);
        					sector_read++;		
        					bytes_to_copy = remaining <= SECTORSIZE
                     					? remaining
                     					: SECTORSIZE;			
        					memcpy(data + offset, buffer, bytes_to_copy);		
        					offset += bytes_to_copy;
        					remaining -= bytes_to_copy;
        				}
    				}
    			}		
    			cluster = getnextcluster(cluster);
			}			
			if (remaining == 0)
			{
				if (sector_read > 0) return 1;
			}
		}		
	}
	return 0;
}

unsigned long getfilesector(const char *filename)
{
	unsigned long first_cluster;
	unsigned long root_sector;
	unsigned long sector;
	bool is_file;
	bool file_exists_on_disk;
	file_entry_t *entry;
	
    first_cluster = 0;
    sector = 0;
    root_sector = getrootdirsectorstart();
    
	entry = findfile(root_sector, filename);
	if (entry != NULL)
	{
		file_exists_on_disk = true;
		first_cluster = getfilefirstcluster(entry);
		if (entry->attribute & F_ATTR_DIRECT) 
		{
			is_file = false;
			sector = clustertosector(first_cluster);
			if (isfat16type()) 
			{
				if (first_cluster == 0) sector = root_sector;
			}						
		}
		else 
		{
			is_file = true;					
    		if (first_cluster != 0)
    		{
    			sector = clustertosector(first_cluster);								
        	}
        	else sector = 0;
		}
	} else 
	{
		file_exists_on_disk = false;
		is_file = false;
		sector = 0;
	}
	return sector;
}

unsigned long getfilecluster(const char *filename)
{
	unsigned long first_cluster;
	unsigned long root_cluster;
	unsigned long cluster;
	bool is_file;
	bool file_exists_on_disk;
	file_entry_t *entry;
	
    first_cluster = 0;
    cluster = 0;
    root_cluster = getrootdircluster();
    
	entry = findfile(root_cluster, filename);
	if (entry != NULL)
	{
		file_exists_on_disk = true;
		first_cluster = getfilefirstcluster(entry);
		if (entry->attribute & F_ATTR_DIRECT) 
		{
			is_file = false;
			cluster = first_cluster;
			if (isfat16type()) 
			{
				if (first_cluster == 0) cluster = root_cluster;
			}						
		}
		else 
		{
			is_file = true;					
    		if (first_cluster != 0)
    		{
    			cluster = first_cluster;
        	}
        	else cluster = 0;
		}
	} else 
	{
		file_exists_on_disk = false;
		is_file = false;
		cluster = 0;
	}
	return cluster;
}

static bool fat_file_valid(FAT_FILE *fp)
{
    return fp != NULL &&
           fp->open &&
           imagedisk_file != NULL &&
           fat != NULL &&
           isfattype();
}

static uint32_t fat_file_cluster_size(void)
{
    if (!fat)
        return 0;

    return (uint32_t)fat->bpb.bpb1.bytes_per_sector *
           (uint32_t)fat->bpb.bpb1.sector_per_cluster;
}

static bool fat_file_update_directory_entry(FAT_FILE *fp)
{
    uint8_t buffer[SECTORSIZE];
    file_entry_t entry;

    if (!fat_file_valid(fp))
        return false;

    if (fp->directory_offset + sizeof(file_entry_t) > SECTORSIZE)
        return false;

    if (!readsector(fp->directory_sector, buffer))
        return false;

    memcpy(&entry,
           buffer + fp->directory_offset,
           sizeof(file_entry_t));

    entry.size = (uint32_t)fp->size;

    if (isfat32type())
        entry.first_cluster_hi =
            (uint16_t)((fp->first_cluster >> 16) & 0xFFFF);
    else
        entry.first_cluster_hi = 0;

    entry.first_cluster_lo =
        (uint16_t)(fp->first_cluster & 0xFFFF);

    set_current_datetime(&entry);

    memcpy(buffer + fp->directory_offset,
           &entry,
           sizeof(file_entry_t));

    if (!writesector(fp->directory_sector, buffer))
        return false;

    fp->entry = entry;

    return true;
}

static bool fat_find_file(const char *filename, found_file_t *result)
{
    path_sub_t path;
    uint32_t directory_sector;
    const char *final_name;

    if (!filename || !result)
        return false;

    if (!imagedisk_file || !hasactive() || !isfattype())
        return false;

    path = getpath(filename);

    if (path.pathcount <= 0)
        return false;

    if (path.pathcount == 1)
    {
        directory_sector = getrootdirsectorstart();

        if (directory_sector == 0)
            return false;
    }
    else
    {
        if (!find_path_directory(&path,
                                 path.pathcount - 1,
                                 &directory_sector))
        {
            return false;
        }
    }

    final_name = path.path[path.pathcount - 1].path;

    if (!findfileinsectorfilenumber(directory_sector,
                                    final_name,
                                    result))
    {
        return false;
    }

    return true;
}

static uint32_t fat_file_cluster_at(FAT_FILE *fp, uint64_t position)
{
    uint32_t cluster_size;
    uint32_t cluster_index;

    if (!fat_file_valid(fp))
        return 0;

    if (fp->first_cluster < 2)
        return 0;

    cluster_size = fat_file_cluster_size();

    if (cluster_size == 0)
        return 0;

    cluster_index = (uint32_t)(position / cluster_size);

    return get_nth_cluster(fp->first_cluster, cluster_index);
}

static bool fat_file_ensure_size(FAT_FILE *fp, uint64_t size)
{
    uint32_t cluster_size;
    uint64_t required64;
    uint32_t required;

    if (!fat_file_valid(fp))
        return false;

    cluster_size = fat_file_cluster_size();

    if (cluster_size == 0)
        return false;

    if (size == 0)
    {
        if (fp->first_cluster != 0)
        {
            if (!freechain(fp->first_cluster))
                return false;

            fp->first_cluster = 0;
        }

        return true;
    }

    required64 =
        (size + cluster_size - 1) / cluster_size;

    if (required64 > UINT32_MAX)
        return false;

    required = (uint32_t)required64;

    if (!resizechain(&fp->first_cluster, required))
        return false;

    return true;
}

static bool fat_file_zero_range(FAT_FILE *fp,
                                uint64_t start,
                                uint64_t end)
{
    uint8_t zero_buffer[SECTORSIZE];

    if (!fat_file_valid(fp))
        return false;

    if (end <= start)
        return true;

    memset(zero_buffer, 0, sizeof(zero_buffer));

    while (start < end)
    {
        uint64_t remaining = end - start;
        size_t amount =
            remaining < sizeof(zero_buffer)
                ? (size_t)remaining
                : sizeof(zero_buffer);

        {
            uint64_t old_position = fp->position;
            size_t written;

            fp->position = start;

            written = fat_write(zero_buffer, 1, amount, fp);

            if (written != amount)
            {
                fp->position = old_position;
                return false;
            }

            start += amount;
        }
    }

    return true;
}

static bool fat_dir_valid(FAT_DIR *dir)
{
    return dir != NULL &&
           dir->open &&
           imagedisk_file != NULL &&
           fat != NULL &&
           isfattype();
}

static bool fat_directory_from_path(const char *dirname,
                                    uint32_t *directory_sector,
                                    uint32_t *directory_cluster,
                                    bool *root_fat16)
{
    path_sub_t path;

    if (!dirname ||
        !directory_sector ||
        !directory_cluster ||
        !root_fat16)
    {
        return false;
    }

    *directory_sector = 0;
    *directory_cluster = 0;
    *root_fat16 = false;

    if (!imagedisk_file ||
        !fat ||
        !hasactive() ||
        !isfattype())
    {
        return false;
    }

    if (dirname[0] == '\0' ||
        strcmp(dirname, "/") == 0 ||
        strcmp(dirname, "\\") == 0)
    {
        *directory_sector = getrootdirsectorstart();

        if (*directory_sector == 0)
            return false;

        if (isfat16type())
        {
            *root_fat16 = true;
            *directory_cluster = 0;
        }
        else
        {
            *directory_cluster = getrootdircluster();

            if (*directory_cluster < 2)
                return false;
        }

        return true;
    }

    path = getpath(dirname);

    if (path.pathcount <= 0)
        return false;

    
    found_file_t found;
    uint32_t sector;

    sector = getrootdirsectorstart();

    if (sector == 0)
        return false;

    if (path.pathcount == 1)
    {
        if (!findfileinsectorfilenumber(
                sector,
                path.path[0].path,
                &found))
        {
            return false;
        }
    }
    else
    {
        if (!find_path_directory(
                &path,
                path.pathcount - 1,
                &sector))
        {
            return false;
        }

        if (!findfileinsectorfilenumber(
                sector,
                path.path[path.pathcount - 1].path,
                &found))
        {
            return false;
        }
    }

    if (!(found.entry.attribute & F_ATTR_DIRECT))
        return false;

    *directory_cluster =
        getfilefirstcluster(&found.entry);

    if (isfat16type() &&
        *directory_cluster == 0)
    {
        return false;
    }

    if (*directory_cluster < 2)
        return false;

    *directory_sector =
        clustertosector(*directory_cluster);

    if (*directory_sector == 0)
        return false;

    *root_fat16 = false;
    

    return true;
}

FAT_FILE *fat_open(const char *filename, const char *mode)
{
    FAT_FILE *fp;
    found_file_t found;

    bool readable = false;
    bool writable = false;
    bool append = false;

    if (!filename || !mode || mode[0] == '\0')
        return NULL;

    if (!imagedisk_file)
        return NULL;

    switch (mode[0])
    {
        case 'r':
            readable = true;
            writable = false;
            break;

        case 'w':
            readable = strchr(mode, '+') != NULL;
            writable = true;
            break;

        case 'a':
            readable = strchr(mode, '+') != NULL;
            writable = true;
            append = true;
            break;

        default:
            return NULL;
    }

    if (!hasactive() || !isfattype())
        return NULL;

    if (!fat_find_file(filename, &found))
        return NULL;

    if (found.entry.attribute & F_ATTR_DIRECT)
        return NULL;

    fp = (FAT_FILE *)calloc(1, sizeof(FAT_FILE));

    if (!fp)
        return NULL;

    fp->entry = found.entry;
    fp->directory_sector = found.sector;
    fp->directory_offset = found.offset;

    fp->first_cluster = getfilefirstcluster(&found.entry);
    fp->size = found.entry.size;

    fp->readable = readable;
    fp->writable = writable;
    fp->append = append;

    fp->position = append ? fp->size : 0;
    fp->eof = false;
    fp->error = false;
    fp->open = true;

    if (mode[0] == 'w')
    {
        if (!fat_file_ensure_size(fp, 0))
        {
            free(fp);
            return NULL;
        }

        fp->size = 0;
        fp->position = 0;

        if (!fat_file_update_directory_entry(fp))
        {
            free(fp);
            return NULL;
        }
    }

    return fp;
}

int fat_close(FAT_FILE *fp)
{
    int result = 0;

    if (!fp)
        return EOF;

    if (!fp->open)
    {
        free(fp);
        return EOF;
    }

    if (fp->writable)
    {
        if (!fat_file_update_directory_entry(fp))
            result = EOF;
    }

    fp->open = false;

    free(fp);

    return result;
}

int fat_seek(FAT_FILE *fp, off_t offset, int whence)
{
    int64_t base;
    int64_t new_position;

    if (!fat_file_valid(fp))
        return -1;

    switch (whence)
    {
        case SEEK_SET:
            base = 0;
            break;

        case SEEK_CUR:
            if (fp->position > INT64_MAX)
                return -1;

            base = (int64_t)fp->position;
            break;

        case SEEK_END:
            if (fp->size > INT64_MAX)
                return -1;

            base = (int64_t)fp->size;
            break;

        default:
            fp->error = true;
            return -1;
    }

    if (offset > 0 && base > INT64_MAX - offset)
    {
        fp->error = true;
        return -1;
    }

    if (offset < 0 && base < INT64_MIN - offset)
    {
        fp->error = true;
        return -1;
    }

    new_position = base + offset;

    if (new_position < 0)
    {
        fp->error = true;
        return -1;
    }

    fp->position = (uint64_t)new_position;
    fp->eof = false;

    return 0;
}

off_t fat_tell(FAT_FILE *fp)
{
    if (!fat_file_valid(fp))
        return (off_t)-1;

    if (fp->position > (uint64_t)INT64_MAX)
        return (off_t)-1;

    return (off_t)fp->position;
}

void fat_rewind(FAT_FILE *fp)
{
    if (!fat_file_valid(fp))
        return;

    fp->position = 0;
    fp->eof = false;
    fp->error = false;
}

int fat_eof(FAT_FILE *fp)
{
    if (!fp || !fp->open)
        return 0;

    return fp->eof ? 1 : 0;
}

size_t fat_read(void *ptr,
                size_t size,
                size_t nmemb,
                FAT_FILE *fp)
{
    uint8_t *destination;
    uint64_t requested;
    uint64_t remaining;
    uint64_t position;

    if (!fat_file_valid(fp) ||
        !ptr ||
        size == 0 ||
        nmemb == 0)
    {
        return 0;
    }

    if (!fp->readable)
    {
        fp->error = true;
        return 0;
    }

    if (nmemb > SIZE_MAX / size)
    {
        fp->error = true;
        return 0;
    }

    requested = (uint64_t)size * (uint64_t)nmemb;

    if (fp->position >= fp->size)
    {
        fp->eof = true;
        return 0;
    }

    remaining = fp->size - fp->position;

    if (requested > remaining)
        requested = remaining;

    destination = (uint8_t *)ptr;
    position = fp->position;

    while (requested > 0)
    {
        uint32_t cluster_size;
        uint32_t cluster;
        uint32_t sector;
        uint32_t sector_offset;
        uint64_t cluster_offset;
        uint64_t cluster_remaining;
        uint64_t amount64;

        cluster_size = fat_file_cluster_size();

        if (cluster_size == 0)
        {
            fp->error = true;
            break;
        }

        cluster = fat_file_cluster_at(fp, position);

        if (cluster < 2)
        {
            fp->error = true;
            break;
        }

        cluster_offset = position % cluster_size;
        cluster_remaining = cluster_size - cluster_offset;

        amount64 = requested < cluster_remaining
                 ? requested
                 : cluster_remaining;

        while (amount64 > 0)
        {
            uint32_t bytes_per_sector =
                fat->bpb.bpb1.bytes_per_sector;

            uint32_t sector_in_cluster =
                (uint32_t)(cluster_offset / bytes_per_sector);

            uint32_t offset_in_sector =
                (uint32_t)(cluster_offset % bytes_per_sector);

            uint32_t absolute_sector =
                clustertosector(cluster) + sector_in_cluster;

            uint8_t sector_buffer[SECTORSIZE];

            uint32_t sector_remaining =
                bytes_per_sector - offset_in_sector;

            uint64_t copy64 =
                amount64 < sector_remaining
                    ? amount64
                    : sector_remaining;

            if (!readsector(absolute_sector, sector_buffer))
            {
                fp->error = true;
                goto read_done;
            }

            memcpy(destination,
                   sector_buffer + offset_in_sector,
                   (size_t)copy64);

            destination += copy64;
            position += copy64;
            requested -= copy64;
            amount64 -= copy64;
            cluster_offset += copy64;
        }
    }

read_done:

    {
        uint64_t bytes_read = position - fp->position;

        fp->position = position;

        if (fp->position >= fp->size)
            fp->eof = true;

        return (size_t)(bytes_read / size);
    }
}

size_t fat_write(const void *ptr,
                 size_t size,
                 size_t nmemb,
                 FAT_FILE *fp)
{
    const uint8_t *source;
    uint64_t requested;
    uint64_t original_position;
    uint64_t end_position;

    if (!fat_file_valid(fp) ||
        !ptr ||
        size == 0 ||
        nmemb == 0)
    {
        return 0;
    }

    if (!fp->writable)
    {
        fp->error = true;
        return 0;
    }

    if (nmemb > SIZE_MAX / size)
    {
        fp->error = true;
        return 0;
    }

    requested = (uint64_t)size * (uint64_t)nmemb;

    if (requested > UINT64_MAX - fp->position)
    {
        fp->error = true;
        return 0;
    }

    original_position = fp->position;
    end_position = fp->position + requested;

    if (fp->position > fp->size)
    {
        uint64_t old_position = fp->position;

        if (!fat_file_ensure_size(fp, fp->position))
        {
            fp->error = true;
            return 0;
        }

        if (!fat_file_zero_range(fp, fp->size, fp->position))
        {
            fp->error = true;
            return 0;
        }

        fp->size = old_position;

        if (!fat_file_update_directory_entry(fp))
        {
            fp->error = true;
            return 0;
        }
    }

    if (end_position > fp->size)
    {
        if (!fat_file_ensure_size(fp, end_position))
        {
            fp->error = true;
            return 0;
        }

        fp->size = end_position;
    }

    source = (const uint8_t *)ptr;

    while (requested > 0)
    {
        uint32_t cluster_size;
        uint32_t cluster;
        uint32_t bytes_per_sector;
        uint64_t cluster_offset;
        uint64_t cluster_remaining;
        uint64_t amount64;

        cluster_size = fat_file_cluster_size();
        bytes_per_sector = fat->bpb.bpb1.bytes_per_sector;

        if (cluster_size == 0 || bytes_per_sector == 0)
        {
            fp->error = true;
            break;
        }

        cluster = fat_file_cluster_at(fp, fp->position);

        if (cluster < 2)
        {
            fp->error = true;
            break;
        }

        cluster_offset = fp->position % cluster_size;
        cluster_remaining = cluster_size - cluster_offset;

        amount64 =
            requested < cluster_remaining
                ? requested
                : cluster_remaining;

        while (amount64 > 0)
        {
            uint32_t sector_in_cluster;
            uint32_t offset_in_sector;
            uint32_t absolute_sector;
            uint32_t sector_remaining;
            uint64_t copy64;
            uint8_t sector_buffer[SECTORSIZE];

            sector_in_cluster =
                (uint32_t)(cluster_offset / bytes_per_sector);

            offset_in_sector =
                (uint32_t)(cluster_offset % bytes_per_sector);

            absolute_sector =
                clustertosector(cluster) + sector_in_cluster;

            sector_remaining =
                bytes_per_sector - offset_in_sector;

            copy64 =
                amount64 < sector_remaining
                    ? amount64
                    : sector_remaining;

            if (offset_in_sector != 0 ||
                copy64 != bytes_per_sector)
            {
                if (!readsector(absolute_sector, sector_buffer))
                {
                    fp->error = true;
                    goto write_done;
                }
            }
            else
            {
                memset(sector_buffer, 0, sizeof(sector_buffer));
            }

            memcpy(sector_buffer + offset_in_sector,
                   source,
                   (size_t)copy64);

            if (!writesector(absolute_sector, sector_buffer))
            {
                fp->error = true;
                goto write_done;
            }

            source += copy64;
            fp->position += copy64;
            requested -= copy64;
            amount64 -= copy64;
            cluster_offset += copy64;
        }
    }

write_done:

    if (fp->position > fp->size)
        fp->size = fp->position;

    if (fp->writable)
    {
        if (!fat_file_update_directory_entry(fp))
        {
            fp->error = true;
        }
    }

    {
        uint64_t written = fp->position - original_position;

        return (size_t)(written / size);
    }
}

static time_t fat_datetime_to_time_t(uint16_t date,
                                     uint16_t time_value)
{
    struct tm tm_value;
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;

    memset(&tm_value, 0, sizeof(tm_value));

    year = 1980 + ((date >> 9) & 0x7F);
    month = (date >> 5) & 0x0F;
    day = date & 0x1F;

    hour = (time_value >> 11) & 0x1F;
    minute = (time_value >> 5) & 0x3F;
    second = (time_value & 0x1F) * 2;

    if (month < 1 || month > 12)
        return (time_t)0;

    if (day < 1 || day > 31)
        return (time_t)0;

    if (hour > 23)
        return (time_t)0;

    if (minute > 59)
        return (time_t)0;

    if (second > 59)
        second = 59;

    tm_value.tm_year = year - 1900;
    tm_value.tm_mon = month - 1;
    tm_value.tm_mday = day;
    tm_value.tm_hour = hour;
    tm_value.tm_min = minute;
    tm_value.tm_sec = second;
    tm_value.tm_isdst = -1;

    return mktime(&tm_value);
}

static time_t fat_access_date_to_time_t(uint16_t date)
{
    return fat_datetime_to_time_t(date, 0);
}

static uint64_t fat_stat_inode(uint32_t directory_sector,
                               uint32_t directory_offset)
{
    return ((uint64_t)directory_sector << 32) |
           (uint64_t)directory_offset;
}

static int fat_stat_from_entry(const file_entry_t *entry,
                               uint32_t directory_sector,
                               uint32_t directory_offset,
                               struct stat *st)
{
    uint32_t first_cluster;
    bool is_directory;
    time_t creation_time;
    time_t access_time;
    time_t write_time;

    if (!entry || !st)
        return -1;

    memset(st, 0, sizeof(*st));

    is_directory =
        (entry->attribute & F_ATTR_DIRECT) != 0;

    first_cluster = getfilefirstcluster(entry);

    if (is_directory)
    {
        if (entry->attribute & F_ATTR_RDONLY)
            st->st_mode = S_IFDIR | 0555;
        else
            st->st_mode = S_IFDIR | 0777;
    }
    else
    {
        if (entry->attribute & F_ATTR_RDONLY)
            st->st_mode = S_IFREG | 0444;
        else
            st->st_mode = S_IFREG | 0666;
    }

    st->st_nlink = 1;

    st->st_dev = 0;
    st->st_uid = 0;
    st->st_gid = 0;

    st->st_size = (off_t)entry->size;

    st->st_ino =
        (ino_t)fat_stat_inode(directory_sector,
                              directory_offset);

    creation_time =
        fat_datetime_to_time_t(entry->creation_date,
                               entry->creation_time);

    access_time =
        fat_access_date_to_time_t(entry->last_date);

    write_time =
        fat_datetime_to_time_t(entry->write_date,
                               entry->write_time);

    st->st_atime = access_time;
    st->st_mtime = write_time;
    st->st_ctime = creation_time;

    (void)first_cluster;

    return 0;
}

int fat_stat(const char *filename, struct stat *st)
{
    found_file_t found;

    if (!filename || !st)
        return -1;

    if (!imagedisk_file ||
        !fat ||
        !hasactive() ||
        !isfattype())
    {
        return -1;
    }

    memset(st, 0, sizeof(*st));
    memset(&found, 0, sizeof(found));

    if (!fat_find_file(filename, &found))
        return -1;

    return fat_stat_from_entry(&found.entry,
                               found.sector,
                               found.offset,
                               st);
}

int fat_fstat(FAT_FILE *fp, struct stat *st)
{
    if (!st)
        return -1;

    if (!fat_file_valid(fp))
        return -1;

    memset(st, 0, sizeof(*st));

    return fat_stat_from_entry(&fp->entry,
                               fp->directory_sector,
                               fp->directory_offset,
                               st);
}

int fat_lstat(const char *filename, struct stat *st)
{
    return fat_stat(filename, st);
}

static bool fat_dir_next_sector(FAT_DIR *dir)
{
    uint32_t sectors_per_cluster;

    if (!fat_dir_valid(dir))
        return false;

    sectors_per_cluster =
        fat->bpb.bpb1.sector_per_cluster;

    if (sectors_per_cluster == 0)
        return false;

    if (dir->root_fat16)
    {
        uint32_t root_start;
        uint32_t root_count;

        root_start = getrootdirsector();
        root_count = getrootdirsectorscount();

        if (root_count == 0)
            return false;

        if (dir->current_sector + 1 <
            root_start + root_count)
        {
            dir->current_sector++;
            dir->current_offset = 0;
            return true;
        }

        dir->eof = true;
        return false;
    }

    dir->sector_in_cluster++;

    if (dir->sector_in_cluster < sectors_per_cluster)
    {
        dir->current_sector =
            clustertosector(dir->current_cluster) +
            dir->sector_in_cluster;

        dir->current_offset = 0;

        return true;
    }

    uint32_t next_cluster;

    next_cluster = (uint32_t)readcluster(dir->current_cluster);

    if (next_cluster == 0 ||
        is_eoc(next_cluster))
    {
        dir->eof = true;
        return false;
    }

    if (!is_valid_data_cluster(next_cluster))
    {
        dir->error = true;
        dir->eof = true;
        return false;
    }

    dir->current_cluster = next_cluster;
    dir->sector_in_cluster = 0;

    dir->current_sector =
        clustertosector(next_cluster);

    if (dir->current_sector == 0)
    {
        dir->error = true;
        dir->eof = true;
        return false;
    }

    dir->current_offset = 0;

    return true;
    
}

static bool fat_dir_validate_lfn(const uint8_t *raw,
                                 size_t count,
                                 const file_entry_t *entry)
{
    uint8_t checksum;
    size_t i;

    if (!raw ||
        !entry ||
        count == 0 ||
        count > LFN_MAX_ENTRIES)
    {
        return false;
    }

    checksum = lfn_checksum((const uint8_t *)entry->name);

    for (i = 0; i < count; ++i)
    {
        lfn_entry_t lfn;

        memcpy(&lfn,
               raw + i * FAT_ENTRY_SIZE,
               sizeof(lfn));

        if (lfn.attribute != F_ATTR_LNGFNM)
            return false;

        if (lfn.type != 0)
            return false;

        if (lfn.first_cluster != 0)
            return false;

        if (lfn.checksum != checksum)
            return false;
    }


    lfn_entry_t first;

    memcpy(&first,
            raw,
            sizeof(first));

    if (!(first.order & LFN_LAST))
        return false;

    if ((first.order & 0x1F) != count)
        return false;

    return true;
}

static bool fat_dir_make_dirent(FAT_DIR *dir,
                                const uint8_t *raw,
                                uint32_t sector,
                                uint32_t offset)
{
    file_entry_t entry;
    char short_name[13];
    char long_name[MAX_FILENAME_LENGTH];

    if (!dir || !raw)
        return false;

    memcpy(&entry,
           raw,
           sizeof(file_entry_t));

    memset(&dir->dirent, 0, sizeof(dir->dirent));

    dir->dirent.entry = entry;

    dir->dirent.directory_sector = sector;
    dir->dirent.directory_offset = offset;

    dir->dirent.first_cluster = getfilefirstcluster(&entry);

    dir->dirent.d_size = entry.size;

    memset(short_name, 0, sizeof(short_name));

    if (!getshortfilename(entry.name,
                          short_name,
                          sizeof(short_name)))
    {
        return false;
    }

    strncpy(dir->dirent.d_short_name,
            short_name,
            sizeof(dir->dirent.d_short_name) - 1);

    if (dir->lfn_count > 0 &&
        fat_dir_validate_lfn(
            dir->lfn_raw,
            dir->lfn_count,
            &entry))
    {
        memset(long_name, 0, sizeof(long_name));

        if (get_lfn_name(
                dir->lfn_raw,
                dir->lfn_count,
                long_name,
                sizeof(long_name)))
        {
            strncpy(dir->dirent.d_name,
                    long_name,
                    sizeof(dir->dirent.d_name) - 1);
        }
    }

    if (dir->dirent.d_name[0] == '\0')
    {
        strncpy(dir->dirent.d_name,
                short_name,
                sizeof(dir->dirent.d_name) - 1);
    }

    if (entry.attribute & F_ATTR_DIRECT)
    {
        dir->dirent.d_type = FAT_DIRENT_DIRECTORY;
    }
    else
    {
        dir->dirent.d_type = FAT_DIRENT_FILE;
    }

    if (entry.attribute & F_ATTR_VOLMID)
        dir->dirent.d_type |= FAT_DIRENT_VOLUME;

    dir->dirent.d_name[
        sizeof(dir->dirent.d_name) - 1] = '\0';

    dir->dirent.d_short_name[
        sizeof(dir->dirent.d_short_name) - 1] = '\0';

    return true;
}

FAT_DIR *fat_opendir(const char *dirname)
{
    FAT_DIR *dir;
    uint32_t sector;
    uint32_t cluster;
    bool root_fat16;

    if (!dirname)
        return NULL;

    if (!imagedisk_file ||
        !fat ||
        !hasactive() ||
        !isfattype())
    {
        return NULL;
    }

    if (!fat_directory_from_path(
            dirname,
            &sector,
            &cluster,
            &root_fat16))
    {
        return NULL;
    }

    dir = (FAT_DIR *)calloc(1, sizeof(FAT_DIR));

    if (!dir)
        return NULL;

    dir->start_sector = sector;

    dir->current_sector = sector;
    dir->current_offset = 0;

    dir->current_cluster = cluster;
    dir->sector_in_cluster = 0;

    dir->root_fat16 = root_fat16;

    dir->eof = false;
    dir->error = false;
    dir->open = true;

    dir->lfn_count = 0;

    memset(&dir->dirent,
           0,
           sizeof(dir->dirent));

    return dir;
}

fat_dirent_t *fat_readdir(FAT_DIR *dir)
{
    uint8_t sector_buffer[SECTORSIZE];

    if (!fat_dir_valid(dir))
        return NULL;

    if (dir->eof)
        return NULL;

    for (;;)
    {
        uint8_t *raw;
        uint32_t offset;

        if (dir->current_offset >= SECTORSIZE)
        {
            if (!fat_dir_next_sector(dir))
                return NULL;
        }

        if (!readsector(dir->current_sector,
                        sector_buffer))
        {
            dir->error = true;
            dir->eof = true;
            return NULL;
        }

        offset = dir->current_offset;

        raw = sector_buffer + offset;

        dir->current_offset += FAT_ENTRY_SIZE;

        if (raw[0] == 0x00)
        {
            dir->lfn_count = 0;
            dir->eof = true;
            return NULL;
        }

        if (raw[0] == FILE_NAME_DELETED)
        {
            dir->lfn_count = 0;
            continue;
        }

        if (raw[11] == F_ATTR_LNGFNM)
        {
            if (dir->lfn_count < LFN_MAX_ENTRIES)
            {
                memcpy(
                    dir->lfn_raw +
                        dir->lfn_count * FAT_ENTRY_SIZE,
                    raw,
                    FAT_ENTRY_SIZE);

                dir->lfn_count++;
            }
            else
            {
                dir->lfn_count = 0;
            }

            continue;
        }

        {
            file_entry_t entry;

            memcpy(&entry,
                   raw,
                   sizeof(entry));

            if (entry.attribute & F_ATTR_VOLMID)
            {
                dir->lfn_count = 0;
                continue;
            }

            if (!fat_dir_make_dirent(
                    dir,
                    raw,
                    dir->current_sector,
                    offset))
            {
                dir->lfn_count = 0;
                continue;
            }

            dir->lfn_count = 0;

            return &dir->dirent;
        }
    }
}

fat_dirent_t *fat_readdir_2(FAT_DIR *dir)
{
    uint8_t sector_buffer[SECTORSIZE];

    if (!fat_dir_valid(dir))
        return NULL;

    if (dir->eof)
        return NULL;

    for (;;)
    {
        uint8_t *raw;
        uint32_t offset;

        if (!readsector(dir->current_sector,
                        sector_buffer))
        {
            dir->error = true;
            dir->eof = true;
            return NULL;
        }

        offset = dir->current_offset;

        if (offset + FAT_ENTRY_SIZE > SECTORSIZE)
        {
            if (!fat_dir_next_sector(dir))
                return NULL;

            continue;
        }

        raw = sector_buffer + offset;

        dir->current_offset += FAT_ENTRY_SIZE;

        if (raw[0] == 0x00)
        {
            dir->lfn_count = 0;
            dir->eof = true;
            return NULL;
        }

        if (raw[0] == FILE_NAME_DELETED)
        {
            dir->lfn_count = 0;
            continue;
        }

        if (raw[11] == F_ATTR_LNGFNM)
        {
            if (dir->lfn_count < LFN_MAX_ENTRIES)
            {
                memcpy(
                    dir->lfn_raw +
                        dir->lfn_count * FAT_ENTRY_SIZE,
                    raw,
                    FAT_ENTRY_SIZE);

                dir->lfn_count++;
            }
            else
            {
                dir->lfn_count = 0;
            }

            continue;
        }

        {
            file_entry_t entry;

            memcpy(&entry,
                   raw,
                   sizeof(entry));

            if (entry.attribute & F_ATTR_VOLMID)
            {
                dir->lfn_count = 0;
                continue;
            }

            if (!fat_dir_make_dirent(
                    dir,
                    raw,
                    dir->current_sector,
                    offset))
            {
                dir->lfn_count = 0;
                continue;
            }

            dir->lfn_count = 0;

            return &dir->dirent;
        }

        if (dir->current_offset >= SECTORSIZE)
        {
            if (!fat_dir_next_sector(dir))
                return NULL;
        }
    }
}

int fat_closedir(FAT_DIR *dir)
{
    if (!dir)
        return -1;

    if (!dir->open)
    {
        free(dir);
        return -1;
    }

    dir->open = false;

    memset(dir->lfn_raw,
           0,
           sizeof(dir->lfn_raw));

    dir->lfn_count = 0;

    memset(&dir->dirent,
           0,
           sizeof(dir->dirent));

    free(dir);

    return 0;
}

int fat_mkdir(const char *path_str)
{
    if (!imagedisk_file ||
        !fat ||
        !hasactive() ||
        !isfattype())
    {
        return -1;
    }

    path_sub_t path = getpath(path_str);
    if (path.pathcount == 0) return -1;

    uint32_t parent_dir_sector = getrootdirsectorstart();
    if (path.pathcount > 1)
    {
        if (!find_path_directory(&path, path.pathcount - 1, &parent_dir_sector))
            return -1;
    }

    found_file_t found;
    if (findfileinsectorfilenumber(parent_dir_sector, path.path[path.pathcount - 1].path, &found))
        return -1; 

    uint32_t new_cluster = allocchain(1);
    if (new_cluster == 0) return -1;

    uint32_t sec_start = getfirstsectorofcluster(new_cluster);
    uint8_t buffer[SECTORSIZE];
    memset(buffer, 0, SECTORSIZE);
    
    for (int i = 0; i < fat->bpb.bpb1.sector_per_cluster; i++)
    {
        writesector(sec_start + i, buffer);
    }

    file_entry_t dot, dotdot;
    memset(&dot, 0, sizeof(file_entry_t));
    memset(&dotdot, 0, sizeof(file_entry_t));
    memset(dot.name, ' ', 11);
    memset(dotdot.name, ' ', 11);
    
    dot.name[0] = '.';
    dotdot.name[0] = '.'; 
    dotdot.name[1] = '.';
    
    dot.attribute = F_ATTR_DIRECT;
    dotdot.attribute = F_ATTR_DIRECT;
    
    dot.first_cluster_hi = (new_cluster >> 16) & 0xFFFF;
    dot.first_cluster_lo = new_cluster & 0xFFFF;
    
    uint32_t parent_cluster = 0;
    if (parent_dir_sector >= getfirstdatasector())
    {
        parent_cluster = getclusterfromsector(parent_dir_sector);
    }
    else if (isfat32type())
    {
        parent_cluster = fat->bpb.bpb2.fat32.root_cluster;
    }
    
    dotdot.first_cluster_hi = (parent_cluster >> 16) & 0xFFFF;
    dotdot.first_cluster_lo = parent_cluster & 0xFFFF;

    set_current_datetime(&dot);
    set_current_datetime(&dotdot);

    memcpy(buffer, &dot, sizeof(file_entry_t));
    memcpy(buffer + sizeof(file_entry_t), &dotdot, sizeof(file_entry_t));
    writesector(sec_start, buffer);

    uint32_t free_sector, free_offset;
    if (!isfat32type() && parent_dir_sector >= getrootdirsector() && parent_dir_sector < getfirstdatasector())
    {
        if (!findfreeslots_root_fat16(1, &free_sector, &free_offset)) return -1;
    }
    else
    {
        if (!findfreeslots(parent_cluster, 1, &free_sector, &free_offset)) return -1;
    }

    file_entry_t new_entry;
    memset(&new_entry, 0, sizeof(file_entry_t));
    uint8_t dos_name[11];
    strfilenamedos(path.path[path.pathcount - 1].path, dos_name);
    
    memcpy(new_entry.name, dos_name, 11);
    new_entry.attribute = F_ATTR_DIRECT;
    new_entry.first_cluster_hi = (new_cluster >> 16) & 0xFFFF;
    new_entry.first_cluster_lo = new_cluster & 0xFFFF;
    set_current_datetime(&new_entry);

    if (!readsector(free_sector, buffer)) return -1;
    memcpy(buffer + free_offset, &new_entry, sizeof(file_entry_t));
    
    return writesector(free_sector, buffer);
}

static bool delete_directory_entry_with_lfn(uint32_t dir_start, found_file_t *found)
{
    uint32_t history_sec[LFN_MAX_ENTRIES];
    uint32_t history_off[LFN_MAX_ENTRIES];
    int h_count = 0;

    uint32_t current_sec;
    uint32_t current_cluster = 0;
    bool is_root_fat16 = (!isfat32type() && dir_start >= getrootdirsector() && dir_start < getfirstdatasector());

    if (is_root_fat16) {
        current_sec = dir_start;
    } else {
        current_cluster = (dir_start >= getfirstdatasector()) ? getclusterfromsector(dir_start) : dir_start;
        if (current_cluster == 0 && isfat32type()) current_cluster = fat->bpb.bpb2.fat32.root_cluster;
        current_sec = getfirstsectorofcluster(current_cluster);
    }

    uint32_t root_sectors = getrootdirsectorscount();
    uint32_t sec_in_cluster = 0;
    uint32_t sec_in_root = 0;

    while (true) {
        uint8_t buffer[SECTORSIZE];
        if (!readsector(current_sec, buffer)) return false;

        for (uint32_t offset = 0; offset < SECTORSIZE; offset += FAT_ENTRY_SIZE) {
            if (current_sec == found->sector && offset == found->offset) {
                buffer[offset] = FILE_NAME_DELETED;
                if (!writesector(current_sec, buffer)) return false;

                uint8_t chksum = lfn_checksum((const uint8_t*)found->entry.name);

                for (int i = h_count - 1; i >= 0; i--) {
                    uint8_t lbuf[SECTORSIZE];
                    if (!readsector(history_sec[i], lbuf)) break;

                    lfn_entry_t *lfn = (lfn_entry_t*)(lbuf + history_off[i]);

                    if (lfn->attribute != F_ATTR_LNGFNM || lfn->checksum != chksum) break;

                    lbuf[history_off[i]] = FILE_NAME_DELETED;
                    writesector(history_sec[i], lbuf);

                    if (lfn->order & LFN_LAST) break;
                }
                return true;
            }

            if (h_count < LFN_MAX_ENTRIES) {
                history_sec[h_count] = current_sec;
                history_off[h_count] = offset;
                h_count++;
            } else {
                memmove(&history_sec[0], &history_sec[1], (LFN_MAX_ENTRIES - 1) * sizeof(uint32_t));
                memmove(&history_off[0], &history_off[1], (LFN_MAX_ENTRIES - 1) * sizeof(uint32_t));
                history_sec[LFN_MAX_ENTRIES - 1] = current_sec;
                history_off[LFN_MAX_ENTRIES - 1] = offset;
            }
        }

        if (is_root_fat16) {
            sec_in_root++;
            if (sec_in_root >= root_sectors) break;
            current_sec = dir_start + sec_in_root;
        } else {
            sec_in_cluster++;
            if (sec_in_cluster >= fat->bpb.bpb1.sector_per_cluster) {
                current_cluster = getnextcluster(current_cluster);
                if (current_cluster == 0 || is_eoc(current_cluster)) break;
                current_sec = getfirstsectorofcluster(current_cluster);
                sec_in_cluster = 0;
            } else {
                current_sec++;
            }
        }
    }
    return false;
}

int fat_unlink(const char *path_str)
{
    if (!imagedisk_file ||
        !fat ||
        !hasactive() ||
        !isfattype())
    {
        return -1;
    }

    path_sub_t path = getpath(path_str);
    if (path.pathcount == 0) return -1;

    uint32_t dir_sector = getrootdirsectorstart();
    if (path.pathcount > 1)
    {
        if (!find_path_directory(&path, path.pathcount - 1, &dir_sector))
            return -1;
    }

    found_file_t found;
    if (!findfileinsectorfilenumber(dir_sector, path.path[path.pathcount - 1].path, &found))
        return -1;

    if (found.entry.attribute & F_ATTR_DIRECT)
        return -1;

    uint32_t cluster = ((uint32_t)found.entry.first_cluster_hi << 16) | found.entry.first_cluster_lo;
    if (cluster >= 2)
        freechain(cluster);

    return delete_directory_entry_with_lfn(dir_sector, &found);
}

int fat_rmdir(const char *path_str)
{
    if (!imagedisk_file ||
        !fat ||
        !hasactive() ||
        !isfattype())
    {
        return -1;
    }

    path_sub_t path = getpath(path_str);
    if (path.pathcount == 0) return -1;

    uint32_t parent_dir_sector = getrootdirsectorstart();
    if (path.pathcount > 1)
    {
        if (!find_path_directory(&path, path.pathcount - 1, &parent_dir_sector))
            return -1;
    }

    found_file_t found;
    if (!findfileinsectorfilenumber(parent_dir_sector, path.path[path.pathcount - 1].path, &found))
        return -1;

    if (!(found.entry.attribute & F_ATTR_DIRECT))
        return -1;

    uint32_t dir_cluster = ((uint32_t)found.entry.first_cluster_hi << 16) | found.entry.first_cluster_lo;

    uint32_t current_cluster = dir_cluster;
    while (is_valid_data_cluster(current_cluster))
    {
        uint32_t sec = getfirstsectorofcluster(current_cluster);
        for (int s = 0; s < fat->bpb.bpb1.sector_per_cluster; s++)
        {
            uint8_t buffer[SECTORSIZE];
            if (!readsector(sec + s, buffer)) return -1;

            for (int i = 0; i < SECTORSIZE; i += FAT_ENTRY_SIZE)
            {
                if (buffer[i] == 0x00) goto empty_check_passed;
                if ((uint8_t)buffer[i] == FILE_NAME_DELETED) continue;

                if (buffer[i] == '.' && buffer[i+1] == ' ') continue;
                if (buffer[i] == '.' && buffer[i+1] == '.' && buffer[i+2] == ' ') continue;

                return -1;
            }
        }
        current_cluster = getnextcluster(current_cluster);
    }

empty_check_passed:
    if (dir_cluster >= 2) freechain(dir_cluster);

    return delete_directory_entry_with_lfn(parent_dir_sector, &found);
}

uint32_t get_free_cluster_count(void)
{
    uint8_t buffer[SECTORSIZE];
    uint32_t cluster;
    uint32_t max_cluster;
    uint32_t free_clusters = 0;

    if (!imagedisk_file || !hasactive() || !isfattype())
        return 0;

    if (isfat32type())
    {
        uint16_t fs_info_sector_offset = fat->bpb.bpb2.fat32.fs_info;

        if (fs_info_sector_offset > 0 && fs_info_sector_offset != 0xFFFF)
        {
            uint32_t fs_info_lba = main_partition->lba_start + fs_info_sector_offset;

            if (readsector(fs_info_lba, buffer))
            {
                fat32_fsinfo_t *fsinfo = (fat32_fsinfo_t *)buffer;

                if (fsinfo->lead_signature == FSINFO_LEAD_SIG &&
                    fsinfo->struct_signature == FSINFO_STRUCT_SIG &&
                    fsinfo->trail_signature == FSINFO_TRAIL_SIG)
                {
                    if (fsinfo->free_cluster_count != FSINFO_UNKNOWN &&
                        fsinfo->free_cluster_count <= getclustercount())
                    {
                        return fsinfo->free_cluster_count;
                    }
                }
            }
        }
    }

    max_cluster = fat_max_cluster();
    for (cluster = 2; cluster <= max_cluster; ++cluster)
    {
        if (readcluster(cluster) == 0)
        {
            free_clusters++;
        }
    }

    return free_clusters;
}

int fat_statfs(const char *path, fat_statfs_t *buf)
{
    (void)path;

    if (!imagedisk_file || !hasactive() || !isfattype() || buf == NULL)
        return -1;

    if (isfat16type())
        buf->f_type = 16;
    else if (isfat32type())
        buf->f_type = 32;
    else
        buf->f_type = 0;

    buf->f_bsize = (uint32_t)fat->bpb.bpb1.bytes_per_sector * fat->bpb.bpb1.sector_per_cluster;
    buf->f_blocks = getclustercount();

    buf->f_bfree = get_free_cluster_count();
    buf->f_bavail = buf->f_bfree;
    buf->f_namelen = MAX_FILENAME_LENGTH;

    return 0;
}

int fat_fstatfs(FAT_FILE *file, fat_statfs_t *buf)
{
    (void)file;
    
    return fat_statfs(NULL, buf);
}

int update_fsinfo(uint32_t free_cluster_count, uint32_t next_free_cluster)
{
    uint32_t fsinfo_sector;
    uint8_t buffer[SECTORSIZE];
    fat32_fsinfo_t *fsinfo;

    if (!imagedisk_file || !hasactive() || !isfat32type())
        return -1;

    if (fat->bpb.bpb2.fat32.fs_info == 0 || fat->bpb.bpb2.fat32.fs_info == 0xFFFF)
        return -1;

    fsinfo_sector = main_partition->lba_start + fat->bpb.bpb2.fat32.fs_info;

    if (!readsector(fsinfo_sector, buffer))
        return -1;

    fsinfo = (fat32_fsinfo_t *)buffer;

    if (fsinfo->lead_signature != FSINFO_LEAD_SIG ||
        fsinfo->struct_signature != FSINFO_STRUCT_SIG ||
        fsinfo->trail_signature != FSINFO_TRAIL_SIG)
    {
        return -1;
    }

    if (free_cluster_count != FSINFO_UNKNOWN)
        fsinfo->free_cluster_count = free_cluster_count;
        
    if (next_free_cluster != FSINFO_UNKNOWN)
        fsinfo->next_free_cluster = next_free_cluster;

    return writesector(fsinfo_sector, buffer);
}

int fat_rename(const char *oldpath, const char *newpath)
{
    if (!imagedisk_file || !hasactive() || !isfat32type())
        return -1;

    found_file_t old_file;
    found_file_t new_file;
    uint32_t old_dir_sector = getrootdirsectorstart();
    uint32_t new_dir_sector = getrootdirsectorstart();
    path_sub_t p_old = getpath(oldpath);
    path_sub_t p_new = getpath(newpath);

    if (p_old.pathcount == 0 || p_new.pathcount == 0) return -1;

    if (p_old.pathcount > 1) {
        if (!find_path_directory(&p_old, p_old.pathcount - 1, &old_dir_sector)) return -1;
    }
    if (!findfileinsectorfilenumber(old_dir_sector, p_old.path[p_old.pathcount - 1].path, &old_file)) return -1;

    if (p_new.pathcount > 1) {
        if (!find_path_directory(&p_new, p_new.pathcount - 1, &new_dir_sector)) return -1;
    }
    if (findfileinsectorfilenumber(new_dir_sector, p_new.path[p_new.pathcount - 1].path, &new_file)) {
        return -1;
    }

    const char *new_filename = p_new.path[p_new.pathcount - 1].path;

    uint16_t utf16_name[LFN_MAX_CHARS];
    size_t utf16_len = utf8_to_utf16(new_filename, utf16_name, LFN_MAX_CHARS);
    size_t req_entries = 1;
    if (utf16_len > 0) {
        req_entries += lfn_entry_count(utf16_len);
    }

    uint32_t new_dir_cluster = getclusterfromsector(new_dir_sector);
    uint32_t found_sector = 0, found_offset = 0;
    bool slots_found = false;

    if (isfat16type() && new_dir_sector >= getrootdirsector() && new_dir_sector < getfirstdatasector()) {
        slots_found = findfreeslots_root_fat16((uint32_t)req_entries, &found_sector, &found_offset);
    } else {
        if (new_dir_cluster == 0 && isfat32type()) {
            new_dir_cluster = getrootdircluster();
        }
        slots_found = findfreeslots(new_dir_cluster, (uint32_t)req_entries, &found_sector, &found_offset);
    }

    if (!slots_found) return -1;

    uint8_t dos_name[11];
    strfilenamedos(new_filename, dos_name);
    uint32_t first_cluster = getfilefirstcluster(&old_file.entry);

    if (!write_file_directory_entries(found_sector, found_offset, new_filename, dos_name, first_cluster, old_file.entry.size)) {
        return -1;
    }

    found_file_t newly_written;
    if (findfileinsectorfilenumber(new_dir_sector, new_filename, &newly_written)) {
        uint8_t new_sec_buf[SECTORSIZE];
        if (readsector(newly_written.sector, new_sec_buf)) {
            file_entry_t *new_entry = (file_entry_t *)(new_sec_buf + newly_written.offset);

            new_entry->attribute = old_file.entry.attribute;
            new_entry->creation_time_tenth = old_file.entry.creation_time_tenth;
            new_entry->creation_time = old_file.entry.creation_time;
            new_entry->creation_date = old_file.entry.creation_date;
            new_entry->last_date = old_file.entry.last_date;
            new_entry->write_time = old_file.entry.write_time;
            new_entry->write_date = old_file.entry.write_date;

            writesector(newly_written.sector, new_sec_buf);
        }
    }

    uint8_t old_sec_buf[SECTORSIZE];
    if (!readsector(old_file.sector, old_sec_buf)) return -1;

    int32_t curr_offset = (int32_t)old_file.offset - FAT_ENTRY_SIZE;
    while (curr_offset >= 0) {
        file_entry_t *prev_entry = (file_entry_t *)(old_sec_buf + curr_offset);
        if (prev_entry->attribute == F_ATTR_LNGFNM) {
            prev_entry->name[0] = (char)FILE_NAME_DELETED;
            curr_offset -= FAT_ENTRY_SIZE;
        } else {
            break;
        }
    }

    old_sec_buf[old_file.offset] = FILE_NAME_DELETED;
    if (!writesector(old_file.sector, old_sec_buf)) return -1;

    return 0;
}

int fat_chmod(const char *path, uint8_t attributes)
{
    if (!imagedisk_file || !hasactive() || !isfat32type())
        return -1;

    found_file_t file;
    uint32_t dir_sector = getrootdirsectorstart();
    path_sub_t p = getpath(path);

    if (p.pathcount == 0) return -1;

    if (p.pathcount > 1) {
        if (!find_path_directory(&p, p.pathcount - 1, &dir_sector)) return -1;
    }

    if (!findfileinsectorfilenumber(dir_sector, p.path[p.pathcount - 1].path, &file)) return -1;

    uint8_t sector_buf[SECTORSIZE];
    if (!readsector(file.sector, sector_buf)) return -1;

    file_entry_t *entry = (file_entry_t *)(sector_buf + file.offset);
    entry->attribute = attributes;

    return writesector(file.sector, sector_buf);
}

int fat_fchmod(FAT_FILE *file, uint8_t attributes)
{
    if (!imagedisk_file || !hasactive() || !isfat32type())
        return -1;

    if (file == NULL || !file->open) return -1;

    uint8_t sector_buf[SECTORSIZE];
    if (!readsector(file->directory_sector, sector_buf)) return -1;

    file_entry_t *entry = (file_entry_t *)(sector_buf + file->directory_offset);
    entry->attribute = attributes;

    if (!writesector(file->directory_sector, sector_buf)) return -1;

    file->entry.attribute = attributes;

    return 0;
}

