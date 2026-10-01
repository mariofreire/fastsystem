// Fast System Kernel Loader - Peripheral Component Interconnect
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2026 DSP Interactive.

#ifndef __PCIDEV_H__
#define __PCIDEV_H__

#pragma pack (push, 1)

typedef struct
{
	unsigned short vendor;
	unsigned short device;
	unsigned short command;
	unsigned short status;
	unsigned char revision;
	unsigned char progif;
	unsigned char subclass;
	unsigned char class;
	unsigned char cache;
	unsigned char lat_timer;
	unsigned char header_type;
	unsigned char bist;
	unsigned long bar[6];
	unsigned long cardbus;
	unsigned short subsystem_vendor;
	unsigned short subsystem_id;
	unsigned long rom_base_addr;
	unsigned char cap_ptr;
	unsigned char reserved0[3];
	unsigned long reserved1;
	unsigned char interrupt_line;
	unsigned char interrupt_pin;
	unsigned char min_gnt;
	unsigned char max_lat;
} pci_t;

typedef struct
{
	unsigned char bus;
	unsigned char slot;
	unsigned char function;
	pci_t pci;
} pci_device_t;

typedef struct 
{
	unsigned char class;
	unsigned char subclass;
	const char *name;
} pci_class_name_t;

#pragma pack (pop)

extern pci_device_t pci_device[32];
extern unsigned char pci_count;
extern int pci_initialized;

const char *get_pci_class_name(unsigned char class, unsigned char subclass);
unsigned char pci_scan_device(unsigned char bus, unsigned char slot, unsigned char func, unsigned char index);
unsigned long pci_config_address(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset);
unsigned char pci_read_byte(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset);
unsigned short pci_read_word(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset);
unsigned long pci_read_long(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset);
void pci_write_byte(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset, unsigned char value);
void pci_write_word(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset, unsigned short value);
void pci_write_long(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset, unsigned long value);
void pci_scan(void);
void loadpci(void);

#endif // __PCIDEV_H__
