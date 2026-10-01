// Fast System System Call
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2026 DSP Interactive.

#include "syscall.h"
#include "taskthread.h"

syscall_regs_t syscall0(unsigned long number)
{
	syscall_regs_t r;

	r.eax = number;
	r.ebx = 0;
	r.ecx = 0;
	r.edx = 0;
	r.esi = 0;
	r.edi = 0;

	__asm__ volatile (
		"int $0x80"
		: "+a"(r.eax),
		  "+b"(r.ebx),
		  "+c"(r.ecx),
		  "+d"(r.edx),
		  "+S"(r.esi),
		  "+D"(r.edi)
		:
		: "memory"
	);

	return r;
}

syscall_regs_t syscall1(unsigned long number, unsigned long arg1)
{
	syscall_regs_t r;

	r.eax = number;
	r.ebx = arg1;
	r.ecx = 0;
	r.edx = 0;
	r.esi = 0;
	r.edi = 0;

	__asm__ volatile (
		"int $0x80"
		: "+a"(r.eax),
		  "+b"(r.ebx),
		  "+c"(r.ecx),
		  "+d"(r.edx),
		  "+S"(r.esi),
		  "+D"(r.edi)
		:
		: "memory"
	);

	return r;
}

syscall_regs_t syscall2(unsigned long number, unsigned long arg1, unsigned long arg2)
{
	syscall_regs_t r;

	r.eax = number;
	r.ebx = arg1;
	r.ecx = arg2;
	r.edx = 0;
	r.esi = 0;
	r.edi = 0;

	__asm__ volatile (
		"int $0x80"
		: "+a"(r.eax),
		  "+b"(r.ebx),
		  "+c"(r.ecx),
		  "+d"(r.edx),
		  "+S"(r.esi),
		  "+D"(r.edi)
		:
		: "memory"
	);

	return r;
}

syscall_regs_t syscall3(unsigned long number, unsigned long arg1, unsigned long arg2, unsigned long arg3)
{
	syscall_regs_t r;

	r.eax = number;
	r.ebx = arg1;
	r.ecx = arg2;
	r.edx = arg3;
	r.esi = 0;
	r.edi = 0;

	__asm__ volatile (
		"int $0x80"
		: "+a"(r.eax),
		  "+b"(r.ebx),
		  "+c"(r.ecx),
		  "+d"(r.edx),
		  "+S"(r.esi),
		  "+D"(r.edi)
		:
		: "memory"
	);

	return r;
}

syscall_regs_t syscall4(unsigned long number, unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4)
{
	syscall_regs_t r;

	r.eax = number;
	r.ebx = arg1;
	r.ecx = arg2;
	r.edx = arg3;
	r.esi = arg4;
	r.edi = 0;

	__asm__ volatile (
		"int $0x80"
		: "+a"(r.eax),
		  "+b"(r.ebx),
		  "+c"(r.ecx),
		  "+d"(r.edx),
		  "+S"(r.esi),
		  "+D"(r.edi)
		:
		: "memory"
	);

	return r;
}

syscall_regs_t syscall5(unsigned long number, unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5)
{
	syscall_regs_t r;

	r.eax = number;
	r.ebx = arg1;
	r.ecx = arg2;
	r.edx = arg3;
	r.esi = arg4;
	r.edi = arg5;

	__asm__ volatile (
		"int $0x80"
		: "+a"(r.eax),
		  "+b"(r.ebx),
		  "+c"(r.ecx),
		  "+d"(r.edx),
		  "+S"(r.esi),
		  "+D"(r.edi)
		:
		: "memory"
	);

	return r;
}

/*
static syscall_regs_t syscall_pci4(
	unsigned long number,
	unsigned long bus,
	unsigned long slot,
	unsigned long func,
	unsigned long offset)
{
	syscall_regs_t r;

	r.eax = number;
	r.ebx = bus;
	r.ecx = slot;
	r.edx = func;
	r.esi = offset;
	r.edi = 0;

	__asm__ volatile (
		"int $0x80"
		: "+a"(r.eax),
		  "+b"(r.ebx),
		  "+c"(r.ecx),
		  "+d"(r.edx),
		  "+S"(r.esi),
		  "+D"(r.edi)
		:
		: "memory"
	);

	return r;
}

static syscall_regs_t syscall_pci5(
	unsigned long number,
	unsigned long bus,
	unsigned long slot,
	unsigned long func,
	unsigned long offset,
	unsigned long data)
{
	syscall_regs_t r;

	r.eax = number;
	r.ebx = bus;
	r.ecx = slot;
	r.edx = func;
	r.esi = offset;
	r.edi = data;

	__asm__ volatile (
		"int $0x80"
		: "+a"(r.eax),
		  "+b"(r.ebx),
		  "+c"(r.ecx),
		  "+d"(r.edx),
		  "+S"(r.esi),
		  "+D"(r.edi)
		:
		: "memory"
	);

	return r;
}
*/

int fork(void)
{
	syscall_regs_t r;
	r = syscall0(2);
	return (int)r.eax;
}

ssize_t read(int fd, void *buf, size_t count)
{
	syscall_regs_t r;
	r = syscall3(3, (unsigned long)fd, (unsigned long)buf, (unsigned long)count);
	return (ssize_t)r.eax;
}

ssize_t write(int fd, const void *buf, size_t count)
{
	syscall_regs_t r;
	r = syscall3(4, (unsigned long)fd, (unsigned long)buf, (unsigned long)count);
	return (ssize_t)r.eax;
}

int open(const char *pathname, int flags)
{
	syscall_regs_t r;
	r = syscall2(5, (unsigned long)pathname, (unsigned long)flags);
	return (int)r.eax;
}

int close(int fd)
{
	syscall_regs_t r;
	r = syscall1(6, (unsigned long)fd);
	return (int)r.eax;
}

int unlink(const char *pathname)
{
	syscall_regs_t r;
	r = syscall1(10, (unsigned long)pathname);
	return (int)r.eax;
}

int execve(const char *pathname, char **argv)
{
	syscall_regs_t r;
	r = syscall2(11, (unsigned long)pathname, (unsigned long)argv);
	return (int)r.eax;
}

int chdir(const char *path)
{
	syscall_regs_t r;
	r = syscall1(12, (unsigned long)path);
	return (int)r.eax;
}

int chmod(const char *path, unsigned char attributes)
{
	syscall_regs_t r;
	r = syscall2(15, (unsigned long)path, (unsigned long)attributes);
	return (int)r.eax;
}

off_t lseek(int fd, off_t offset, int whence)
{
	syscall_regs_t r;
	r = syscall3(19, (unsigned long)fd, (unsigned long)offset, (unsigned long)whence);
	return (off_t)r.eax;
}

void presskey(void)
{
	syscall0(29);
}

int rename(const char *oldpath, const char *newpath)
{
	syscall_regs_t r;
	r = syscall2(38, (unsigned long)oldpath, (unsigned long)newpath);
	return (int)r.eax;
}

int mkdir(const char *path)
{
	syscall_regs_t r;
	r = syscall1(39, (unsigned long)path);
	return (int)r.eax;
}

int rmdir(const char *path)
{
	syscall_regs_t r;
	r = syscall1(40, (unsigned long)path);
	return (int)r.eax;
}

int socket_shutdown(void)
{
	syscall_regs_t r;
	r = syscall2(102, 13, 0);
	return (int)r.eax;
}

void restart(void)
{
	syscall0(88);
}

int statfs(const char *path, fat_statfs_t *buf)
{
	syscall_regs_t r;
	r = syscall2(99, (unsigned long)path, (unsigned long)buf);
	return (int)r.eax;
}

int stat(const char *path, struct stat *buf)
{
	syscall_regs_t r;
	r = syscall2(106, (unsigned long)path, (unsigned long)buf);
	return (int)r.eax;
}

int lstat(const char *path, struct stat *buf)
{
	syscall_regs_t r;
	r = syscall2(107, (unsigned long)path, (unsigned long)buf);
	return (int)r.eax;
}

int getdents(int fd, struct sys_dirent *dirp, unsigned int count)
{
	syscall_regs_t r;
	r = syscall3(141, (unsigned long)fd, (unsigned long)dirp, (unsigned long)count);
	return (int)r.eax;
}

char *getcwd(char *buf, size_t size)
{
	syscall_regs_t r;
	r = syscall2(183, (unsigned long)buf, (unsigned long)size);
	return (char *)r.eax;
}

int set_video_vesa_mode(unsigned long mode)
{
	syscall_regs_t r;

	r = syscall1(
		225,
		mode);

	return (int)r.eax;
}

int setxattr(const char *path, const char *name, const void *value, size_t size, int flags)
{
	syscall_regs_t r;
	r = syscall5(226, (unsigned long)path, (unsigned long)name, (unsigned long)value, (unsigned long)size, (unsigned long)flags);
	return (int)r.eax;
}

int getxattr(const char *path, const char *name, void *value, size_t size)
{
	syscall_regs_t r;
	r = syscall4(229, (unsigned long)path, (unsigned long)name, (unsigned long)value, (unsigned long)size);
	return (int)r.eax;
}

int removexattr(const char *path, const char *name)
{
	syscall_regs_t r;
	r = syscall2(235, (unsigned long)path, (unsigned long)name);
	return (int)r.eax;
}

/*
unsigned long pci_config_address(
	unsigned char bus,
	unsigned char slot,
	unsigned char func,
	unsigned char offset)
{
	syscall_regs_t r;

	r = syscall_pci4(
		400,
		(unsigned long)bus,
		(unsigned long)slot,
		(unsigned long)func,
		(unsigned long)offset);

	return r.eax;
}

unsigned char pci_read_byte(
	unsigned char bus,
	unsigned char slot,
	unsigned char func,
	unsigned char offset)
{
	syscall_regs_t r;

	r = syscall_pci4(
		401,
		(unsigned long)bus,
		(unsigned long)slot,
		(unsigned long)func,
		(unsigned long)offset);

	return (unsigned char)r.eax;
}

unsigned short pci_read_word(
	unsigned char bus,
	unsigned char slot,
	unsigned char func,
	unsigned char offset)
{
	syscall_regs_t r;

	r = syscall_pci4(
		402,
		(unsigned long)bus,
		(unsigned long)slot,
		(unsigned long)func,
		(unsigned long)offset);

	return (unsigned short)r.eax;
}

unsigned long pci_read_long(
	unsigned char bus,
	unsigned char slot,
	unsigned char func,
	unsigned char offset)
{
	syscall_regs_t r;

	r = syscall_pci4(
		403,
		(unsigned long)bus,
		(unsigned long)slot,
		(unsigned long)func,
		(unsigned long)offset);

	return r.eax;
}

void pci_write_byte(
	unsigned char bus,
	unsigned char slot,
	unsigned char func,
	unsigned char offset,
	unsigned char data)
{
	syscall_pci5(
		404,
		(unsigned long)bus,
		(unsigned long)slot,
		(unsigned long)func,
		(unsigned long)offset,
		(unsigned long)data);
}

void pci_write_word(
	unsigned char bus,
	unsigned char slot,
	unsigned char func,
	unsigned char offset,
	unsigned short data)
{
	syscall_pci5(
		405,
		(unsigned long)bus,
		(unsigned long)slot,
		(unsigned long)func,
		(unsigned long)offset,
		(unsigned long)data);
}

void pci_write_long(
	unsigned char bus,
	unsigned char slot,
	unsigned char func,
	unsigned char offset,
	unsigned long data)
{
	syscall_pci5(
		406,
		(unsigned long)bus,
		(unsigned long)slot,
		(unsigned long)func,
		(unsigned long)offset,
		data);
}
*/

int read_sector(unsigned long sector, void *buffer)
{
	syscall_regs_t r;
	r = syscall3(413, 0, sector, (unsigned long)buffer);
	return (int)r.eax;
}

int write_sector(unsigned long sector, const void *buffer)
{
	syscall_regs_t r;
	r = syscall3(414, 0, sector, (unsigned long)buffer);
	return (int)r.eax;
}

int get_video_mode(void)
{
	syscall_regs_t r;
	r = syscall0(424);
	return (int)r.eax;
}

int set_isr(unsigned long address, unsigned char interrupt_number)
{
	syscall_regs_t r;
	r = syscall2(486, address, (unsigned long)interrupt_number);
	return (int)r.eax;
}

unsigned char getdiskcount(void)
{
	syscall_regs_t r;
	r = syscall0(506);
	return (unsigned char)r.edx;
}

int mouse_initialized(void)
{
	syscall_regs_t r;
	r = syscall0(508);
	return (int)r.eax;
}

int mouse_enable(void)
{
	syscall_regs_t r;
	r = syscall0(509);
	return (int)r.eax;
}

int mouse_disable(void)
{
	syscall_regs_t r;
	r = syscall0(510);
	return (int)r.eax;
}

int mouse_get_info(sys_mouse_regs_t *result)
{
	syscall_regs_t r;

	if (!result)
		return -1;

	r = syscall0(511);

	result->eax = r.eax;
	result->ebx = r.ebx;
	result->ecx = r.ecx;
	result->edx = r.edx;

	return (int)r.eax;
}

void getmouse(int *x, int *y, int *buttons)
{
	syscall_regs_t r;

	r = syscall0(512);

	if (x)
		*x = (int)r.ecx;

	if (y)
		*y = (int)r.edx;

	if (buttons)
		*buttons = (int)r.ebx;
}

int init_mouse(void)
{
	syscall_regs_t r;
	r = syscall0(513);
	return (int)r.eax;
}

int uninit_mouse(void)
{
	syscall_regs_t r;
	r = syscall0(514);
	return (int)r.eax;
}

int set_mouse_screen_size(int width, int height)
{
	syscall_regs_t r;
	r = syscall3(515, 0, (unsigned long)width, (unsigned long)height);
	return (int)r.eax;
}

int kbhit(int mode)
{
	syscall_regs_t r;
	r = syscall3(516, 0, 0,	(unsigned long)mode);
	return (int)r.eax;
}

void resetkeys(void)
{
	syscall0(517);
}
