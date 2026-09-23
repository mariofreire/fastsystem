#ifndef _FATLIB_H_
#define _FATLIB_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <io.h>
#define fseeko _fseeki64
#define ftello _ftelli64
typedef __int64 off_t;
#endif

#define SECTORSIZE 512

#define PARTITION_ACTIVE   0x80
#define PARTITION_INACTIVE 0x00

#define MAX_PARTITION 4

#define BOOT_SIGNATURE 0xAA55

#define PARTITION_FAT16_LESS_32MB  0x04
#define PARTITION_FAT16            0x06
#define PARTITION_FAT16_LBA        0x0E
#define PARTITION_FAT32            0x0B
#define PARTITION_FAT32_LBA        0x0C

#define MBR_BOOTSTRAP_SIZE  0x1BE
#define FAT32_BOOTSTRAP_SIZE 0x1A4
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

#define FILE_NAME_DELETED    0xE5
#define FILE_NAME_DIRECTORY  0x2E

#define MAX_PATH_COMPONENTS 32
#define MAX_PATH_LENGTH     256
#define MAX_FILENAME_LENGTH 1024

#define LFN_LAST       0x40
#define LFN_MAX_ENTRIES 20
#define LFN_MAX_CHARS   255

#define FAT_DIRENT_FILE      0x01
#define FAT_DIRENT_DIRECTORY 0x02
#define FAT_DIRENT_VOLUME    0x04
#define FAT_DIRENT_LFN       0x08

#define FSINFO_LEAD_SIG   0x41615252
#define FSINFO_STRUCT_SIG 0x61417272
#define FSINFO_TRAIL_SIG  0xAA550000
#define FSINFO_UNKNOWN    0xFFFFFFFF

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

typedef struct 
{
    uint32_t lead_signature;
    uint8_t  reserved1[480];
    uint32_t struct_signature;
    uint32_t free_cluster_count;
    uint32_t next_free_cluster;
    uint8_t  reserved2[12];
    uint32_t trail_signature;
} fat32_fsinfo_t;

#pragma pack(pop)

typedef struct
{
    file_entry_t entry;

    uint32_t directory_sector;
    uint32_t directory_offset;

    uint32_t first_cluster;

    uint64_t position;

    uint64_t size;

    bool readable;
    bool writable;
    bool append;

    bool eof;

    bool error;

    bool open;
} FAT_FILE;

typedef struct
{
    file_entry_t entry;

    uint32_t directory_sector;
    uint32_t directory_offset;

    uint32_t first_cluster;

    char d_name[MAX_FILENAME_LENGTH];
    char d_short_name[13];

    uint8_t d_type;

    uint64_t d_size;
} fat_dirent_t;

typedef struct
{
    uint32_t start_sector;

    uint32_t current_sector;
    uint32_t current_offset;

    uint32_t current_cluster;
    uint32_t sector_in_cluster;

    bool root_fat16;
    bool eof;
    bool error;
    bool open;

    uint8_t lfn_raw[LFN_MAX_ENTRIES * FAT_ENTRY_SIZE];
    size_t lfn_count;

    fat_dirent_t dirent;
} FAT_DIR;

typedef struct 
{
    uint32_t f_type;
    uint32_t f_bsize;
    uint32_t f_blocks;
    uint32_t f_bfree;
    uint32_t f_bavail;
    uint32_t f_namelen;
} fat_statfs_t;

extern FILE *imagedisk_file;
extern uint64_t imagedisk_size;

extern uint8_t MBR[SECTORSIZE];
extern uint8_t BOOT[SECTORSIZE];

extern mbr_t *mbr;
extern fat_t *fat;

extern int active_partition;

extern partition_entry_t *partition;
extern partition_entry_t *main_partition;

bool initimagedisk(const char *filename);
bool uninitimagedisk(void);
bool fileexists(const char *filename);
uint64_t filesize(const char *filename);
bool readsector(uint32_t sector, uint8_t *buffer);
bool writesector(uint32_t sector, const uint8_t *buffer);
uint64_t sectortobytes(uint32_t sector);
uint32_t bytestosector(uint64_t bytes);
uint64_t filesizeondisk(uint64_t size);
bool loadmbr(void);
bool loadfat(void);
bool hasactive(void);
bool haspartition(void);
bool isfat16type(void);
bool isfat32type(void);
bool isfattype(void);
bool hasfat16lba(void);
bool hasfat32lba(void);
bool hasfatlba(void);
uint32_t getfatsize(void);
uint32_t getrootdirsector(void);
uint32_t getrootdirsectorscount(void);
uint32_t getrootdircluster(void);
uint32_t getrootdirsectorstart(void);
uint32_t getfirstdatasector(void);
uint32_t getdatasector(void);
uint32_t getdatasectorcount(void);
uint32_t getsectornumber(uint32_t sector);
uint32_t getentryoffset(uint32_t sector);
uint32_t getfatentrysize(void);
uint32_t getfatsector(uint32_t cluster);
uint32_t getfatentryoffset(uint32_t cluster);
uint32_t clustertosector(uint32_t cluster);
uint32_t getclusterfromsector(uint32_t sector);
uint32_t getfirstsectorofcluster(uint32_t cluster);
uint32_t getclustercount(void);
unsigned long readcluster(unsigned long cluster);
bool writecluster(unsigned long cluster, unsigned long value);
uint32_t getnextcluster(uint32_t cluster);
bool islastcluster(uint32_t cluster);
uint32_t fat_eoc_mark(void);
uint32_t fat_max_cluster(void);
bool is_valid_data_cluster(uint32_t cluster);
bool is_eoc(uint32_t cluster);
unsigned long neededcluster(uint64_t size);
bool freechain(uint32_t first_cluster);
unsigned long allocchain(unsigned long count);
uint32_t chain_length(uint32_t first_cluster);
uint32_t get_nth_cluster(uint32_t first_cluster, uint32_t index);
bool resizechain(uint32_t *first_cluster, uint32_t required);
file_entry_t *getfileentryofcluster(uint32_t cluster);
file_entry_t *getfileentryofsector(uint32_t sector);
file_entry_t *findfileinsector(uint32_t sector, char *filename);
file_entry_t* findfile(uint32_t sector, const char *filename);
bool getfileentriesofsector(uint32_t sector, file_entry_t entries[16]);
uint32_t getfilefirstcluster(const file_entry_t *entry);
uint32_t listdir(uint32_t sector);
bool findfileinsectorfilenumber(uint32_t directory_start, const char *filename, found_file_t *result);
bool find_path_directory(const path_sub_t *path, int component_count, uint32_t *directory_sector);
path_sub_t getpath(const char *path);
void strfilenamedos(const char *source, uint8_t destination[11]);
bool getshortfilename(const char *name11, char *output, size_t output_size);
bool names_equal(const char *a, const char *b);
uint8_t lfn_checksum(const uint8_t *name);
size_t utf8_to_utf16(const char *src, uint16_t *dst, size_t max_units);
size_t lfn_entry_count(size_t utf16_length);
bool utf16_to_utf8(const uint16_t *src, size_t count, char *output, size_t output_size);
bool get_lfn_name(const uint8_t *raw_entries, size_t entry_count, char *output, size_t output_size);
uint8_t getlongfilename(char *filename, uint32_t sector);
bool findfreeslots_root_fat16(uint32_t required_entries, uint32_t *sector_found, uint32_t *offset_found);
bool findfreeslots(uint32_t dir_cluster, uint32_t required_entries, uint32_t *sector_found, uint32_t *offset_found);
void set_current_datetime(file_entry_t *entry);
bool write_lfn_entry(uint8_t *buffer, uint32_t offset, uint8_t order, const uint16_t *name, 
size_t name_length, uint8_t checksum);
bool write_file_directory_entries(uint32_t sector, uint32_t offset, const char *long_name, 
const uint8_t dos_name[11], uint32_t cluster, uint32_t file_size);
bool write_file_data(FILE *source, uint64_t file_size, uint32_t first_cluster);
unsigned long getfilesize(const char *filename, unsigned long sector);
unsigned long getfilesizeondisk(const char *filename, unsigned long sector);
unsigned char getfiledata(const char *filename, unsigned long sector, unsigned char *data);

FAT_FILE *fat_open(const char *filename, const char *mode);
int       fat_close(FAT_FILE *fp);
int       fat_seek(FAT_FILE *fp, off_t offset, int whence);
off_t     fat_tell(FAT_FILE *fp);
void      fat_rewind(FAT_FILE *fp);
size_t    fat_read(void *ptr, size_t size, size_t nmemb, FAT_FILE *fp);
size_t    fat_write(const void *ptr, size_t size, size_t nmemb, FAT_FILE *fp);
int       fat_eof(FAT_FILE *fp);
int       fat_fchmod(FAT_FILE *file, uint8_t attributes);
int       fat_chmod(const char *path, uint8_t attributes);
int       fat_rename(const char *oldpath, const char *newpath);
int       fat_statfs(const char *path, fat_statfs_t *buf);
int       fat_fstatfs(FAT_FILE *file, fat_statfs_t *buf);
int       fat_stat(const char *filename, struct stat *st);
int       fat_fstat(FAT_FILE *fp, struct stat *st);
int       fat_lstat(const char *filename, struct stat *st);
int       fat_unlink(const char *path_str);
int       fat_mkdir(const char *path_str);
int       fat_rmdir(const char *path_str);
FAT_DIR  *fat_opendir(const char *dirname);
int       fat_closedir(FAT_DIR *dir);
fat_dirent_t *fat_readdir(FAT_DIR *dir);

uint32_t get_free_cluster_count(void);
int update_fsinfo(uint32_t free_cluster_count, uint32_t next_free_cluster);

#endif // _FATLIB_H_
