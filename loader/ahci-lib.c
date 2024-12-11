// Fast System Kernel Loader - Advanced Host Controller Interface
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

unsigned long ahci_hba_address ENHANCED_DATA_SECTION = 0;
ahci_hba_port_t *ahci_port ENHANCED_DATA_SECTION;
ahci_hba_memory_t *ahci_hba ENHANCED_DATA_SECTION;
unsigned char *ahci_ptr ENHANCED_DATA_SECTION = (unsigned char *)AHCI_ADDRESS;
ahci_t *ahci ENHANCED_DATA_SECTION;

ENHANCED_CODE_SECTION int init_ahci(void)
{
	unsigned long portcnt = 0;
	if (pci_count > 0)
	{
		for(int i=0;i<pci_count;i++) 
		{
			if (pci_device[i].pci.vendor != 0xFFFF)
			{
				if ((pci_device[i].pci.class == 0x01) && (pci_device[i].pci.subclass == 0x06))
				{
					if (pci_device[i].pci.bar[5] != 0)
					{
						ahci_hba_address = pci_device[i].pci.bar[5];
						ahci_hba = (ahci_hba_memory_t*)ahci_hba_address;
						ahci_ptr = (unsigned char *)AHCI_ADDRESS;
						ahci = (ahci_t*)ahci_ptr;
						strcpy(ahci->signature, "AHCI");
						ahci->version = 1;
						portcnt = check_ahci_ports(&ahci->list[0], &ahci->count);
						ahci->list_count = portcnt;	
						init_ahci_ports();
						return 1;
					}						
				}
			}
		}
	}
	return 0;
}

ENHANCED_CODE_SECTION unsigned long get_ahci_hba(void)
{
	return ahci_hba_address;
}

ENHANCED_CODE_SECTION void *get_ahci_hba_ptr(void)
{
	unsigned char *hba_ptr = (unsigned char *)get_ahci_hba();
	return hba_ptr;
}

ENHANCED_CODE_SECTION ahci_hba_memory_t *get_ahci_hba_memory(void)
{
	ahci_hba = (ahci_hba_memory_t*)get_ahci_hba();
	return ahci_hba;
}

ENHANCED_CODE_SECTION int check_ahci_type(ahci_hba_port_t *port)
{
	unsigned long ssts = port->ssts;

	unsigned char ipm = (ssts >> 8) & 0x0F;
	unsigned char det = ssts & 0x0F;

	if (det != 3)
		return 0;
	if (ipm != 1)
		return 0;

	switch (port->sig)
	{
	case 0xEB140101:
		return 4;
	case 0xC33C0101:
		return 2;
	case 0x96690101:
		return 3;
	default:
		return 1;
	}
}

ENHANCED_CODE_SECTION int check_ahci_ports(unsigned char *ports, unsigned char *portmax)
{
	unsigned long pi = ahci_hba->pi;
	int i = 0;
	int portcount = 0;
	while (i < 32)
	{
		if (pi & 1)
		{
			int dt = check_ahci_type(&ahci_hba->ports[i]);
			if (dt == 1)
			{
				ports[portcount] = i;
				portcount++;
				*portmax = i;
				ahci->port[i].type = dt;
			}
			else if (dt == 4)
			{
				ports[portcount] = i;
				portcount++;
				*portmax = i;
				ahci->port[i].type = dt;
			}
			else if (dt == 2)
			{
				ports[portcount] = i;
				portcount++;
				*portmax = i;
				ahci->port[i].type = dt;
			}
			else if (dt == 3)
			{
				ports[portcount] = i;
				portcount++;
				*portmax = i;
				ahci->port[i].type = dt;
			}
			else
			{
				ahci->port[i].type = dt;				
			}
		}
		pi >>= 1;
		i++;
	}
	return portcount;
}

ENHANCED_CODE_SECTION int check_ahci_port(int portno)
{
	int portcnt = ahci->list_count;
	int i = 0;
	if (portcnt > 0)
	{
		while (i < portcnt)
		{
			if (ahci->list[i] == portno)
			{
				int dt = check_ahci_type(&ahci_hba->ports[portno]);
				ahci->port[portno].type = dt;
				return 1;
			}
			i++;
		}
	}
	return 0;
}

ENHANCED_CODE_SECTION void ahci_start_cmd(ahci_hba_port_t *port)
{
	while (port->cmd & 0x8000)
		;
	port->cmd |= 0x0010;
	port->cmd |= 0x0001; 
}

ENHANCED_CODE_SECTION void ahci_stop_cmd(ahci_hba_port_t *port)
{
	port->cmd &= ~0x0001;
	port->cmd &= ~0x0010;
	while(1)
	{
		if (port->cmd & 0x4000)
			continue;
		if (port->cmd & 0x8000)
			continue;
		break;
	}
}

ENHANCED_CODE_SECTION void ahci_port_rebase(ahci_hba_port_t *port, int portno)
{
	ahci_stop_cmd(port);
	port->clb = 0x400000 + (portno<<10);
	port->clbu = 0;
	memset((void*)(port->clb), 0, 1024);
	port->fb = 0x400000 + (32<<10) + (portno<<8);
	port->fbu = 0;
	memset((void*)(port->fb), 0, 256);
	ahci_hba_cmd_header_t *cmdheader = (ahci_hba_cmd_header_t*)(port->clb);
	for (int i=0; i<32; i++)
	{
		cmdheader[i].prdtl = 8;
		cmdheader[i].ctba = 0x400000 + (40<<10) + (portno<<13) + (i<<8);
		cmdheader[i].ctbau = 0;
		memset((void*)cmdheader[i].ctba, 0, 256);
	}
	ahci_start_cmd(port);
}

ENHANCED_CODE_SECTION int init_ahci_ports()
{
	unsigned long pi = ahci_hba->pi;
	int i = 0;
	int r = 0;
	int f = 0;
	while (i < 32)
	{
		if (pi & 1)
		{
			int dt = check_ahci_type(&ahci_hba->ports[i]);
			if (dt == 1)
			{
				ahci->port[i].type = dt;
				ahci_port_rebase(&ahci_hba->ports[i], i);
				if (f == 0)
				{
					ahci_port = &ahci_hba->ports[i];
					f = 1;
				}
				r = 1;
			}
			else if (dt == 4)
			{
				ahci->port[i].type = dt;
				ahci_port_rebase(&ahci_hba->ports[i], i);
				r = 1;
			}
			else
			{
				ahci->port[i].type = dt;				
			}
		}
		pi >>= 1;
		i++;
	}
	return r;
}

ENHANCED_CODE_SECTION int ahci_find_cmdslot(ahci_hba_port_t *port)
{
	unsigned long slots = (port->sact | port->ci);
	int num_of_slots = (ahci_hba->cap & 0x0f00) >> 8;
	for (int i=0; i<num_of_slots; i++)
	{
		if ((slots&1) == 0)
			return i;
		slots >>= 1;
	}
	return -1;
}

ENHANCED_CODE_SECTION unsigned char get_sata_ident(ahci_hba_port_t *port, void *buffer)
{
	int i=0,sp=0;
	port->is = (unsigned long)-1;
	int slot = ahci_find_cmdslot(port);
	if (slot == -1) return 0;
	int dt = check_ahci_type(port);
	ahci_hba_cmd_header_t *cmdheader = (ahci_hba_cmd_header_t*)port->clb;
	cmdheader += slot;
	cmdheader->cfl = sizeof(ahci_fis_reg_h2d_t)/sizeof(unsigned long);
	cmdheader->w = 0;
	cmdheader->prdtl = 1;
	ahci_hba_cmd_tbl_t *cmdtbl = (ahci_hba_cmd_tbl_t*)(cmdheader->ctba);
	memset(cmdtbl, 0, sizeof(ahci_hba_cmd_tbl_t)+(cmdheader->prdtl-1)*sizeof(ahci_hba_prdt_entry_t));
	cmdtbl->prdt_entry[i].dba = (unsigned long)buffer;
	cmdtbl->prdt_entry[i].dbc = 2048;
	cmdtbl->prdt_entry[i].i = 1;
	ahci_fis_reg_h2d_t *cmdfis = (ahci_fis_reg_h2d_t*)(&cmdtbl->cfis);
	cmdfis->fis_type = 0x27;
	cmdfis->c = 1;
	if (dt == 4) cmdfis->command = 0xA1;
	else cmdfis->command = 0xEC;
	cmdfis->device = 0;
	cmdfis->countl = 1;
	cmdfis->counth = 0;
	while ((port->tfd & (0x80|0x08)) && (sp<1000000)) sp++;
	if (sp==1000000) return 0;
	port->ci = 1<<slot;
	while (1)
	{
		if ((port->ci & (1<<slot)) == 0) break;
		if (port->is & (1 << 30)) return 0;
	}
	if (port->is & (1 << 30)) return 0;
	return 1;
}

ENHANCED_CODE_SECTION unsigned long sata_read(int id, void *buffer, unsigned long sector, unsigned long count)
{
	int i=0,sp=0;
	if (ahci == NULL) return 0;
	if (ahci->list == NULL) return 0;
	int index = ahci->list[id];
	ahci_hba_port_t *port = &ahci_hba->ports[index];
	if (port == NULL) return 0;
	port->is = (unsigned long)-1;
	int slot = ahci_find_cmdslot(port);
	if (slot == -1) return 0;
	ahci_hba_cmd_header_t *cmdheader = (ahci_hba_cmd_header_t*)port->clb;
	cmdheader += slot;
	cmdheader->cfl = sizeof(ahci_fis_reg_h2d_t)/sizeof(unsigned long);
	cmdheader->w = 0;
	cmdheader->prdtl = (unsigned short)((count-1)>>4)+1;
	ahci_hba_cmd_tbl_t *cmdtbl = (ahci_hba_cmd_tbl_t*)(cmdheader->ctba);
	memset(cmdtbl, 0, sizeof(ahci_hba_cmd_tbl_t)+(cmdheader->prdtl-1)*sizeof(ahci_hba_prdt_entry_t));
	for (i=0;i<(cmdheader->prdtl-1);i++)
	{
		cmdtbl->prdt_entry[i].dba = (unsigned long)buffer;
		cmdtbl->prdt_entry[i].dbc = 8*1024-1;
		cmdtbl->prdt_entry[i].i = 1;
		buffer += 4*1024;
		count -= 16;
	}
	cmdtbl->prdt_entry[i].dba = (unsigned long)buffer;
	cmdtbl->prdt_entry[i].dbc = (count<<9)-1;
	cmdtbl->prdt_entry[i].i = 1;
	ahci_fis_reg_h2d_t *cmdfis = (ahci_fis_reg_h2d_t*)(&cmdtbl->cfis);
	cmdfis->fis_type = 0x27;
	cmdfis->c = 1;
	cmdfis->command = 0x25;
	cmdfis->lba0 = (unsigned char)sector;
	cmdfis->lba1 = (unsigned char)(sector>>8);
	cmdfis->lba2 = (unsigned char)(sector>>16);
	cmdfis->lba3 = (unsigned char)(sector>>24);
	cmdfis->lba4 = 0;
	cmdfis->lba5 = 0;
	cmdfis->device = 1<<6;
	cmdfis->countl = count & 0xFF;
	cmdfis->counth = (count>>8) & 0xFF;
	while ((port->tfd & (0x80|0x08)) && (sp<1000000)) sp++;
	if (sp==1000000) return 0;
	port->ci = 1<<slot;
	while(1)
	{
		if ((port->ci & (1<<slot)) == 0) break;
		if (port->is & (1 << 30)) return 0;
	}
	if (port->is & (1 << 30)) return 0;
	return count;
}

ENHANCED_CODE_SECTION unsigned char get_sata_name(ahci_hba_port_t *port, char *s)
{
	unsigned long i;
	unsigned char st = 0;
	unsigned short buf[256];
	char *b = (char*)buf;
	int dt = check_ahci_type(port);
	if ((dt == 1) || (dt == 4))
	{
		if (get_sata_ident(port, (void*)&buf[0]))
		{
			for(i=0;i<512;i+=2) 
			{
				st = b[i];
				b[i] = b[i + 1];
				b[i + 1] = st;
			}
			b[256]=0;
			strcpy(s, &b[54]);
			s[40] = 0;
		}
	}
	else
	{
		return 0;
	}
	return 1;
}

ENHANCED_CODE_SECTION void detectahci(void)
{
	int i=0;
	char buffer[512];
	unsigned long pi = ahci_hba->pi;
	while (i<32)
	{
		if (pi & 1)
		{
			int dt = ahci->port[i].type;
			if ((dt == 1) || (dt == 4))
			{
				memset(buffer, 0, 512);
				if (get_sata_name(&ahci_hba->ports[i], &buffer[0]))
				{
					if (dt == 1)
					{
						if (strlen(buffer) == 0)
						{
							strcpy(buffer, "Virtual HD");
						}
						printk("AHCI SATA Drive %d: %s\n", i, buffer);
					}
					else if (dt == 4)
					{
						if (strlen(buffer) == 0)
						{
							strcpy(buffer, "Virtual CD");
						}
						printk("AHCI SATA Drive %d: %s\n", i, buffer);
					}					
				}
			}
			else
			{
				printk("AHCI SATA Drive %d: None\n", i);
			}
		}
		pi >>= 1;
		i++;
	}
}

ENHANCED_CODE_SECTION void dump_hex(const void *data, size_t size) 
{
    const unsigned char *buffer = (const unsigned char *)data;
    size_t i, j;
    for (i = 0; i < size; i += 16) 
	{
        printk("%06x: ", (unsigned int)i);
        for (j = 0; j < 16; j++) {
            if (i + j < size)
                printk("%02x ", buffer[i + j]);
            else
                printk("   ");
            if (j % 16 == 7)
                printk(" ");
        }
        printk("\n");
    }
}

ENHANCED_CODE_SECTION void ahci_port_rebase(ahci_hba_port_t *port, int portno);
ENHANCED_CODE_SECTION int init_ahci_ports();
ENHANCED_CODE_SECTION int ahci_find_cmdslot(ahci_hba_port_t *port);
ENHANCED_CODE_SECTION unsigned char get_sata_ident(ahci_hba_port_t *port, void *buffer);
ENHANCED_CODE_SECTION unsigned long sata_read(int id, void *buffer, unsigned long sector, unsigned long count);
ENHANCED_CODE_SECTION unsigned char get_sata_name(ahci_hba_port_t *port, char *s);
ENHANCED_CODE_SECTION void detectahci(void);
ENHANCED_CODE_SECTION void dump_hex(const void *data, size_t size);