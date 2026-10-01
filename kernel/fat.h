// Fast System File Allocation Table (FAT)
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2026 DSP Interactive.

#ifndef _FAT_H_
#define _FAT_H_

#include <sys/stat.h>
#include "itypes.h"

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

#define FAT_FAT32_FSINFO_ADDRESS                   0x8C00

#ifndef EOF
#define EOF (-1)
#endif

#ifndef SEEK_SET
#define SEEK_SET 0
#endif

#ifndef SEEK_CUR
#define SEEK_CUR 1
#endif

#ifndef SEEK_END
#define SEEK_END 2
#endif

#ifndef DT_UNKNOWN
#define DT_UNKNOWN 0
#endif
#ifndef DT_REG
#define DT_REG 8
#endif
#ifndef DT_DIR
#define DT_DIR 4
#endif
#ifndef DT_VOL
#define DT_VOL 6
#endif
#ifndef DIRENT_NAME_MAX
#define DIRENT_NAME_MAX 1023
#endif

#define MIN_FD 8
#define MAX_FD 1024

#ifndef XATTR_CREATE
#define XATTR_CREATE  0x01
#endif

#ifndef XATTR_REPLACE
#define XATTR_REPLACE 0x02
#endif

#define FAT_XATTR_MAX_ENTRIES 64
#define FAT_XATTR_NAME_MAX    128
#define FAT_XATTR_VALUE_MAX   1024
#define FAT_XATTR_USED 0x01

#pragma pack(push, 1)

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
    unsigned char order;
    unsigned short name1[5];
    unsigned char attribute;
    unsigned char type;
    unsigned char checksum;
    unsigned short name2[6];
    unsigned short first_cluster;
    unsigned short name3[2];
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
    unsigned long sector;
    unsigned long offset;
    unsigned long file_number;
} found_file_t;

typedef struct 
{
    unsigned long lead_signature;
    unsigned char  reserved1[480];
    unsigned long struct_signature;
    unsigned long free_cluster_count;
    unsigned long next_free_cluster;
    unsigned char  reserved2[12];
    unsigned long trail_signature;
} fat32_fsinfo_t;

struct dirent
{
    unsigned long d_ino;
    unsigned long d_off;
    unsigned short d_reclen;
    unsigned char d_type;
    char d_name[1];
};

struct sys_dirent 
{
    unsigned long d_ino;
    unsigned long d_off;
    unsigned short d_reclen;
    char d_name[];
};

#pragma pack(pop)

typedef struct
{
    file_entry_t entry;

    unsigned long directory_sector;
    unsigned long directory_offset;

    unsigned long first_cluster;

    unsigned long position;

    unsigned long size;

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

    unsigned long directory_sector;
    unsigned long directory_offset;

    unsigned long first_cluster;

    char d_name[MAX_FILENAME_LENGTH];
    char d_short_name[13];

    unsigned char d_type;

    unsigned long d_size;
} fat_dirent_t;

typedef struct
{
    unsigned long start_sector;

    unsigned long current_sector;
    unsigned long current_offset;

    unsigned long current_cluster;
    unsigned long sector_in_cluster;

    bool root_fat16;
    bool eof;
    bool error;
    bool open;

    unsigned char lfn_raw[LFN_MAX_ENTRIES * FAT_ENTRY_SIZE];
    size_t lfn_count;

    fat_dirent_t dirent;
} FAT_DIR;

typedef struct 
{
    unsigned long f_type;
    unsigned long f_bsize;
    unsigned long f_blocks;
    unsigned long f_bfree;
    unsigned long f_bavail;
    unsigned long f_namelen;
} fat_statfs_t;

typedef enum 
{
    FD_TYPE_NONE = 0,
    FD_TYPE_FILE,
    FD_TYPE_DIR
} fd_type_t;

typedef struct 
{
    fd_type_t type;
    union {
        FAT_FILE *file;
        FAT_DIR *dir;
    } handle;
} fd_entry_t;

typedef struct
{
    unsigned char used;
    char path[MAX_PATH_LENGTH];
    char name[FAT_XATTR_NAME_MAX];
    unsigned char value[FAT_XATTR_VALUE_MAX];
    size_t size;
} fat_xattr_entry_t;

extern mbr_t *mbr;
extern fat_t *fat;

extern int active_partition;

extern partition_entry_t *partition;
extern partition_entry_t *main_partition;

extern unsigned char bootstrap[FAT32_BOOTSTRAP_SIZE];

extern unsigned char *fat32_fsinfo;
extern fat32_fsinfo_t *fsinfo;

extern file_entry_t file_dir_sector[16];

bool initdisk(void);
bool uninitdisk(void);
bool fileexists(const char *filename);
unsigned long filesize(const char *filename);
bool readsector(unsigned long sector, unsigned char *buffer);
bool writesector(unsigned long sector, const unsigned char *buffer);
unsigned long sectortobytes(unsigned long sector);
unsigned long bytestosector(unsigned long bytes);
unsigned long filesizeondisk(unsigned long size);
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
bool has_partition_active(void);
unsigned long getfatsize(void);
unsigned long getrootdirsector(void);
unsigned long getrootdirsectorscount(void);
unsigned long getrootdircluster(void);
unsigned long getrootdirsectorstart(void);
unsigned long getfirstdatasector(void);
unsigned long getdatasector(void);
unsigned long getdatasectorcount(void);
unsigned long getsectornumber(unsigned long sector);
unsigned long getentryoffset(unsigned long sector);
unsigned long getfatentrysize(void);
unsigned long getfatsector(unsigned long cluster);
unsigned long getfatentryoffset(unsigned long cluster);
unsigned long clustertosector(unsigned long cluster);
unsigned long getclusterfromsector(unsigned long sector);
unsigned long getfirstsectorofcluster(unsigned long cluster);
unsigned long getclustercount(void);
unsigned long readcluster(unsigned long cluster);
bool writecluster(unsigned long cluster, unsigned long value);
unsigned long getnextcluster(unsigned long cluster);
bool islastcluster(unsigned long cluster);
unsigned long fat_eoc_mark(void);
unsigned long fat_max_cluster(void);
bool is_valid_data_cluster(unsigned long cluster);
bool is_eoc(unsigned long cluster);
unsigned long neededcluster(unsigned long size);
bool freechain(unsigned long first_cluster);
unsigned long allocchain(unsigned long count);
unsigned long chain_length(unsigned long first_cluster);
unsigned long get_nth_cluster(unsigned long first_cluster, unsigned long index);
bool resizechain(unsigned long *first_cluster, unsigned long required);
file_entry_t *getfileentryofcluster(unsigned long cluster);
file_entry_t *getfileentryofsector(unsigned long sector);
file_entry_t *findfileinsector(unsigned long sector, char *filename);
file_entry_t* findfile(unsigned long sector, const char *filename);
bool getfileentriesofsector(unsigned long sector, file_entry_t entries[16]);
unsigned long getfilefirstcluster(const file_entry_t *entry);
unsigned char qlistdir(const char *filename);
unsigned char listdir(const char *filename);
unsigned long listdironsector(unsigned long sector);
bool findfileinsectorfilenumber(unsigned long directory_start, const char *filename, found_file_t *result);
bool find_path_directory(const path_sub_t *path, int component_count, unsigned long *directory_sector);
path_sub_t getpath(const char *path);
void strfilenamedos(const char *source, unsigned char destination[11]);
bool getshortfilename(const char *name11, char *output, size_t output_size);
bool names_equal(const char *a, const char *b);
unsigned char lfn_checksum(const unsigned char *name);
size_t utf8_to_utf16(const char *src, unsigned short *dst, size_t max_units);
size_t lfn_entry_count(size_t utf16_length);
bool utf16_to_utf8(const unsigned short *src, size_t count, char *output, size_t output_size);
bool get_lfn_name(const unsigned char *raw_entries, size_t entry_count, char *output, size_t output_size);
unsigned char getlongfilename(char *filename, unsigned long sector);
bool findfreeslots_root_fat16(unsigned long required_entries, unsigned long *sector_found, unsigned long *offset_found);
bool findfreeslots(unsigned long dir_cluster, unsigned long required_entries, unsigned long *sector_found, unsigned long *offset_found);
void set_current_datetime(file_entry_t *entry);
bool write_lfn_entry(unsigned char *buffer, unsigned long offset, unsigned char order, const unsigned short *name, 
size_t name_length, unsigned char checksum);
bool write_file_directory_entries(unsigned long sector, unsigned long offset, const char *long_name, 
const unsigned char dos_name[11], unsigned long cluster, unsigned long file_size);
unsigned long getfilesize(const char *filename);
unsigned long getfilesizeondisk(const char *filename);
unsigned long getfilesizeonsector(const char *filename, unsigned long sector);
unsigned long getfilesizeondiskonsector(const char *filename, unsigned long sector);
unsigned char getfiledataonsector(const char *filename, unsigned long sector, unsigned char *data);
unsigned char getfiledata(const char *filename, unsigned char *data);

FAT_FILE *fat_open(const char *filename, const char *mode);
int       fat_close(FAT_FILE *fp);
int       fat_seek(FAT_FILE *fp, off_t offset, int whence);
off_t     fat_tell(FAT_FILE *fp);
void      fat_rewind(FAT_FILE *fp);
size_t    fat_read(void *ptr, size_t size, size_t nmemb, FAT_FILE *fp);
size_t    fat_write(const void *ptr, size_t size, size_t nmemb, FAT_FILE *fp);
int       fat_eof(FAT_FILE *fp);
int       fat_fchmod(FAT_FILE *file, unsigned char attributes);
int       fat_chmod(const char *path, unsigned char attributes);
int       fat_rename(const char *oldpath, const char *newpath);
int       fat_statfs(const char *path, fat_statfs_t *buf);
int       fat_fstatfs(FAT_FILE *file, fat_statfs_t *buf);
int       fat_stat(const char *filename, struct stat *st);
int       fat_fstat(FAT_FILE *fp, struct stat *st);
int       fat_lstat(const char *filename, struct stat *st);
int       fat_unlink(const char *path_str);
int       fat_mkdir(const char *path_str);
int       fat_rmdir(const char *path_str);
int       fat_chdir(const char *path);
char     *fat_getcwd(char *buf, size_t size);
long      fat_setxattr(const char *path, const char *name, const void *value, size_t size, int flags);
long      fat_getxattr(const char *path, const char *name, void *value, size_t size);
int       fat_removexattr(const char *path, const char *name);
int       fat_getdents_dir(const char *dirname, void *buffer, unsigned long count);
FAT_DIR  *fat_opendir(const char *dirname);
int       fat_closedir(FAT_DIR *dir);
fat_dirent_t *fat_readdir(FAT_DIR *dir);

unsigned long get_free_cluster_count(void);
int update_fsinfo(unsigned long free_cluster_count, unsigned long next_free_cluster);

bool direxists(const char *path_str);

void remap_mbr(void);
void remap_fat(void);
void remap_boot(void);

int sys_chmod_handler(const char *path, unsigned char attributes);
int sys_unlink_handler(const char *path);
int sys_rename_handler(const char *oldpath, const char *newpath);
int sys_stat_handler(const char *filename, struct stat *st);
int sys_statfs_handler(const char *path, fat_statfs_t *buf);
int sys_lstat_handler(const char *filename, struct stat *st);
int sys_chdir_handler(const char *path);
int sys_mkdir_handler(const char *path);
int sys_rmdir_handler(const char *path);
char *sys_getcwd_handler(char *buf, size_t size);
int sys_open_handler(const char *pathname, int flags);
int sys_close_handler(int fd);
off_t sys_lseek_handler(int fd, off_t offset, int whence);
size_t sys_read_handler(int fd, void *buf, size_t count);
size_t sys_write_handler(int fd, const void *buf, size_t count);
int sys_getdents_handler(int fd, struct sys_dirent *dirp, unsigned int count);
long sys_setxattr_handler(const char *path, const char *name, const void *value, size_t size, int flags);
long sys_getxattr_handler(const char *path, const char *name, void *value, size_t size);
int sys_removexattr_handler(const char *path, const char *name);

#endif // _FAT_H_
