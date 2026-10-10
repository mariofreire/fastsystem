// Fast System Virtual File System
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2026 DSP Interactive.

#include "itypes.h"
#include "errno.h"
#include "fcntl.h"
#include "string.h"
#include "vfs.h"

static vfs_fs_type_t vfs_fs_table[VFS_MAX_FS];
static vfs_mount_t vfs_mount_table[VFS_MAX_MOUNTS];
static vfs_mount_t *vfs_fd_owner[VFS_FD_MAX];
static char vfs_cwd[VFS_PATH_MAX];
static int vfs_ready = 0;

static void vfs_copy(char *dst, const char *src, size_t n)
{
    size_t i;

    if (n == 0)
        return;
    if (src == NULL)
        src = "";
    for (i = 0; src[i] != 0 && (i + 1) < n; i++)
        dst[i] = src[i];
    dst[i] = 0;
}

static int vfs_normalize(const char *path, char *out, size_t out_sz)
{
    char joined[VFS_PATH_MAX];
    char built[VFS_PATH_MAX];
    unsigned short start[32];
    int count;
    int i;
    size_t len;
    const char *src;
    size_t pos;

    if (out == NULL || out_sz < 2)
        return -EINVAL;

    if (path == NULL || path[0] == 0)
    {
        vfs_copy(joined, vfs_cwd, sizeof(joined));
    }
    else if (path[0] != '/')
    {
        vfs_copy(joined, vfs_cwd, sizeof(joined));
        len = strlen(joined);
        if (len == 0 || joined[len - 1] != '/')
        {
            if (len + 1 >= sizeof(joined))
                return -ENAMETOOLONG;
            joined[len] = '/';
            joined[len + 1] = 0;
            len++;
        }
        if (len + strlen(path) >= sizeof(joined))
            return -ENAMETOOLONG;
        strcpy(joined + len, path);
    }
    else
    {
        vfs_copy(joined, path, sizeof(joined));
    }

    count = 0;
    src = joined;
    while (*src == '/')
        src++;

    while (*src != 0)
    {
        const char *slash;
        size_t n;
        char comp[VFS_PATH_MAX];

        slash = src;
        while (*slash != 0 && *slash != '/')
            slash++;
        n = (size_t)(slash - src);
        if (n == 0)
        {
            src = slash;
            while (*src == '/')
                src++;
            continue;
        }
        if (n >= sizeof(comp))
            return -ENAMETOOLONG;
        memcpy(comp, src, n);
        comp[n] = 0;

        if (strcmp(comp, ".") == 0)
        {
        }
        else if (strcmp(comp, "..") == 0)
        {
            if (count > 0)
                count--;
        }
        else
        {
            if (count >= 32)
                return -ENAMETOOLONG;
            start[count] = (unsigned short)(src - joined);
            count++;
        }

        src = slash;
        while (*src == '/')
            src++;
    }

    if (count == 0)
    {
        vfs_copy(out, "/", out_sz);
        return 0;
    }

    pos = 0;
    built[0] = 0;
    for (i = 0; i < count; i++)
    {
        const char *comp;
        size_t n;
        size_t off;

        off = start[i];
        comp = joined + off;
        n = 0;
        while (comp[n] != 0 && comp[n] != '/')
            n++;
        if (pos + 1 + n >= sizeof(built))
            return -ENAMETOOLONG;
        built[pos++] = '/';
        memcpy(built + pos, comp, n);
        pos += n;
        built[pos] = 0;
    }

    vfs_copy(out, built, out_sz);
    return 0;
}

static int vfs_prefix(const char *mount_path, const char *path)
{
    size_t n;

    if (mount_path == NULL || path == NULL)
        return 0;
    if (mount_path[0] == '/' && mount_path[1] == 0)
        return 1;

    n = strlen(mount_path);
    if (strncmp(path, mount_path, n) != 0)
        return 0;
    if (path[n] == 0 || path[n] == '/')
        return 1;
    return 0;
}

static vfs_mount_t *vfs_match(const char *abs)
{
    vfs_mount_t *best;
    size_t best_len;
    int i;

    best = NULL;
    best_len = 0;
    for (i = 0; i < VFS_MAX_MOUNTS; i++)
    {
        size_t n;

        if (!vfs_mount_table[i].used)
            continue;
        if (!vfs_prefix(vfs_mount_table[i].path, abs))
            continue;
        n = strlen(vfs_mount_table[i].path);
        if (best == NULL || n > best_len)
        {
            best = &vfs_mount_table[i];
            best_len = n;
        }
    }
    return best;
}

static int vfs_local_path(const vfs_mount_t *mnt, const char *abs, char *local, size_t local_sz)
{
    size_t n;

    if (mnt == NULL || abs == NULL || local == NULL)
        return -EINVAL;

    if (mnt->path[0] == '/' && mnt->path[1] == 0)
    {
        vfs_copy(local, abs, local_sz);
        return 0;
    }

    n = strlen(mnt->path);
    if (abs[n] == 0)
        vfs_copy(local, "/", local_sz);
    else
        vfs_copy(local, abs + n, local_sz);
    return 0;
}

static int vfs_resolve(const char *path, vfs_mount_t **out_mnt, char *local, size_t local_sz)
{
    char abs[VFS_PATH_MAX];
    vfs_mount_t *mnt;
    int rc;

    rc = vfs_normalize(path, abs, sizeof(abs));
    if (rc != 0)
        return rc;

    mnt = vfs_match(abs);
    if (mnt == NULL || mnt->type == NULL)
        return -ENOENT;

    rc = vfs_local_path(mnt, abs, local, local_sz);
    if (rc != 0)
        return rc;

    if (out_mnt != NULL)
        *out_mnt = mnt;
    return 0;
}

static int vfs_readonly(const vfs_mount_t *mnt)
{
    if (mnt == NULL)
        return 0;
    return (mnt->flags & VFS_MNT_RDONLY) != 0;
}

static void vfs_bind_fd(int fd, vfs_mount_t *mnt)
{
    if (fd < 0 || fd >= VFS_FD_MAX)
        return;
    vfs_fd_owner[fd] = mnt;
}

static vfs_mount_t *vfs_fd_mount(int fd)
{
    if (fd < 0 || fd >= VFS_FD_MAX)
        return NULL;
    return vfs_fd_owner[fd];
}

int vfs_init(void)
{
    memset(vfs_fs_table, 0, sizeof(vfs_fs_table));
    memset(vfs_mount_table, 0, sizeof(vfs_mount_table));
    memset(vfs_fd_owner, 0, sizeof(vfs_fd_owner));
    vfs_copy(vfs_cwd, "/", sizeof(vfs_cwd));
    vfs_ready = 1;
    return 0;
}

int vfs_is_ready(void)
{
    return vfs_ready;
}

vfs_fs_type_t *vfs_find_fs(const char *name)
{
    int i;

    if (name == NULL || name[0] == 0)
        return NULL;
    for (i = 0; i < VFS_MAX_FS; i++)
    {
        if (!vfs_fs_table[i].used)
            continue;
        if (strcmp(vfs_fs_table[i].name, name) == 0)
            return &vfs_fs_table[i];
    }
    return NULL;
}

int vfs_register(vfs_fs_type_t *type)
{
    int i;

    if (!vfs_ready)
        return -ENODEV;
    if (type == NULL || type->name[0] == 0)
        return -EINVAL;
    if (type->mount == NULL || type->umount == NULL)
        return -EINVAL;
    if (vfs_find_fs(type->name) != NULL)
        return -EEXIST;

    for (i = 0; i < VFS_MAX_FS; i++)
    {
        if (vfs_fs_table[i].used)
            continue;
        vfs_fs_table[i] = *type;
        vfs_fs_table[i].used = 1;
        return 0;
    }
    return -ENOMEM;
}

vfs_mount_t *vfs_mount_slot(int index)
{
    if (index < 0 || index >= VFS_MAX_MOUNTS)
        return NULL;
    return &vfs_mount_table[index];
}

static vfs_mount_t *vfs_find_mount_path(const char *path)
{
    int i;

    for (i = 0; i < VFS_MAX_MOUNTS; i++)
    {
        if (!vfs_mount_table[i].used)
            continue;
        if (strcmp(vfs_mount_table[i].path, path) == 0)
            return &vfs_mount_table[i];
    }
    return NULL;
}

int vfs_mount(const char *fstype, const char *target, const char *source, unsigned long flags)
{
    char norm[VFS_PATH_MAX];
    vfs_fs_type_t *type;
    vfs_mount_t *slot;
    int i;
    int rc;

    if (!vfs_ready)
        return -ENODEV;
    if (fstype == NULL || target == NULL)
        return -EINVAL;

    rc = vfs_normalize(target, norm, sizeof(norm));
    if (rc != 0)
        return rc;

    if (vfs_find_mount_path(norm) != NULL)
        return -EBUSY;

    type = vfs_find_fs(fstype);
    if (type == NULL)
        return -ENODEV;

    slot = NULL;
    for (i = 0; i < VFS_MAX_MOUNTS; i++)
    {
        if (!vfs_mount_table[i].used)
        {
            slot = &vfs_mount_table[i];
            break;
        }
    }
    if (slot == NULL)
        return -ENOMEM;

    memset(slot, 0, sizeof(*slot));
    vfs_copy(slot->path, norm, sizeof(slot->path));
    vfs_copy(slot->source, source != NULL ? source : "", sizeof(slot->source));
    vfs_copy(slot->fstype, type->name, sizeof(slot->fstype));
    slot->type = type;
    slot->flags = flags;

    rc = type->mount(slot, slot->source, flags);
    if (rc != 0)
    {
        memset(slot, 0, sizeof(*slot));
        return rc;
    }

    slot->used = 1;
    return 0;
}

int vfs_umount(const char *target)
{
    char norm[VFS_PATH_MAX];
    vfs_mount_t *slot;
    int rc;
    int i;

    if (!vfs_ready)
        return -ENODEV;
    if (target == NULL)
        return -EINVAL;

    rc = vfs_normalize(target, norm, sizeof(norm));
    if (rc != 0)
        return rc;

    slot = vfs_find_mount_path(norm);
    if (slot == NULL || slot->type == NULL || slot->type->umount == NULL)
        return -EINVAL;

    for (i = 0; i < VFS_FD_MAX; i++)
    {
        if (vfs_fd_owner[i] == slot)
            return -EBUSY;
    }

    rc = slot->type->umount(slot);
    if (rc != 0)
        return rc;

    memset(slot, 0, sizeof(*slot));
    return 0;
}

void vfs_set_cwd(const char *path)
{
    char norm[VFS_PATH_MAX];

    if (!vfs_ready || path == NULL)
        return;
    if (vfs_normalize(path, norm, sizeof(norm)) != 0)
        return;
    vfs_copy(vfs_cwd, norm, sizeof(vfs_cwd));
}

char *vfs_getcwd(char *buf, size_t size)
{
    vfs_mount_t *mnt;
    char local[VFS_PATH_MAX];

    if (buf == NULL || size == 0)
        return NULL;
    if (!vfs_ready)
        return NULL;

    mnt = vfs_match(vfs_cwd);
    if (mnt != NULL && mnt->type != NULL && mnt->type->ops.getcwd != NULL)
    {
        if (vfs_local_path(mnt, vfs_cwd, local, sizeof(local)) == 0)
        {
            char *got = mnt->type->ops.getcwd(mnt, buf, size);
            if (got != NULL)
                return got;
        }
    }

    if (strlen(vfs_cwd) + 1 > size)
        return NULL;
    vfs_copy(buf, vfs_cwd, size);
    return buf;
}

int vfs_open(const char *path, int flags)
{
    vfs_mount_t *mnt;
    char local[VFS_PATH_MAX];
    int fd;
    int rc;

    if (!vfs_ready)
        return -ENODEV;

    rc = vfs_resolve(path, &mnt, local, sizeof(local));
    if (rc != 0)
        return rc;
    if (mnt->type->ops.open == NULL)
        return -ENOSYS;
    if (vfs_readonly(mnt))
    {
        int acc = flags & O_ACCMODE;
        if (acc == O_WRONLY || acc == O_RDWR || (flags & (O_CREAT | O_TRUNC | O_APPEND)))
            return -EROFS;
    }

    fd = mnt->type->ops.open(mnt, local, flags);
    if (fd >= 0)
        vfs_bind_fd(fd, mnt);
    return fd;
}

int vfs_close(int fd)
{
    vfs_mount_t *mnt;
    int rc;

    mnt = vfs_fd_mount(fd);
    if (mnt == NULL || mnt->type == NULL || mnt->type->ops.close == NULL)
        return -EBADF;
    rc = mnt->type->ops.close(mnt, fd);
    vfs_bind_fd(fd, NULL);
    return rc;
}

size_t vfs_read(int fd, void *buf, size_t count)
{
    vfs_mount_t *mnt;

    mnt = vfs_fd_mount(fd);
    if (mnt == NULL || mnt->type == NULL || mnt->type->ops.read == NULL)
        return (size_t)-EBADF;
    return mnt->type->ops.read(mnt, fd, buf, count);
}

size_t vfs_write(int fd, const void *buf, size_t count)
{
    vfs_mount_t *mnt;

    mnt = vfs_fd_mount(fd);
    if (mnt == NULL || mnt->type == NULL || mnt->type->ops.write == NULL)
        return (size_t)-EBADF;
    if (vfs_readonly(mnt))
        return (size_t)-EROFS;
    return mnt->type->ops.write(mnt, fd, buf, count);
}

off_t vfs_lseek(int fd, off_t offset, int whence)
{
    vfs_mount_t *mnt;

    mnt = vfs_fd_mount(fd);
    if (mnt == NULL || mnt->type == NULL || mnt->type->ops.lseek == NULL)
        return -EBADF;
    return mnt->type->ops.lseek(mnt, fd, offset, whence);
}

int vfs_stat(const char *path, struct stat *st)
{
    vfs_mount_t *mnt;
    char local[VFS_PATH_MAX];
    int rc;

    if (st == NULL)
        return -EFAULT;
    rc = vfs_resolve(path, &mnt, local, sizeof(local));
    if (rc != 0)
        return rc;
    if (mnt->type->ops.stat == NULL)
        return -ENOSYS;
    return mnt->type->ops.stat(mnt, local, st);
}

int vfs_lstat(const char *path, struct stat *st)
{
    vfs_mount_t *mnt;
    char local[VFS_PATH_MAX];
    int rc;

    if (st == NULL)
        return -EFAULT;
    rc = vfs_resolve(path, &mnt, local, sizeof(local));
    if (rc != 0)
        return rc;
    if (mnt->type->ops.lstat == NULL)
        return -ENOSYS;
    return mnt->type->ops.lstat(mnt, local, st);
}

int vfs_unlink(const char *path)
{
    vfs_mount_t *mnt;
    char local[VFS_PATH_MAX];
    int rc;

    rc = vfs_resolve(path, &mnt, local, sizeof(local));
    if (rc != 0)
        return rc;
    if (vfs_readonly(mnt))
        return -EROFS;
    if (mnt->type->ops.unlink == NULL)
        return -ENOSYS;
    return mnt->type->ops.unlink(mnt, local);
}

int vfs_rename(const char *oldpath, const char *newpath)
{
    vfs_mount_t *old_mnt;
    vfs_mount_t *new_mnt;
    char old_local[VFS_PATH_MAX];
    char new_local[VFS_PATH_MAX];
    int rc;

    rc = vfs_resolve(oldpath, &old_mnt, old_local, sizeof(old_local));
    if (rc != 0)
        return rc;
    rc = vfs_resolve(newpath, &new_mnt, new_local, sizeof(new_local));
    if (rc != 0)
        return rc;
    if (old_mnt != new_mnt)
        return -EXDEV;
    if (vfs_readonly(old_mnt))
        return -EROFS;
    if (old_mnt->type->ops.rename == NULL)
        return -ENOSYS;
    return old_mnt->type->ops.rename(old_mnt, old_local, new_local);
}

int vfs_mkdir(const char *path)
{
    vfs_mount_t *mnt;
    char local[VFS_PATH_MAX];
    int rc;

    rc = vfs_resolve(path, &mnt, local, sizeof(local));
    if (rc != 0)
        return rc;
    if (vfs_readonly(mnt))
        return -EROFS;
    if (mnt->type->ops.mkdir == NULL)
        return -ENOSYS;
    return mnt->type->ops.mkdir(mnt, local);
}

int vfs_rmdir(const char *path)
{
    vfs_mount_t *mnt;
    char local[VFS_PATH_MAX];
    int rc;

    rc = vfs_resolve(path, &mnt, local, sizeof(local));
    if (rc != 0)
        return rc;
    if (vfs_readonly(mnt))
        return -EROFS;
    if (mnt->type->ops.rmdir == NULL)
        return -ENOSYS;
    return mnt->type->ops.rmdir(mnt, local);
}

int vfs_chdir(const char *path)
{
    vfs_mount_t *mnt;
    char local[VFS_PATH_MAX];
    char abs[VFS_PATH_MAX];
    int rc;

    rc = vfs_normalize(path, abs, sizeof(abs));
    if (rc != 0)
        return rc;
    rc = vfs_resolve(abs, &mnt, local, sizeof(local));
    if (rc != 0)
        return rc;
    if (mnt->type->ops.chdir == NULL)
        return -ENOSYS;
    rc = mnt->type->ops.chdir(mnt, local);
    if (rc != 0)
        return rc;
    vfs_copy(vfs_cwd, abs, sizeof(vfs_cwd));
    return 0;
}

int vfs_chmod(const char *path, unsigned char mode)
{
    vfs_mount_t *mnt;
    char local[VFS_PATH_MAX];
    int rc;

    rc = vfs_resolve(path, &mnt, local, sizeof(local));
    if (rc != 0)
        return rc;
    if (vfs_readonly(mnt))
        return -EROFS;
    if (mnt->type->ops.chmod == NULL)
        return -ENOSYS;
    return mnt->type->ops.chmod(mnt, local, mode);
}

int vfs_getdents(int fd, void *dirp, unsigned int count)
{
    vfs_mount_t *mnt;

    mnt = vfs_fd_mount(fd);
    if (mnt == NULL || mnt->type == NULL || mnt->type->ops.getdents == NULL)
        return -EBADF;
    return mnt->type->ops.getdents(mnt, fd, dirp, count);
}

int vfs_statfs(const char *path, void *buf)
{
    vfs_mount_t *mnt;
    char local[VFS_PATH_MAX];
    int rc;

    if (buf == NULL)
        return -EFAULT;
    rc = vfs_resolve(path, &mnt, local, sizeof(local));
    if (rc != 0)
        return rc;
    if (mnt->type->ops.statfs == NULL)
        return -ENOSYS;
    return mnt->type->ops.statfs(mnt, local, buf);
}

long vfs_setxattr(const char *path, const char *name, const void *value, size_t size, int flags)
{
    vfs_mount_t *mnt;
    char local[VFS_PATH_MAX];
    int rc;

    rc = vfs_resolve(path, &mnt, local, sizeof(local));
    if (rc != 0)
        return rc;
    if (vfs_readonly(mnt))
        return -EROFS;
    if (mnt->type->ops.setxattr == NULL)
        return -ENOSYS;
    return mnt->type->ops.setxattr(mnt, local, name, value, size, flags);
}

long vfs_getxattr(const char *path, const char *name, void *value, size_t size)
{
    vfs_mount_t *mnt;
    char local[VFS_PATH_MAX];
    int rc;

    rc = vfs_resolve(path, &mnt, local, sizeof(local));
    if (rc != 0)
        return rc;
    if (mnt->type->ops.getxattr == NULL)
        return -ENOSYS;
    return mnt->type->ops.getxattr(mnt, local, name, value, size);
}

int vfs_removexattr(const char *path, const char *name)
{
    vfs_mount_t *mnt;
    char local[VFS_PATH_MAX];
    int rc;

    rc = vfs_resolve(path, &mnt, local, sizeof(local));
    if (rc != 0)
        return rc;
    if (vfs_readonly(mnt))
        return -EROFS;
    if (mnt->type->ops.removexattr == NULL)
        return -ENOSYS;
    return mnt->type->ops.removexattr(mnt, local, name);
}
