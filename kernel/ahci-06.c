// Fast System AHCI
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2024 DSP Interactive.

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "pci.h"

#define AHCI_ADDRESS 0x841500

#define isdigit(c)                      ((c >= '0') && (c <= '9'))

#pragma pack (push, 1)

typedef struct
{
	uint32_t clb;
	uint32_t clbu;
	uint32_t fb;
	uint32_t fbu;
	uint32_t is;
	uint32_t ie;
	uint32_t cmd;
	uint32_t rsv0;
	uint32_t tfd;
	uint32_t sig;
	uint32_t ssts;
	uint32_t sctl;
	uint32_t serr;
	uint32_t sact;
	uint32_t ci;
	uint32_t sntf;
	uint32_t fbs;
	uint32_t rsv1[11];
	uint32_t vendor[4];
} ahci_hba_port_t;

typedef struct
{
	uint32_t cap;
	uint32_t ghc;
	uint32_t is;
	uint32_t pi;
	uint32_t vs;
	uint32_t ccc_ctl;
	uint32_t ccc_pts;
	uint32_t em_loc;
	uint32_t em_ctl;
	uint32_t cap2;
	uint32_t bohc;
	uint8_t  rsv[0xA0-0x2C];
	uint8_t  vendor[0x100-0xA0];
	ahci_hba_port_t	ports[32];
} ahci_hba_memory_t;

typedef struct
{
	uint32_t dba;
	uint32_t dbau;
	uint32_t rsv0;
	uint32_t dbc:22;
	uint32_t rsv1:9;
	uint32_t i:1;
} ahci_hba_prdt_entry_t;

typedef struct
{
	uint8_t cfis[64];
	uint8_t acmd[16];
	uint8_t rsv[48];
	ahci_hba_prdt_entry_t prdt_entry[1];
} ahci_hba_cmd_tbl_t;

typedef struct
{
	uint8_t  cfl:5;
	uint8_t  a:1;
	uint8_t  w:1;
	uint8_t  p:1;
	uint8_t  r:1;
	uint8_t  b:1;
	uint8_t  c:1;
	uint8_t  rsv0:1;
	uint8_t  pmp:4;
	uint16_t prdtl;
	volatile uint32_t prdbc;
	uint32_t ctba;
	uint32_t ctbau;
	uint32_t rsv1[4];
} ahci_hba_cmd_header_t;

typedef struct
{
	uint8_t fis_type;
	uint8_t pmport:4;
	uint8_t rsv0:3;
	uint8_t c:1;
	uint8_t command;
	uint8_t featurel;
	uint8_t lba0;
	uint8_t lba1;
	uint8_t lba2;
	uint8_t device;
	uint8_t lba3;
	uint8_t lba4;
	uint8_t lba5;
	uint8_t featureh;
	uint8_t countl;
	uint8_t counth;
	uint8_t icc;
	uint8_t control;
	uint8_t rsv1[4];
} ahci_fis_reg_h2d_t;

typedef struct
{
	uint32_t type;
} ahci_port_t;

typedef struct
{
	char signature[4];
	uint8_t version;
	uint8_t count;
	ahci_port_t port[32];
	uint8_t list_count;
	uint8_t list[32];
} ahci_t;

#pragma pack (pop)


uint32_t ahci_hba_address = 0;
ahci_hba_port_t *ahci_port;
ahci_hba_memory_t *ahci_hba;
unsigned char *ahci_ptr = (unsigned char *)AHCI_ADDRESS;
ahci_t *ahci;


int check_ahci_ports(uint8_t *ports, uint8_t *portmax);
int init_ahci_ports();

int init_ahci(void)
{
	uint32_t portcnt = 0;
	if (init_pci())
	{
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
	}
	return 0;
}

uint32_t get_ahci_hba(void)
{
	return ahci_hba_address;
}

void *get_ahci_hba_ptr(void)
{
	unsigned char *hba_ptr = (unsigned char *)get_ahci_hba();
	return hba_ptr;
}

ahci_hba_memory_t *get_ahci_hba_memory(void)
{
	ahci_hba = (ahci_hba_memory_t*)get_ahci_hba();
	return ahci_hba;
}

static int check_ahci_type(ahci_hba_port_t *port)
{
	uint32_t ssts = port->ssts;

	uint8_t ipm = (ssts >> 8) & 0x0F;
	uint8_t det = ssts & 0x0F;

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

int check_ahci_ports(uint8_t *ports, uint8_t *portmax)
{
	uint32_t pi = ahci_hba->pi;
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

int check_ahci_port(int portno)
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

void ahci_start_cmd(ahci_hba_port_t *port)
{
	while (port->cmd & 0x8000)
		;
	port->cmd |= 0x0010;
	port->cmd |= 0x0001; 
}

void ahci_stop_cmd(ahci_hba_port_t *port)
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

void ahci_port_rebase(ahci_hba_port_t *port, int portno)
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

int init_ahci_ports()
{
	uint32_t pi = ahci_hba->pi;
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

int ahci_find_cmdslot(ahci_hba_port_t *port)
{
	uint32_t slots = (port->sact | port->ci);
	int num_of_slots = (ahci_hba->cap & 0x0f00) >> 8;
	for (int i=0; i<num_of_slots; i++)
	{
		if ((slots&1) == 0)
			return i;
		slots >>= 1;
	}
	return -1;
}

unsigned char get_sata_ident(ahci_hba_port_t *port, void *buffer)
{
	int i=0,sp=0;
	port->is = (uint32_t)-1;
	int slot = ahci_find_cmdslot(port);
	if (slot == -1) return 0;
	int dt = check_ahci_type(port);
	ahci_hba_cmd_header_t *cmdheader = (ahci_hba_cmd_header_t*)port->clb;
	cmdheader += slot;
	cmdheader->cfl = sizeof(ahci_fis_reg_h2d_t)/sizeof(uint32_t);
	cmdheader->w = 0;
	cmdheader->prdtl = 1;
	ahci_hba_cmd_tbl_t *cmdtbl = (ahci_hba_cmd_tbl_t*)(cmdheader->ctba);
	memset(cmdtbl, 0, sizeof(ahci_hba_cmd_tbl_t)+(cmdheader->prdtl-1)*sizeof(ahci_hba_prdt_entry_t));
	cmdtbl->prdt_entry[i].dba = (uint32_t)buffer;
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

uint32_t sata_read(int id, void *buffer, uint32_t sector, uint32_t count)
{
	int i=0,sp=0;
	if (ahci == NULL) return 0;
	if (ahci->list == NULL) return 0;
	int index = ahci->list[id];
	ahci_hba_port_t *port = &ahci_hba->ports[index];
	if (port == NULL) return 0;
	port->is = (uint32_t)-1;
	int slot = ahci_find_cmdslot(port);
	if (slot == -1) return 0;
	ahci_hba_cmd_header_t *cmdheader = (ahci_hba_cmd_header_t*)port->clb;
	cmdheader += slot;
	cmdheader->cfl = sizeof(ahci_fis_reg_h2d_t)/sizeof(uint32_t);
	cmdheader->w = 0;
	cmdheader->prdtl = (uint16_t)((count-1)>>4)+1;
	ahci_hba_cmd_tbl_t *cmdtbl = (ahci_hba_cmd_tbl_t*)(cmdheader->ctba);
	memset(cmdtbl, 0, sizeof(ahci_hba_cmd_tbl_t)+(cmdheader->prdtl-1)*sizeof(ahci_hba_prdt_entry_t));
	for (i=0;i<(cmdheader->prdtl-1);i++)
	{
		cmdtbl->prdt_entry[i].dba = (uint32_t)buffer;
		cmdtbl->prdt_entry[i].dbc = 8*1024-1;
		cmdtbl->prdt_entry[i].i = 1;
		buffer += 4*1024;
		count -= 16;
	}
	cmdtbl->prdt_entry[i].dba = (uint32_t)buffer;
	cmdtbl->prdt_entry[i].dbc = (count<<9)-1;
	cmdtbl->prdt_entry[i].i = 1;
	ahci_fis_reg_h2d_t *cmdfis = (ahci_fis_reg_h2d_t*)(&cmdtbl->cfis);
	cmdfis->fis_type = 0x27;
	cmdfis->c = 1;
	cmdfis->command = 0x25;
	cmdfis->lba0 = (uint8_t)sector;
	cmdfis->lba1 = (uint8_t)(sector>>8);
	cmdfis->lba2 = (uint8_t)(sector>>16);
	cmdfis->lba3 = (uint8_t)(sector>>24);
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

unsigned char get_sata_name(ahci_hba_port_t *port, char *s)
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

void detectahci(void)
{
	int i=0;
	char buffer[512];
	uint32_t pi = ahci_hba->pi;
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
						printf("AHCI SATA Drive %d: %s\n", i, buffer);
					}
					else if (dt == 4)
					{
						if (strlen(buffer) == 0)
						{
							strcpy(buffer, "Virtual CD");
						}
						printf("AHCI SATA Drive %d: %s\n", i, buffer);
					}					
				}
			}
			else
			{
				printf("AHCI SATA Drive %d: None\n", i);
			}
		}
		pi >>= 1;
		i++;
	}
}

void dump_hex(const void *data, size_t size) 
{
    const unsigned char *buffer = (const unsigned char *)data;
    size_t i, j;
    for (i = 0; i < size; i += 16) 
	{
        printf("%06x: ", (unsigned int)i);
        for (j = 0; j < 16; j++) {
            if (i + j < size)
                printf("%02x ", buffer[i + j]);
            else
                printf("   ");
            if (j % 16 == 7)
                printf(" ");
        }
        printf("\n");
    }
}

int main(int argc, char *argv[]) 
{
	int id=0,sector_start=0;
	uint8_t sector[512];
	memset(sector, 0, 512);
	if (init_ahci())
	{
		printf("Detecting Virtual AHCI...\n");
		detectahci();
		if (argc > 1)
		{
			if (isdigit(argv[1][0]))
			{
				id = atol(argv[1]);
			}
			if (argc > 2)
			{
				if (isdigit(argv[2][0]))
				{
					sector_start = atol(argv[2]);
				}	
			}
		}
		if (sata_read(id, &sector[0], sector_start, 1))
		{
			dump_hex(sector, 512);
		}
	}
	return 0;
}

