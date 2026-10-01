// Fast System Unix Standard
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2024 DSP Interactive.

#ifndef __UNISTD_H__
#define __UNISTD_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

void exit_group(int status);
void exit(int code);
int fork(void);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
int open(const char *pathname, int flags);
int close(int fd);
int unlink(const char *pathname);
int execve(const char *pathname, char **argv);
int chdir(const char *path);
off_t lseek(int fd, off_t offset, int whence);
void presskey(void);
int rename(const char *oldpath, const char *newpath);
void *sbrk(unsigned long increment);
void restart(void);
int socket_shutdown(void);
int getdents(int fd, struct sys_dirent *dirp, unsigned int count);
char *getcwd(char *buf, size_t size);
int set_video_vesa_mode(unsigned long mode);
int setxattr(const char *path, const char *name, const void *value, size_t size, int flags);
int getxattr(const char *path, const char *name, void *value, size_t size);
int removexattr(const char *path, const char *name);

#endif // __UNISTD_H__
