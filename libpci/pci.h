// Fast System PCI
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2024 DSP Interactive.

#ifndef __PCI_H__
#define __PCI_H__

#define SYSTEM_PCI_ADDRESS 0x840300

#define PCI_BAR_ADDRESS_SIZE_16(n,b)       ((uint16_t)(pci_device[n].pci.bar[b] & 0xFFF0))
#define PCI_BAR_ADDRESS_SIZE_32(n,b)       ((uint32_t)(pci_device[n].pci.bar[b] & 0xFFFFFFF0))
#define PCI_BAR_ADDRESS_SIZE_64(n,b)       ((uint64_t)(((uint32_t)(pci_device[n].pci.bar[b] & 0xFFFFFFF0)) + (((uint32_t)(pci_device[n].pci.bar[b + 1] & 0xFFFFFFFF)) << 32)))

#pragma pack (push, 1)

typedef struct
{
	uint16_t vendor;
	uint16_t device;
	uint16_t command;
	uint16_t status;
	uint8_t revision;
	uint8_t progif;
	uint8_t subclass;
	uint8_t class;
	uint8_t cache;
	uint8_t lat_timer;
	uint8_t header_type;
	uint8_t bist;
	uint32_t bar[6];
	uint32_t cardbus;
	uint16_t subsystem_vendor;
	uint16_t subsystem_id;
	uint32_t rom_base_addr;
	uint8_t cap_ptr;
	uint8_t reserved0[3];
	uint32_t reserved1;
	uint8_t interrupt_line;
	uint8_t interrupt_pin;
	uint8_t min_gnt;
	uint8_t max_lat;
} pci_t;

typedef struct
{
	uint8_t bus;
	uint8_t slot;
	uint8_t function;
	pci_t pci;
} pci_device_t;

typedef struct
{
	char signature[4];
	uint8_t version;
	uint8_t count;
	pci_device_t device[32];
} system_pci_t;

#pragma pack (pop)

extern uint8_t *system_pci;
extern system_pci_t *sys_pci;
extern pci_device_t *pci_device;
extern int pci_count;

unsigned long pci_config_address(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset);
unsigned char pci_read_byte(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset);
unsigned short pci_read_word(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset);
unsigned long pci_read_long(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset);
void pci_write_byte(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset, unsigned char value);
void pci_write_word(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset, unsigned short value);
void pci_write_long(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset, unsigned long value);

const char *get_pci_class_name(unsigned char class, unsigned char subclass);
int init_pci(void);

#endif // __PCI_H__
