// Fast System Kernel Loader - Peripheral Component Interconnect
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2024 DSP Interactive.

#ifndef __PCI_H__
#define __PCI_H__

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

#pragma pack (pop)

extern pci_device_t pci_device[32] ENHANCED_DATA_SECTION;
extern unsigned char pci_count ENHANCED_DATA_SECTION;

extern unsigned long pci_config_address(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset) ENHANCED_CODE_SECTION;
extern unsigned char pci_read_byte(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset) ENHANCED_CODE_SECTION;
extern unsigned short pci_read_word(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset) ENHANCED_CODE_SECTION;
extern unsigned long pci_read_long(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset) ENHANCED_CODE_SECTION;
extern unsigned char pci_scan_device(unsigned char bus, unsigned char slot, unsigned char func, unsigned char index) ENHANCED_CODE_SECTION;
extern void pci_scan_bus(unsigned char bus) ENHANCED_CODE_SECTION;
extern void pci_scan(void) ENHANCED_CODE_SECTION;
extern void loadpci(void) ENHANCED_CODE_SECTION;
extern int init_pci(void) ENHANCED_CODE_SECTION;

#endif // __PCI_H__
