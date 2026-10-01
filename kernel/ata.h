// Fast System Kernel - Advanced Technology Attachment
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2026 DSP Interactive.

#ifndef __ATA_H__
#define __ATA_H__

extern unsigned int ata_delay;

void ata_reset(int id);
unsigned char ata_wait(int id, int mask, int state);
void ata_pio_read(int id, void *buffer, int size);
unsigned char ata_begin(int id, int command, int sector, int count);
unsigned long ata_read(int id, void *buffer, int sector, int count);
unsigned long ata_write(int id, const void *buffer, int sector, int count);
unsigned char get_ata_ident(int id, int command, void *buffer);
unsigned char get_ata_name(int id, char *s);
unsigned char get_ata_type(int id);
void detectide(void);

#endif // __ATA_H__
