// Fast System Kernel - Storage Controller
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2024 DSP Interactive.

#ifndef __STORAGE_H__
#define __STORAGE_H__

#pragma pack (push, 1)

typedef struct
{
	unsigned char size;
	unsigned char unused;
	unsigned short sector_count;
	unsigned long buffer_ptr;
	unsigned long lba_start_1;
	unsigned long lba_start_2;
} dap_t;

#pragma pack (pop)

extern dap_t* dap;

void loaddap(void);

unsigned long storage_read(int id, void *buffer, int sector, int count);
unsigned long storage_write(int id, const void *buffer, int sector, int count);

unsigned char read_sector(unsigned long sector, unsigned char *buffer);
unsigned char readsector(unsigned long sector, unsigned char *buffer);

unsigned char write_sector(unsigned long sector, const unsigned char *buffer);
unsigned char writesector(unsigned long sector, const unsigned char *buffer);

#endif // __STORAGE_H__
