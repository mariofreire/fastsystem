// Fast System FAT filesystem type for the VFS
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2026 DSP Interactive.
//
// The "fat" type does not reimplement the filesystem. Every callback
// goes through the existing kernel/fat.c operations.

#include "itypes.h"
#include "errno.h"
#include "string.h"
#include "fat.h"
#include "vfs.h"
#include "fatfs.h"

static int fat_mounts = 0;

static int fat_mount(vfs_mount_t *mnt, const char *source, unsigned long flags)
{
    (void)source;
    (void)flags;

    if (mnt == NULL)
        return -EINVAL;
    if (fat == NULL || !hasactive() || !isfattype())
        return -ENODEV;
    if (fat_mounts != 0)
        return -EBUSY;

    mnt->private_data = fat;
    fat_mounts++;
    return 0;
}

static int fat_umount(vfs_mount_t *mnt)
{
    if (mnt == NULL)
        return -EINVAL;
    mnt->private_data = NULL;
    if (fat_mounts > 0)
        fat_mounts--;
    return 0;
}

static int fat_vfs_open(vfs_mount_t *mnt, const char *path, int flags)
{
    (void)mnt;
    return sys_open_handler(path, flags);
}

static int fat_vfs_close(vfs_mount_t *mnt, int fd)
{
    (void)mnt;
    return sys_close_handler(fd);
}

static size_t fat_vfs_read(vfs_mount_t *mnt, int fd, void *buf, size_t count)
{
    (void)mnt;
    return sys_read_handler(fd, buf, count);
}

static size_t fat_vfs_write(vfs_mount_t *mnt, int fd, const void *buf, size_t count)
{
    (void)mnt;
    return sys_write_handler(fd, buf, count);
}

static off_t fat_vfs_lseek(vfs_mount_t *mnt, int fd, off_t offset, int whence)
{
    (void)mnt;
    return sys_lseek_handler(fd, offset, whence);
}

static int fat_vfs_stat(vfs_mount_t *mnt, const char *path, struct stat *st)
{
    (void)mnt;
    return sys_stat_handler(path, st);
}

static int fat_vfs_lstat(vfs_mount_t *mnt, const char *path, struct stat *st)
{
    (void)mnt;
    return sys_lstat_handler(path, st);
}

static int fat_vfs_unlink(vfs_mount_t *mnt, const char *path)
{
    (void)mnt;
    return sys_unlink_handler(path);
}

static int fat_vfs_rename(vfs_mount_t *mnt, const char *oldpath, const char *newpath)
{
    (void)mnt;
    return sys_rename_handler(oldpath, newpath);
}

static int fat_vfs_mkdir(vfs_mount_t *mnt, const char *path)
{
    (void)mnt;
    return sys_mkdir_handler(path);
}

static int fat_vfs_rmdir(vfs_mount_t *mnt, const char *path)
{
    (void)mnt;
    return sys_rmdir_handler(path);
}

static int fat_vfs_chdir(vfs_mount_t *mnt, const char *path)
{
    (void)mnt;
    return sys_chdir_handler(path);
}

static char *fat_vfs_getcwd(vfs_mount_t *mnt, char *buf, size_t size)
{
    (void)mnt;
    return sys_getcwd_handler(buf, size);
}

static int fat_vfs_chmod(vfs_mount_t *mnt, const char *path, unsigned char mode)
{
    (void)mnt;
    return sys_chmod_handler(path, mode);
}

static int fat_vfs_getdents(vfs_mount_t *mnt, int fd, void *dirp, unsigned int count)
{
    (void)mnt;
    return sys_getdents_handler(fd, (struct sys_dirent *)dirp, count);
}

static int fat_vfs_statfs(vfs_mount_t *mnt, const char *path, void *buf)
{
    (void)mnt;
    return sys_statfs_handler(path, (fat_statfs_t *)buf);
}

static long fat_vfs_setxattr(vfs_mount_t *mnt, const char *path, const char *name,
                             const void *value, size_t size, int flags)
{
    (void)mnt;
    return sys_setxattr_handler(path, name, value, size, flags);
}

static long fat_vfs_getxattr(vfs_mount_t *mnt, const char *path, const char *name,
                             void *value, size_t size)
{
    (void)mnt;
    return sys_getxattr_handler(path, name, value, size);
}

static int fat_vfs_removexattr(vfs_mount_t *mnt, const char *path, const char *name)
{
    (void)mnt;
    return sys_removexattr_handler(path, name);
}

int fatfs_register(void)
{
    vfs_fs_type_t fs;

    memset(&fs, 0, sizeof(fs));
    strcpy(fs.name, "fat");
    fs.mount = fat_mount;
    fs.umount = fat_umount;
    fs.ops.open = fat_vfs_open;
    fs.ops.close = fat_vfs_close;
    fs.ops.read = fat_vfs_read;
    fs.ops.write = fat_vfs_write;
    fs.ops.lseek = fat_vfs_lseek;
    fs.ops.stat = fat_vfs_stat;
    fs.ops.lstat = fat_vfs_lstat;
    fs.ops.unlink = fat_vfs_unlink;
    fs.ops.rename = fat_vfs_rename;
    fs.ops.mkdir = fat_vfs_mkdir;
    fs.ops.rmdir = fat_vfs_rmdir;
    fs.ops.chdir = fat_vfs_chdir;
    fs.ops.getcwd = fat_vfs_getcwd;
    fs.ops.chmod = fat_vfs_chmod;
    fs.ops.getdents = fat_vfs_getdents;
    fs.ops.statfs = fat_vfs_statfs;
    fs.ops.setxattr = fat_vfs_setxattr;
    fs.ops.getxattr = fat_vfs_getxattr;
    fs.ops.removexattr = fat_vfs_removexattr;
    return vfs_register(&fs);
}
