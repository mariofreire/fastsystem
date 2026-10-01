// Fast System Directory Entry
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2026 DSP Interactive.

#ifndef _DIRENT_H
#define _DIRENT_H

#define F_ATTR_NORMAL 0x00
#define F_ATTR_RDONLY 0x01
#define F_ATTR_HIDDEN 0x02
#define F_ATTR_SYSTEM 0x04
#define F_ATTR_VOLMID 0x08
#define F_ATTR_DIRECT 0x10
#define F_ATTR_ARCHVE 0x20
#define F_ATTR_LNGFNM 0x0F

#define FAT_ENTRY_SIZE 32

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

#define FILE_NAME_DELETED    0xE5
#define FILE_NAME_DIRECTORY  0x2E

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

#pragma pack(push, 1)

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

#pragma pack(pop)

#endif // _DIRENT_H
