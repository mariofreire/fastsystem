// Fast System System Call
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2026 DSP Interactive.

#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef struct
{
	unsigned long eax;
	unsigned long ebx;
	unsigned long ecx;
	unsigned long edx;
	unsigned long esi;
	unsigned long edi;
} syscall_regs_t;

struct stat;
struct sys_dirent;

typedef struct
{
	unsigned long eax;
	unsigned long ebx;
	unsigned long ecx;
	unsigned long edx;
} sys_mouse_regs_t;

void exit(int status);
int fork(void);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
int open(const char *pathname, int flags);
int close(int fd);
int unlink(const char *pathname);
int execve(const char *pathname, char **argv);
int chdir(const char *path);
int chmod(const char *path, unsigned char attributes);
off_t lseek(int fd, off_t offset, int whence);
void presskey(void);
int rename(const char *oldpath, const char *newpath);
int mkdir(const char *path);
int rmdir(const char *path);
void *sbrk(unsigned long increment);
void restart(void);
int socket_shutdown(void);
int getdents(int fd, struct sys_dirent *dirp, unsigned int count);
char *getcwd(char *buf, size_t size);
int set_video_vesa_mode(unsigned long mode);
int setxattr(const char *path, const char *name, const void *value, size_t size, int flags);
int getxattr(const char *path, const char *name, void *value, size_t size);
int removexattr(const char *path, const char *name);


/*
unsigned long pci_config_address(
	unsigned char bus,
	unsigned char slot,
	unsigned char func,
	unsigned char offset);

unsigned char pci_read_byte(
	unsigned char bus,
	unsigned char slot,
	unsigned char func,
	unsigned char offset);

unsigned short pci_read_word(
	unsigned char bus,
	unsigned char slot,
	unsigned char func,
	unsigned char offset);

unsigned long pci_read_long(
	unsigned char bus,
	unsigned char slot,
	unsigned char func,
	unsigned char offset);

void pci_write_byte(
	unsigned char bus,
	unsigned char slot,
	unsigned char func,
	unsigned char offset,
	unsigned char data);

void pci_write_word(
	unsigned char bus,
	unsigned char slot,
	unsigned char func,
	unsigned char offset,
	unsigned short data);

void pci_write_long(
	unsigned char bus,
	unsigned char slot,
	unsigned char func,
	unsigned char offset,
	unsigned long data);
*/

int read_sector(unsigned long sector, void *buffer);
int write_sector(unsigned long sector, const void *buffer);

int get_video_mode(void);
int set_isr(unsigned long address, unsigned char interrupt_number);

unsigned char getdiskcount(void);
int getch(void);

int mouse_initialized(void);
int mouse_enable(void);
int mouse_disable(void);

int mouse_get_info(sys_mouse_regs_t *result);
void getmouse(int *x, int *y, int *buttons);
int init_mouse(void);
int uninit_mouse(void);
int set_mouse_screen_size(int width, int height);
int kbhit(int mode);
void resetkeys(void);


syscall_regs_t syscall0(unsigned long number);
syscall_regs_t syscall1(unsigned long number, unsigned long arg1);
syscall_regs_t syscall2(unsigned long number, unsigned long arg1, unsigned long arg2);
syscall_regs_t syscall3(unsigned long number, unsigned long arg1, unsigned long arg2, unsigned long arg3);
syscall_regs_t syscall4(unsigned long number, unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4);
syscall_regs_t syscall5(unsigned long number, unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5);

#endif // _SYSCALL_H
