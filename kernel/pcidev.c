// Fast System Kernel Loader - Peripheral Component Interconnect
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2026 DSP Interactive.

#include <stdarg.h>
#include "fskrnl.h"
#include "enum.h"

pci_device_t pci_device[32];
unsigned char pci_count=0;
int pci_initialized = 0;

pci_class_name_t pci_class_name[] = 
{
	{0x00, 0x00, "Non-VGA-Compatible Unclassified Device"},
	{0x00, 0x01, "VGA-Compatible Unclassified Device"},
	{0x01, 0x00, "SCSI Bus Controller"},
	{0x01, 0x01, "IDE Controller"},
	{0x01, 0x02, "Floppy Disk Controller"},
	{0x01, 0x03, "IPI Bus Controller"},
	{0x01, 0x04, "RAID Controller"},
	{0x01, 0x05, "ATA Controller"},
	{0x01, 0x06, "Serial ATA Controller"},
	{0x01, 0x07, "Serial Attached SCSI Controller"},
	{0x01, 0x08, "Non-Volatile Memory Controller"},
	{0x01, 0x80, "Other Mass Storage Controller"},
	{0x02, 0x00, "Ethernet Controller"},
	{0x02, 0x01, "Token Ring Controller"},
	{0x02, 0x02, "FDDI Controller"},
	{0x02, 0x03, "ATM Controller"},
	{0x02, 0x04, "ISDN Controller"},
	{0x02, 0x05, "WorldFip Controller"},
	{0x02, 0x06, "PICMG 2.14 Multi Computing Controller"},
	{0x02, 0x07, "Infiniband Controller"},
	{0x02, 0x08, "Fabric Controller"},
	{0x02, 0x80, "Other Network Controller"},
	{0x03, 0x00, "VGA Compatible Controller"},
	{0x03, 0x01, "XGA Controller"},
	{0x03, 0x02, "3D Controller (Not VGA-Compatible)"},
	{0x03, 0x80, "Other Display Controller"},
	{0x04, 0x00, "Multimedia Video Controller"},
	{0x04, 0x01, "Multimedia Audio Controller"},
	{0x04, 0x02, "Computer Telephony Device"},
	{0x04, 0x03, "Audio Device"},
	{0x04, 0x80, "Other Multimedia Controller"},
	{0x05, 0x00, "RAM Controller"},
	{0x05, 0x01, "Flash Controller"},
	{0x05, 0x80, "Other Memory Controller"},
	{0x06, 0x00, "Host Bridge"},
	{0x06, 0x01, "ISA Bridge"},
	{0x06, 0x02, "EISA Bridge"},
	{0x06, 0x03, "MCA Bridge"},
	{0x06, 0x04, "PCI-to-PCI Brige"},
	{0x06, 0x05, "PCMCIA Bridge"},
	{0x06, 0x06, "NuBus Bridge"},
	{0x06, 0x07, "CardBus Bridge"},
	{0x06, 0x08, "RACEway Bridge"},
	{0x06, 0x09, "PCI-to-PCI Bridge"},
	{0x06, 0x0A, "Infiniband-to-PCI Host Bridge"},
	{0x06, 0x80, "Other Bridge"},
	{0x07, 0x00, "Serial Controller"},
	{0x07, 0x01, "Parallel Controller"},
	{0x07, 0x02, "Multiport Serial Controller"},
	{0x07, 0x03, "Modem"},
	{0x07, 0x04, "IEEE 488.1/2 (GPIB) Controller"},
	{0x07, 0x05, "Smart Card Controller"},
	{0x07, 0x80, "Other Simple Communication Controller"},
	{0x08, 0x00, "PIC"},
	{0x08, 0x01, "DMA Controller"},
	{0x08, 0x02, "Timer"},
	{0x08, 0x03, "RTC Controller"},
	{0x08, 0x04, "PCI Hot-Plug Controller"},
	{0x08, 0x05, "SD Host Controller"},
	{0x08, 0x07, "IOMMU"},
	{0x08, 0x80, "Other Base System Peripheral"},
	{0x09, 0x00, "Keyboard Controller"},
	{0x09, 0x01, "Digitizer Pen"},
	{0x09, 0x02, "Mouse Controller"},
	{0x09, 0x03, "Scanner Controller"},
	{0x09, 0x04, "Gameport Controller"},
	{0x09, 0x80, "Other Input Device Controller"},
	{0x0A, 0x00, "Generic Docking Station"},
	{0x0A, 0x80, "Other Docking Station"},
	{0x0B, 0x00, "386 Processor"},
	{0x0B, 0x01, "486 Processor"},
	{0x0B, 0x02, "Pentium Processor"},
	{0x0B, 0x03, "Pentioum Pro Processor"},
	{0x0B, 0x10, "Alpha Processor"},
	{0x0B, 0x20, "PowerPC Processor"},
	{0x0B, 0x30, "MIPS Processor"},
	{0x0B, 0x40, "Co-Processor"},
	{0x0B, 0x80, "Other Processor"},
	{0x0C, 0x00, "FireWire (IEEE 1394) Controller"},
	{0x0C, 0x01, "ACCESS Bus Controller"},
	{0x0C, 0x02, "SSA"},
	{0x0C, 0x03, "USB Controller"},
	{0x0C, 0x04, "Fibre Channel"},
	{0x0C, 0x05, "SMBus Controller"},
	{0x0C, 0x06, "InfiniBand Controller"},
	{0x0C, 0x07, "IPMI Interface"},
	{0x0C, 0x08, "SERCOS Interface (IEC 61491)"},
	{0x0C, 0x09, "CANbus Controller"},
	{0x0C, 0x80, "Other Serial Bus Controller"},
	{0x0D, 0x00, "iRDA Compatible Controller"},
	{0x0D, 0x00, "Consumer IR Controller"},
	{0x0D, 0x00, "RF Controller"},
	{0x0D, 0x00, "Bluetooth Controller"},
	{0x0D, 0x00, "Broadband Controller"},
	{0x0D, 0x00, "Ethernet Controller (802.1a)"},
	{0x0D, 0x00, "Ethernet Controller (802.1b)"},
	{0x0D, 0x00, "Other Wireless Controller"},
	{0x0E, 0x00, "I20"},
	{0x0F, 0x01, "Satellite TV Controller"},
	{0x0F, 0x02, "Satellite Audio Controller"},
	{0x0F, 0x03, "Satellite Voice Controller"},
	{0x0F, 0x04, "Satellite Data Controller"},
	{0x10, 0x00, "Network and Computing Encryption/Decryption"},
	{0x10, 0x10, "Entertainment Encryption/Decryption"},
	{0x10, 0x80, "Other Encryption Controller"},
	{0x11, 0x00, "DPIO Modules"},
	{0x11, 0x01, "Performance Counters"},
	{0x11, 0x10, "Communication Synchronizer"},
	{0x11, 0x20, "Signal Processing Management"},
	{0x11, 0x80, "Other Signal Processing Controller"},
};

const char *get_pci_class_name(unsigned char class, unsigned char subclass)
{
	const char *unknown_class_name = "Unknown PCI Device";
	size_t i;
	size_t class_list_size = (sizeof(pci_class_name)/sizeof(pci_class_name_t));
	switch(class) 
	{
		case 0x12:
		{
			return "Processing Accelerator";
		}
		break;
		case 0x13:
		{
			return "Non-Essential Instrumentation";
		}
		break;
		case 0x40:
		{
			return "Co-Processor";
		}
		break;
		case 0xFF:
		{
			return "Vendor Specific";
		}
		break;
		default:
		{			
			for (i = 0; i < class_list_size; i++) 
			{
				if (pci_class_name[i].class == class && pci_class_name[i].subclass == subclass) 
				{
					return pci_class_name[i].name;
				}
			}
		}
		break;
	}
	return unknown_class_name;
}

unsigned char pci_scan_device(unsigned char bus, unsigned char slot, unsigned char func, unsigned char index)
{
	unsigned short pci_vendor = pci_read_word(bus, slot, func, 2);
	if (index >= 32) return 0;
	if (pci_vendor == 0xFFFF)
	{
		return 0;
	}
	unsigned short *pci_buf = (unsigned short*)&pci_device[index].pci;
	for(unsigned char i=0;i<32;i++)
	{
		pci_buf[i] = pci_read_word(bus, slot, func, i*2);
	}
	unsigned char base_count=0;
	switch(pci_device[index].pci.header_type)
	{
		case 0:
		{
			base_count = 6;
		}
		break;
		case 1:
		{
			base_count = 2;
		}
		break;
		case 2:
		{
			base_count = 1;
			pci_scan_device(pci_read_byte(bus, slot, func, 0x18), slot, func, index);
			pci_count++;
		}
		break;
	};	
	if (base_count > 0)
	{
		for(int j=0;j<base_count;j++)
		{
			int is64=0;
			int pf=0;
			unsigned long x = pci_read_long(bus, slot, func, 16 + (j * 4));
			if ((!x) || (x == (unsigned long)~0))
			{
				continue;
			}
			if ((x & 0x01) == 0x01)
			{
				pci_device[index].pci.bar[j] = x;
			}
			else
			{
				if ((x & 0x06) != 0x04)
				{
					pci_device[index].pci.bar[j] = x;
				}
				else if (j == base_count-1)
				{
					// skip
				}
				else
				{
					unsigned long y = pci_read_long(bus, slot, func, 16 + ((++j) * 4));
					if (!y)
					{
						pci_device[index].pci.bar[j-1] = x;
					}
					else
					{
						is64=1;
					}
				}
			}
			unsigned long bar_x = pci_device[index].pci.bar[j];
			unsigned long bar_y;
			if (is64) 
			{
				bar_y = bar_x;
			}
			else
			{
				bar_y = ((~(bar_x & (~0x7ff))) + 1) & 0xffffffff;
				bar_y &= 0xFFFFFFF0;
				bar_y &= 0xFFFFFFFC;
			}
			if ((bar_y & 0x08) == 0x00)
			{
				if ((bar_y & 0xFF0000) == 0)
				{
					unsigned int bar_m_pre_fetch = -bar_y;
					// prefetch memory
					pci_device[index].pci.bar[j] = bar_m_pre_fetch;
					pf=1;
					if ((pci_device[index].pci.class == 0x03) && (pci_device[index].pci.subclass == 0x00))
					{
						if (pci_video_memory_found == 0)
						{
							pci_video_memory_address = bar_y;
							pci_video_memory_found = 1;
						}
					}
				}
			}
			if (!pf)
			{			
				if (is64) 
				{
					pci_device[index].pci.bar[j] = bar_y;
				}
				else
				{
					pci_device[index].pci.bar[j] &= 0xFFFFFFF0;
					pci_device[index].pci.bar[j] &= 0xFFFFFFFC;				
				}
			}
		}
	}
	pci_device[index].bus = bus;
	pci_device[index].slot = slot;
	pci_device[index].function = func;
	return 1;
}

unsigned long pci_config_address(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset)
{
	unsigned long l_bus = (unsigned long)bus;
	unsigned long l_slot = (unsigned long)slot;
	unsigned long l_func = (unsigned long)func;
	unsigned long l_addr = (unsigned long)((l_bus << 16) | (l_slot << 11) | (l_func << 8) | (offset & 0xfc) | ((unsigned long)0x80000000));
	return l_addr;
}

unsigned char pci_read_byte(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset)
{
	unsigned long addr = pci_config_address(bus, slot, func, offset);
	unsigned char r;
	outl(0x0CF8, (unsigned long)addr);
	r = inb(0x0CFC + (offset&3));
	return r;
}

unsigned short pci_read_word(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset)
{
	unsigned long addr = pci_config_address(bus, slot, func, offset);
	unsigned short r;
	outl(0x0CF8, (unsigned long)addr);
	r = inw(0x0CFC + (offset&2));
	return r;
}

unsigned long pci_read_long(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset)
{
	unsigned long addr = pci_config_address(bus, slot, func, offset);
	unsigned long r;
	outl(0x0CF8, (unsigned long)addr);
	r = inl(0x0CFC);
	return r;
}

void pci_write_byte(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset, unsigned char value)
{
	unsigned long addr = pci_config_address(bus, slot, func, offset);
	outl(0x0CF8, (unsigned long)addr);
	outb(0x0CFC + (offset&3), value);
}

void pci_write_word(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset, unsigned short value)
{
	unsigned long addr = pci_config_address(bus, slot, func, offset);
	outl(0x0CF8, (unsigned long)addr);
	outw(0x0CFC + (offset&2), value);
}

void pci_write_long(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset, unsigned long value)
{
	unsigned long addr = pci_config_address(bus, slot, func, offset);
	outl(0x0CF8, (unsigned long)addr);
	outl(0x0CFC, value);
}

void pci_read_data(unsigned char bus, unsigned char slot, unsigned char func, pci_t *pci)
{
	unsigned short pci_vendor = pci_read_word(bus, slot, func, 2);
	if (pci_vendor == 0xFFFF)
	{
		return;
	}
	unsigned short *pci_buf = (unsigned short*)pci;
	for(unsigned char i=0;i<32;i++)
	{
		pci_buf[i] = pci_read_word(bus, slot, func, i*2);
	}
}

void pci_write_data(unsigned char bus, unsigned char slot, unsigned char func, pci_t *pci)
{
	unsigned short pci_vendor = pci_read_word(bus, slot, func, 2);
	if (pci_vendor == 0xFFFF)
	{
		return;
	}
	unsigned short *pci_buf = (unsigned short*)pci;
	for(unsigned char i=0;i<32;i++)
	{
		pci_write_word(bus, slot, func, i*2, pci_buf[i]);
	}
}

void pci_scan_bus(unsigned char bus)
{
	unsigned char i;
	for (i=0;i<32;i++)
	{
		if (pci_scan_device(bus, i, 0, pci_count))
		{
			for(int j=0;j<8;j++)
			{
				if (pci_scan_device(bus, i, j, pci_count))
				{
					pci_count++;
				}
			}
		}
	}
}

void pci_scan(void)
{
	int i;
	pci_scan_bus(0);
	if ((pci_device[0].pci.header_type & 0x80) != 0x00)
	{
		for(i=1;i<256;i++)
		{
			pci_scan_bus(i);
		}
	}
}

void loadpci(void)
{
	int i;
	for(i=0;i<32;i++)
	{
		memset(&pci_device[i].pci, 0, sizeof(pci_t));
		pci_device[i].pci.vendor = 0xFFFF;
	}
	pci_scan();
	system_pci = (unsigned char *)0x840300;
	memset(system_pci, 0, 0x1000);
	sys_pci = (system_pci_t*)system_pci;
	strcpy(sys_pci->signature, "PCI");
	sys_pci->version = 1;
	sys_pci->count = pci_count;
	memcpy(&sys_pci->device[0], &pci_device[0], sizeof(pci_device_t)*32);
	pci_initialized = 1;
}
