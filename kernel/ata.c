// Fast System Kernel - Advanced Technology Attachment
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2026 DSP Interactive.

#include <stdarg.h>
#include "fskrnl.h"
#include "enum.h"

const unsigned short ata_base[4] =
{
	0x1F0,
	0x1F0,
	0x170,
	0x170
};

unsigned int ata_delay = 1;

dap_t* dap;

void ata_reset(int id)
{
	ata_delay = 10;	
	outb(ata_base[id] + 0x206, 4);
	msleep(ata_delay);
	outb(ata_base[id] + 0x206, 0);
	msleep(ata_delay);
}

unsigned char ata_wait(int id, int mask, int state)
{
	int s;
	int t=0;
	while(1)
	{
		s = inb(ata_base[id] + 7);
		if((s & mask) == state)
		{
			return 1;
		}
		if ((s & 0x01) || (t >= 300))
		{
			ata_reset(id);
			return 0;
		}
		ata_delay = 10;
		msleep(ata_delay);
		t++;
	}
}

void ata_pio_read(int id, void *buffer, int size) 
{
	unsigned short *buf = (unsigned short*)buffer;
	while(size > 0) {
		*buf = inw(ata_base[id]);
		buf++;
		size -= 2;
	}
}

unsigned char ata_begin(int id, int command, int sector, int count) 
{
	int base;
	int r;
	int sector_start;
	int cylinder_lo;
	int cylinder_hi;
	int flags;
	base = ata_base[id];	
	flags = 0x80;
	flags |= 0x40;
	flags |= 0x20;	
	if(id % 2) flags |= 0x10; // slave
	sector_start = (sector >> 0) & 0xff;
	cylinder_lo = (sector >> 8) & 0xff;
	cylinder_hi = (sector >> 16) & 0xff;
	flags |= (sector >> 24) & 0x0f;
	if(!ata_wait(id, 0x80, 0)) return 0;
	outb(base + 6, flags);
	if(command == 0xA1) r = ata_wait(id, 0x80, 0);
	else r = ata_wait(id, 0x80|0x40, 0x40);
	if(!r) return 0;
	outb(base + 0x206, 0);
	outb(base + 2, count);
	outb(base + 3, sector_start);
	outb(base + 4, cylinder_lo);
	outb(base + 5, cylinder_hi);
	outb(base + 6, flags);
	outb(base + 7, command);
	return 1;
}

unsigned long ata_read(int id, void *buffer, int sector, int count)
{
	int i;
	if(!ata_begin(id, 0x20, sector, count)) return 0;
	delay_ms(1);
	for(i = 0;i < count; i++) 
	{
		if(!ata_wait(id, 8, 8)) return 0;
		ata_pio_read(id, buffer, SECTORSIZE);
		buffer = ((char*)buffer) + SECTORSIZE;
		sector++;
	}
	if(!ata_wait(id, 0x80, 0)) return 0;
	return count;
}

unsigned long ata_write(int id, const void *buffer, int sector, int count)
{
	int i;
	int j;
	const unsigned short *buf;
	if(count <= 0) return 0;
	if(!ata_begin(id, 0x30, sector, count))	return 0;
	delay_ms(1);
	for(i = 0; i < count; i++)
	{
		if(!ata_wait(id, 8, 8)) return 0;
		buf = (const unsigned short*)buffer;
		for(j = 0; j < SECTORSIZE / 2; j++)
		{
			outw(ata_base[id], buf[j]);
		}
		buffer = ((const char*)buffer) + SECTORSIZE;
		sector++;
	}
	if(!ata_wait(id, 0x80, 0)) return 0;
	return count;
}

unsigned char get_ata_ident(int id, int command, void *buffer)
{
	if (!ata_begin(id, command, 0, 0)) return 0;
	if (!ata_wait(id, 8, 8)) return 0;
	ata_pio_read(id, buffer, SECTORSIZE);
	return 1;
}

unsigned char get_ata_name(int id, char *s)
{
	unsigned char is_ata, is_atapi;
	unsigned long i;
	unsigned short buf[256];
	char *b = (char*)buf;
	unsigned char st = inb(ata_base[id] + 7);
	if(st == 0xff) {
		return 0;
	}
	ata_reset(id);
	memset(b,0,512);
	is_ata = get_ata_ident(id, 0xEC, b);
	is_atapi = get_ata_ident(id, 0xA1, b);
	if((!is_ata) && (!is_atapi)) {
		return 0;
	}
	for(i=0;i<512;i+=2) {
		st = b[i];
		b[i] = b[i + 1];
		b[i + 1] = st;
	}
	b[256]=0;
	strcpy(s, &b[54]);
	s[40] = 0;
	return 1;
}

unsigned char get_ata_type(int id)
{
	unsigned char is_ata, is_atapi;
	unsigned long i;
	unsigned short buf[256];
	char *b = (char*)buf;
	unsigned char st = inb(ata_base[id] + 7);
	if(st == 0xff) {
		return 0;
	}
	ata_reset(id);
	memset(b,0,512);
	is_ata = get_ata_ident(id, 0xEC, b);
	is_atapi = get_ata_ident(id, 0xA1, b);
	if (is_ata)
	{
		return 1;
	}
	if (is_atapi)
	{
		return 2;
	}
	return 0;
}

void detectide(void)
{
	int i;
	char ata_ide_name[40][4];
	char *ata_ide_order[4] = {"Primary IDE Master", "Primary IDE Slave", "Secondary IDE Master", "Secondary IDE Slave"};
	for(i=0;i<4;i++)
	{
		if (get_ata_name(i, ata_ide_name[i]))
		{
			printk("%s: %s\n", ata_ide_order[i], ata_ide_name[i]);
		}
		else
		{
			printk("%s: None\n", ata_ide_order[i]);
		}
	}
}
