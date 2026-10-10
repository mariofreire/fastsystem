// Fast System Virtual File System
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2026 DSP Interactive.

#ifndef __VFS_H__
#define __VFS_H__

#include "itypes.h"

struct stat;

#define VFS_NAME_MAX   32
#define VFS_PATH_MAX   256
#define VFS_MAX_FS     8
#define VFS_MAX_MOUNTS 16
#define VFS_FD_MAX     1024

#define VFS_MNT_RDONLY 0x0001

struct vfs_mount;

typedef struct vfs_ops
{
    int    (*open)(struct vfs_mount *mnt, const char *path, int flags);
    int    (*close)(struct vfs_mount *mnt, int fd);
    size_t (*read)(struct vfs_mount *mnt, int fd, void *buf, size_t count);
    size_t (*write)(struct vfs_mount *mnt, int fd, const void *buf, size_t count);
    off_t  (*lseek)(struct vfs_mount *mnt, int fd, off_t offset, int whence);
    int    (*stat)(struct vfs_mount *mnt, const char *path, struct stat *st);
    int    (*lstat)(struct vfs_mount *mnt, const char *path, struct stat *st);
    int    (*unlink)(struct vfs_mount *mnt, const char *path);
    int    (*rename)(struct vfs_mount *mnt, const char *oldpath, const char *newpath);
    int    (*mkdir)(struct vfs_mount *mnt, const char *path);
    int    (*rmdir)(struct vfs_mount *mnt, const char *path);
    int    (*chdir)(struct vfs_mount *mnt, const char *path);
    char  *(*getcwd)(struct vfs_mount *mnt, char *buf, size_t size);
    int    (*chmod)(struct vfs_mount *mnt, const char *path, unsigned char mode);
    int    (*getdents)(struct vfs_mount *mnt, int fd, void *dirp, unsigned int count);
    int    (*statfs)(struct vfs_mount *mnt, const char *path, void *buf);
    long   (*setxattr)(struct vfs_mount *mnt, const char *path, const char *name,
                       const void *value, size_t size, int flags);
    long   (*getxattr)(struct vfs_mount *mnt, const char *path, const char *name,
                       void *value, size_t size);
    int    (*removexattr)(struct vfs_mount *mnt, const char *path, const char *name);
} vfs_ops_t;

typedef struct vfs_fs_type
{
    char name[VFS_NAME_MAX];
    int (*mount)(struct vfs_mount *mnt, const char *source, unsigned long flags);
    int (*umount)(struct vfs_mount *mnt);
    vfs_ops_t ops;
    int used;
} vfs_fs_type_t;

typedef struct vfs_mount
{
    char path[VFS_PATH_MAX];
    char source[VFS_PATH_MAX];
    char fstype[VFS_NAME_MAX];
    vfs_fs_type_t *type;
    void *private_data;
    unsigned long flags;
    int used;
} vfs_mount_t;

int vfs_init(void);
int vfs_is_ready(void);

int vfs_register(vfs_fs_type_t *type);
vfs_fs_type_t *vfs_find_fs(const char *name);

int vfs_mount(const char *fstype, const char *target, const char *source, unsigned long flags);
int vfs_umount(const char *target);

vfs_mount_t *vfs_mount_slot(int index);

void vfs_set_cwd(const char *path);
char *vfs_getcwd(char *buf, size_t size);

int vfs_open(const char *path, int flags);
int vfs_close(int fd);
size_t vfs_read(int fd, void *buf, size_t count);
size_t vfs_write(int fd, const void *buf, size_t count);
off_t vfs_lseek(int fd, off_t offset, int whence);
int vfs_stat(const char *path, struct stat *st);
int vfs_lstat(const char *path, struct stat *st);
int vfs_unlink(const char *path);
int vfs_rename(const char *oldpath, const char *newpath);
int vfs_mkdir(const char *path);
int vfs_rmdir(const char *path);
int vfs_chdir(const char *path);
int vfs_chmod(const char *path, unsigned char mode);
int vfs_getdents(int fd, void *dirp, unsigned int count);
int vfs_statfs(const char *path, void *buf);
long vfs_setxattr(const char *path, const char *name, const void *value, size_t size, int flags);
long vfs_getxattr(const char *path, const char *name, void *value, size_t size);
int vfs_removexattr(const char *path, const char *name);

#endif /* __VFS_H__ */
