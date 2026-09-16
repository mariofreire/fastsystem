#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <limits.h>

#ifdef _WIN32
#include <io.h>
#define fseeko _fseeki64
#define ftello _ftelli64
typedef __int64 off_t;
#else
#include <sys/types.h>
#endif

#define SECTORSIZE 512

#define PARTITION_ACTIVE 0x80
#define PARTITION_INACTIVE 0x00

#define MAX_PARTITION 4

#define BOOT_SIGNATURE 0xAA55

#define PARTITION_FAT16_LESS_32MB 0x04
#define PARTITION_FAT16 0x06
#define PARTITION_FAT16_LBA 0x0E
#define PARTITION_FAT32 0x0B
#define PARTITION_FAT32_LBA 0x0C

#define MBR_BOOTSTRAP_SIZE 0x1BE
#define FAT32_BOOTSTRAP_SIZE 0x1A4
//#define FAT16_BOOTSTRAP_SIZE 0x1C0
#define FAT16_BOOTSTRAP_SIZE 0x1C

#define FAT_ENTRY_SIZE 32

#define FAT12_EOC_MARK 0x0FF8
#define FAT16_EOC_MARK 0xFFF8
#define FAT32_EOC_MARK 0x0FFFFFF8U

#define FAT16_CHAIN_MARK 0xFFF0
#define FAT16_CHAIN_MASK 0xFFFF
#define FAT32_CHAIN_MARK 0x0FFFFFF0U
#define FAT32_CHAIN_MASK 0x0FFFFFFFU

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

#define MAX_PATH_COMPONENTS 32
#define MAX_PATH_LENGTH 256
#define MAX_FILENAME_LENGTH 1024

#define LFN_LAST 0x40
#define LFN_MAX_ENTRIES 20
#define LFN_MAX_CHARS 255

#pragma pack(push, 1)

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
    uint8_t order;
    uint16_t name1[5];
    uint8_t attribute;
    uint8_t type;
    uint8_t checksum;
    uint16_t name2[6];
    uint16_t first_cluster;
    uint16_t name3[2];
} lfn_entry_t;

typedef struct
{
    char path[MAX_PATH_LENGTH];
} path_t;

typedef struct
{
    path_t path[MAX_PATH_COMPONENTS];
    int pathcount;
} path_sub_t;

typedef struct
{
    file_entry_t entry;
    uint32_t sector;
    uint32_t offset;
    uint32_t file_number;
} found_file_t;

#pragma pack(pop)

FILE *imagedisk_file = NULL;
uint64_t imagedisk_size = 0;
uint8_t MBR[SECTORSIZE];
uint8_t BOOT[SECTORSIZE];
mbr_t *mbr = NULL;
fat_t *fat = NULL;
int active_partition = -1;
partition_entry_t *partition = NULL;
partition_entry_t *main_partition = NULL;

bool hasactive(void);
bool haspartition(void);
bool isfat16type(void);
bool isfat32type(void);
bool isfattype(void);
uint32_t getfirstdatasector(void);
uint32_t clustertosector(uint32_t cluster);
uint32_t getclusterfromsector(uint32_t sector);
unsigned long readcluster(unsigned long cluster);
bool writecluster(unsigned long cluster, unsigned long value);

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

int main(int argc, char *argv[])
{
    const char *image_name;
    const char *source_name;
    const char *destination_name;
    const char *final_name;
    FILE *source = NULL;
    uint64_t file_size;
    uint32_t required_clusters;
    uint32_t first_cluster;
    path_sub_t path;
    uint32_t directory_sector;
    found_file_t existing;
    uint8_t dos_name[11];
    bool file_exists_on_disk = false;
    uint16_t utf16_name[LFN_MAX_CHARS];
    size_t utf16_length;
    size_t lfn_count;
    size_t directory_entries_needed;

    if (argc != 4)
    {
        printf("FAT16/FAT32 File Put\n");
        printf("   Created by Mario Freire\n");
        printf("\n");
        printf("Usage: fput [image-file] [source-file-name] [destination-file-name]\n");
        printf("Example: fput harddisk.img loader loader\n");
        printf("\n");
        return 1;
    }

    image_name = argv[1];
    source_name = argv[2];
    destination_name = argv[3];

    source = fopen(source_name, "rb");

    if (!source)
    {
        fprintf(stderr, "Error: Cannot open source file: %s\n", source_name);
        return 1;
    }

    if (fseeko(source, 0, SEEK_END) != 0)
    {
        fprintf(stderr, "Error: Cannot seek source file.\n");
        fclose(source);
        return 1;
    }

    {
        off_t size = ftello(source);

        if (size < 0)
        {
            fprintf(stderr, "Error: Cannot determine source size.\n");
            fclose(source);
            return 1;
        }

        file_size = (uint64_t)size;
    }

    rewind(source);

    if (file_size > UINT32_MAX)
    {
        fprintf(stderr, "Error: FAT directory entries support files up to 4 GiB.\n");
        fclose(source);
        return 1;
    }

    if (!initimagedisk(image_name))
    {
        fprintf(stderr, "Error: Cannot initialize disk image.\n");
        fclose(source);
        return 1;
    }

    if (!hasactive())
    {
        fprintf(stderr, "Error: No active FAT partition found.\n");
        uninitimagedisk();
        fclose(source);
        return 1;
    }

    if (!isfattype())
    {
        fprintf(stderr, "Error: Partition is not FAT16/FAT32.\n");
        uninitimagedisk();
        fclose(source);
        return 1;
    }

    if (!loadfat())
    {
        fprintf(stderr, "Error: Invalid FAT boot sector.\n");
        uninitimagedisk();
        fclose(source);
        return 1;
    }

    path = getpath(destination_name);

    if (path.pathcount == 0)
    {
        fprintf(stderr, "Error: Invalid destination filename.\n");
        uninitimagedisk();
        fclose(source);
        return 1;
    }

    final_name = path.path[path.pathcount - 1].path;

    utf16_length = utf8_to_utf16(final_name, utf16_name, LFN_MAX_CHARS);

    if (utf16_length == 0)
    {
        fprintf(stderr, "Error: Invalid UTF-8 filename.\n");
        uninitimagedisk();
        fclose(source);
        return 1;
    }

    if (utf16_length > LFN_MAX_CHARS)
    {
        fprintf(stderr, "Error: Filename is longer than 255 UTF-16 characters.\n");
        uninitimagedisk();
        fclose(source);
        return 1;
    }

    lfn_count = lfn_entry_count(utf16_length);

    if (lfn_count == 0 || lfn_count > LFN_MAX_ENTRIES)
    {
        fprintf(stderr, "Error: Invalid LFN length.\n");
        uninitimagedisk();
        fclose(source);
        return 1;
    }

    directory_entries_needed = lfn_count + 1;

    strfilenamedos(final_name, dos_name);

    if (path.pathcount > 1)
    {
        if (!find_path_directory(&path, path.pathcount - 1, &directory_sector))
        {
            fprintf(stderr, "Error: Destination directory not found.\n");
            uninitimagedisk();
            fclose(source);
            return 1;
        }
    }
    else
    {
        directory_sector = getrootdirsectorstart();
    }

    if (directory_sector == 0)
    {
        fprintf(stderr, "Error: Invalid destination directory sector.\n");
        uninitimagedisk();
        fclose(source);
        return 1;
    }

    if (findfileinsectorfilenumber(directory_sector, final_name, &existing))
        file_exists_on_disk = true;

    required_clusters = (uint32_t)neededcluster(file_size);

    if (file_size != 0 && required_clusters == 0)
    {
        fprintf(stderr, "Error: Cannot determine required clusters.\n");
        uninitimagedisk();
        fclose(source);
        return 1;
    }

    first_cluster = 0;

    if (file_exists_on_disk)
    {
        first_cluster = getfilefirstcluster(&existing.entry);

        if (existing.entry.size != 0 && first_cluster < 2)
        {
            fprintf(stderr, "Error: Existing file has an invalid cluster chain.\n");
            fclose(source);
            uninitimagedisk();
            return 1;
        }

        if (existing.entry.size == 0)
            first_cluster = 0;

        if (!resizechain(&first_cluster, required_clusters))
        {
            fprintf(stderr, "Error: Cannot resize existing file chain.\n");
            fclose(source);
            uninitimagedisk();
            return 1;
        }

        rewind(source);

        if (!write_file_data(source, file_size, first_cluster))
        {
            fprintf(stderr, "Error: Failed to write file data.\n");
            fclose(source);
            uninitimagedisk();
            return 1;
        }

        {
            uint8_t buffer[SECTORSIZE];
            file_entry_t entry;

            if (!readsector(existing.sector, buffer))
            {
                fprintf(stderr, "Error: Cannot read existing directory entry.\n");
                fclose(source);
                uninitimagedisk();
                return 1;
            }

            memcpy(&entry, buffer + existing.offset, sizeof(entry));
            entry.size = (uint32_t)file_size;

            if (isfat32type())
                entry.first_cluster_hi = (uint16_t)(first_cluster >> 16);
            else
                entry.first_cluster_hi = 0;

            entry.first_cluster_lo = (uint16_t)(first_cluster & 0xFFFF);
            set_current_datetime(&entry);

            memcpy(buffer + existing.offset, &entry, sizeof(entry));

            if (!writesector(existing.sector, buffer))
            {
                fprintf(stderr, "Error: Failed to update directory entry.\n");
                fclose(source);
                uninitimagedisk();
                return 1;
            }
        }
    }
    else
    {
        if (required_clusters != 0)
        {
            first_cluster = (uint32_t)allocchain(required_clusters);

            if (first_cluster == 0)
            {
                fprintf(stderr, "Error: Not enough free clusters.\n");
                uninitimagedisk();
                fclose(source);
                return 1;
            }
        }

        rewind(source);

        if (!write_file_data(source, file_size, first_cluster))
        {
            if (first_cluster != 0)
                freechain(first_cluster);

            fprintf(stderr, "Error: Failed to write file data.\n");
            fclose(source);
            uninitimagedisk();
            return 1;
        }

        {
            uint32_t free_sector = 0;
            uint32_t free_offset = 0;

            if (isfat16type() && directory_sector == getrootdirsector())
            {
                if (!findfreeslots_root_fat16((uint32_t)directory_entries_needed, &free_sector, &free_offset))
                {
                    if (first_cluster != 0)
                        freechain(first_cluster);

                    fprintf(stderr, "Error: Root directory does not have enough contiguous entries.\n");
                    fclose(source);
                    uninitimagedisk();
                    return 1;
                }
            }
            else
            {
                uint32_t directory_cluster = getclusterfromsector(directory_sector);

                if (isfat32type() && directory_sector == getrootdirsectorstart())
                    directory_cluster = getrootdircluster();

                if (directory_cluster < 2)
                {
                    if (first_cluster != 0)
                        freechain(first_cluster);

                    fprintf(stderr, "Error: Invalid destination directory cluster.\n");
                    fclose(source);
                    uninitimagedisk();
                    return 1;
                }

                if (!findfreeslots(directory_cluster, (uint32_t)directory_entries_needed, &free_sector, &free_offset))
                {
                    if (first_cluster != 0)
                        freechain(first_cluster);

                    fprintf(stderr, "Error: No contiguous directory space for LFN.\n");
                    fclose(source);
                    uninitimagedisk();
                    return 1;
                }
            }

            if (!write_file_directory_entries(free_sector, free_offset, final_name, dos_name, first_cluster, (uint32_t)file_size))
            {
                if (first_cluster != 0)
                    freechain(first_cluster);

                fprintf(stderr, "Error: Cannot write LFN directory entries.\n");
                fclose(source);
                uninitimagedisk();
                return 1;
            }
        }
    }

    printf("Put file successfully.\n");

    fclose(source);
    uninitimagedisk();

    return 0;
}
