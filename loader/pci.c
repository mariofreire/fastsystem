// Fast System Kernel Loader - Peripheral Component Interconnect
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2024 DSP Interactive.

#include <stdarg.h>
#include "enum.h"
#include "funcbase.h"
#include "ecs.h"
#include "pci.h"
#include "ahci.h"
//#include "ehci.h"
//#include "ohci.h"
//#include "uhci.h"
//#include "xhci.h"

pci_device_t pci_device[32] ENHANCED_DATA_SECTION;
unsigned char pci_count ENHANCED_DATA_SECTION = 0;

ENHANCED_CODE_SECTION unsigned long pci_config_address(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset)
{
	unsigned long l_bus = (unsigned long)bus;
	unsigned long l_slot = (unsigned long)slot;
	unsigned long l_func = (unsigned long)func;
	unsigned long l_addr = (unsigned long)((l_bus << 16) | (l_slot << 11) | (l_func << 8) | (offset & 0xfc) | ((unsigned long)0x80000000));
	return l_addr;
}

ENHANCED_CODE_SECTION unsigned char pci_read_byte(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset)
{
	unsigned long addr = pci_config_address(bus, slot, func, offset);
	unsigned char r;
	outl(0x0CF8, (unsigned long)addr);
	r = inb(0x0CFC + (offset&3));
	return r;
}

ENHANCED_CODE_SECTION unsigned short pci_read_word(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset)
{
	unsigned long addr = pci_config_address(bus, slot, func, offset);
	unsigned short r;
	outl(0x0CF8, (unsigned long)addr);
	r = inw(0x0CFC + (offset&2));
	return r;
}

ENHANCED_CODE_SECTION unsigned long pci_read_long(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset)
{
	unsigned long addr = pci_config_address(bus, slot, func, offset);
	unsigned long r;
	outl(0x0CF8, (unsigned long)addr);
	r = inl(0x0CFC);
	return r;
}

ENHANCED_CODE_SECTION unsigned char pci_scan_device(unsigned char bus, unsigned char slot, unsigned char func, unsigned char index)
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

ENHANCED_CODE_SECTION void pci_scan_bus(unsigned char bus)
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

ENHANCED_CODE_SECTION void pci_scan(void)
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

ENHANCED_CODE_SECTION void loadpci(void)
{
	int i;
	pci_count = 0;
	for(i=0;i<32;i++)
	{
		memset(&pci_device[i].pci, 0, sizeof(pci_t));
		pci_device[i].pci.vendor = 0xFFFF;
	}
	pci_scan();
}



