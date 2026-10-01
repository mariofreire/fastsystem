// Fast System System Stat
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2026 DSP Interactive.

#ifndef _SYS_STAT_H
#define _SYS_STAT_H

#include <sys/types.h>

#define S_IFMT  0170000
#define S_IFDIR 0040000
#define S_IFREG 0100000
#define S_IRUSR 0000400
#define S_IWUSR 0000200
#define S_IXUSR 0000100

struct stat 
{
    unsigned st_dev;
    unsigned st_ino;
    mode_t   st_mode;
    unsigned st_nlink;
    uid_t    st_uid;
    gid_t    st_gid;
    unsigned st_rdev;
    off_t    st_size;
    time_t   st_atime;
    time_t   st_mtime;
    time_t   st_ctime;
};

typedef struct
{
	unsigned long f_type;
	unsigned long f_bsize;
	unsigned long f_blocks;
	unsigned long f_bfree;
	unsigned long f_bavail;
	unsigned long f_files;
	unsigned long f_ffree;
} fat_statfs_t;

int stat(const char *path, struct stat *buf);
int lstat(const char *path, struct stat *buf);
int statfs(const char *path, fat_statfs_t *buf);
int chmod(const char *path, unsigned char attributes);
int mkdir(const char *path);
int rmdir(const char *path);

#endif // _SYS_STAT_H
