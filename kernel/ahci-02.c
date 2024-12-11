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

#define ATA_DEV_BUSY 0x80
#define ATA_DEV_DRQ 0x08
#define HBA_PxIS_TFES   (1 << 30)
#define ATA_CMD_READ_DMA_EX     0x25
#define ATA_CMD_WRITE_DMA_EX     0x35

#define	AHCI_BASE	0x400000

#define HBA_PxCMD_ST    0x0001
#define HBA_PxCMD_FRE   0x0010
#define HBA_PxCMD_FR    0x4000
#define HBA_PxCMD_CR    0x8000

#define	SATA_SIG_ATA	0x00000101
#define	SATA_SIG_ATAPI	0xEB140101
#define	SATA_SIG_SEMB	0xC33C0101
#define	SATA_SIG_PM		0x96690101

#define AHCI_DEV_NULL 0
#define AHCI_DEV_SATA 1
#define AHCI_DEV_SEMB 2
#define AHCI_DEV_PM 3
#define AHCI_DEV_SATAPI 4

#define HBA_PORT_IPM_ACTIVE 1
#define HBA_PORT_DET_PRESENT 3

#define TRUE 1
#define FALSE 0

typedef enum
{
	FIS_TYPE_REG_H2D	= 0x27,
	FIS_TYPE_REG_D2H	= 0x34,
	FIS_TYPE_DMA_ACT	= 0x39,
	FIS_TYPE_DMA_SETUP	= 0x41,
	FIS_TYPE_DATA		= 0x46,
	FIS_TYPE_BIST		= 0x58,
	FIS_TYPE_PIO_SETUP	= 0x5F,
	FIS_TYPE_DEV_BITS	= 0xA1,
} FIS_TYPE;

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


int check_ports(ahci_hba_memory_t *abar, uint8_t *ports, uint8_t *portmax);
int init_ports(ahci_hba_memory_t *abar);

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
							portcnt = check_ports(ahci_hba, &ahci->list[0], &ahci->count);
							ahci->list_count = portcnt;	
							init_ports(ahci_hba);
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

static int check_type(ahci_hba_port_t *port)
{
	uint32_t ssts = port->ssts;

	uint8_t ipm = (ssts >> 8) & 0x0F;
	uint8_t det = ssts & 0x0F;

	if (det != HBA_PORT_DET_PRESENT)
		return AHCI_DEV_NULL;
	if (ipm != HBA_PORT_IPM_ACTIVE)
		return AHCI_DEV_NULL;

	switch (port->sig)
	{
	case SATA_SIG_ATAPI:
		return AHCI_DEV_SATAPI;
	case SATA_SIG_SEMB:
		return AHCI_DEV_SEMB;
	case SATA_SIG_PM:
		return AHCI_DEV_PM;
	default:
		return AHCI_DEV_SATA;
	}
}

int check_ports(ahci_hba_memory_t *abar, uint8_t *ports, uint8_t *portmax)
{
	uint32_t pi = abar->pi;
	int i = 0;
	int portcount = 0;
	while (i < 32)
	{
		if (pi & 1)
		{
			int dt = check_type(&abar->ports[i]);
			if (dt == AHCI_DEV_SATA)
			{
				ports[portcount] = i;
				portcount++;
				*portmax = i;
				ahci->port[i].type = dt;
			}
			else if (dt == AHCI_DEV_SATAPI)
			{
				ports[portcount] = i;
				portcount++;
				*portmax = i;
				ahci->port[i].type = dt;
			}
			else if (dt == AHCI_DEV_SEMB)
			{
				ports[portcount] = i;
				portcount++;
				*portmax = i;
				ahci->port[i].type = dt;
			}
			else if (dt == AHCI_DEV_PM)
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

int check_port(int portno)
{
	int portcnt = ahci->list_count;
	int i = 0;
	if (portcnt > 0)
	{
		while (i < portcnt)
		{
			if (ahci->list[i] == portno)
			{
				int dt = check_type(&ahci_hba->ports[portno]);
				ahci->port[portno].type = dt;
				return 1;
			}
			i++;
		}
	}
	return 0;
}

void probe_port(void)
{
	uint32_t pi = ahci_hba->pi;
	int i = 0;
	while (i<32)
	{
		if (pi & 1)
		{
			int dt = ahci->port[i].type;
			if (dt == AHCI_DEV_SATA)
			{
				printf("SATA drive found at port %d\n", i);
			}
			else if (dt == AHCI_DEV_SATAPI)
			{
				printf("SATAPI drive found at port %d\n", i);
			}
			else if (dt == AHCI_DEV_SEMB)
			{
				printf("SEMB drive found at port %d\n", i);
			}
			else if (dt == AHCI_DEV_PM)
			{
				printf("PM drive found at port %d\n", i);
			}
			else
			{
				printf("No drive found at port %d\n", i);
			}
		}
		pi >>= 1;
		i ++;
	}
}

void start_cmd(ahci_hba_port_t *port)
{
	while (port->cmd & HBA_PxCMD_CR)
		;
	port->cmd |= HBA_PxCMD_FRE;
	port->cmd |= HBA_PxCMD_ST; 
}

void stop_cmd(ahci_hba_port_t *port)
{
	port->cmd &= ~HBA_PxCMD_ST;
	port->cmd &= ~HBA_PxCMD_FRE;
	while(1)
	{
		if (port->cmd & HBA_PxCMD_FR)
			continue;
		if (port->cmd & HBA_PxCMD_CR)
			continue;
		break;
	}
}

void port_rebase(ahci_hba_port_t *port, int portno)
{
	stop_cmd(port);
	port->clb = AHCI_BASE + (portno<<10);
	port->clbu = 0;
	memset((void*)(port->clb), 0, 1024);
	port->fb = AHCI_BASE + (32<<10) + (portno<<8);
	port->fbu = 0;
	memset((void*)(port->fb), 0, 256);
	ahci_hba_cmd_header_t *cmdheader = (ahci_hba_cmd_header_t*)(port->clb);
	for (int i=0; i<32; i++)
	{
		cmdheader[i].prdtl = 8;
		cmdheader[i].ctba = AHCI_BASE + (40<<10) + (portno<<13) + (i<<8);
		cmdheader[i].ctbau = 0;
		memset((void*)cmdheader[i].ctba, 0, 256);
	}
	start_cmd(port);
}

int init_ports(ahci_hba_memory_t *abar)
{
	uint32_t pi = abar->pi;
	int i = 0;
	int r = 0;
	int f = 0;
	while (i < 32)
	{
		if (pi & 1)
		{
			int dt = check_type(&abar->ports[i]);
			if (dt == AHCI_DEV_SATA)
			{
				ahci->port[i].type = dt;
				port_rebase(&abar->ports[i], i);
				if (f == 0)
				{
					ahci_port = &abar->ports[i];
					f = 1;
				}
				r = 1;
			}
			else if (dt == AHCI_DEV_SATAPI)
			{
				ahci->port[i].type = dt;
				port_rebase(&abar->ports[i], i);
				r = 1;
			}
			else if (dt == AHCI_DEV_SEMB)
			{
				ahci->port[i].type = dt;
				port_rebase(&abar->ports[i], i);
				r = 1;
			}
			else if (dt == AHCI_DEV_PM)
			{
				ahci->port[i].type = dt;
				port_rebase(&abar->ports[i], i);
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

int find_cmdslot(ahci_hba_port_t *port)
{
	uint32_t slots = (port->sact | port->ci);
	int num_of_slots = (ahci_hba->cap & 0x0f00) >> 8;
	for (int i=0; i<num_of_slots; i++)
	{
		if ((slots&1) == 0)
			return i;
		slots >>= 1;
	}
	printf("Cannot find free command list entry\n");
	return -1;
}

bool ahci_read(ahci_hba_port_t *port, uint32_t startl, uint32_t starth, uint32_t count, uint16_t *buf)
{
	int i=0;
	port->is = (uint32_t) -1;
	int spin = 0;
	int slot = find_cmdslot(port);
	if (slot == -1)
		return false;
	ahci_hba_cmd_header_t *cmdheader = (ahci_hba_cmd_header_t*)port->clb;
	cmdheader += slot;
	cmdheader->cfl = sizeof(ahci_fis_reg_h2d_t)/sizeof(uint32_t);
	cmdheader->w = 0;
	cmdheader->prdtl = (uint16_t)((count-1)>>4) + 1;
	ahci_hba_cmd_tbl_t *cmdtbl = (ahci_hba_cmd_tbl_t*)(cmdheader->ctba);
	memset(cmdtbl, 0, sizeof(ahci_hba_cmd_tbl_t) + (cmdheader->prdtl-1)*sizeof(ahci_hba_prdt_entry_t));
	for (i=0; i<cmdheader->prdtl-1; i++)
	{
		cmdtbl->prdt_entry[i].dba = (uint32_t) buf;
		cmdtbl->prdt_entry[i].dbc = 8*1024-1;
		cmdtbl->prdt_entry[i].i = 1;
		buf += 4*1024;
		count -= 16;
	}
	cmdtbl->prdt_entry[i].dba = (uint32_t) buf;
	cmdtbl->prdt_entry[i].dbc = (count<<9)-1;
	cmdtbl->prdt_entry[i].i = 1;
	ahci_fis_reg_h2d_t *cmdfis = (ahci_fis_reg_h2d_t*)(&cmdtbl->cfis);
	cmdfis->fis_type = FIS_TYPE_REG_H2D;
	cmdfis->c = 1;
	cmdfis->command = ATA_CMD_READ_DMA_EX;
	cmdfis->lba0 = (uint8_t)startl;
	cmdfis->lba1 = (uint8_t)(startl>>8);
	cmdfis->lba2 = (uint8_t)(startl>>16);
	cmdfis->device = 1<<6;
	cmdfis->lba3 = (uint8_t)(startl>>24);
	cmdfis->lba4 = (uint8_t)starth;
	cmdfis->lba5 = (uint8_t)(starth>>8);
	cmdfis->countl = count & 0xFF;
	cmdfis->counth = (count >> 8) & 0xFF;
	while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
	{
		spin++;
	}
	if (spin == 1000000)
	{
		printf("Port is hung\n");
		return FALSE;
	}
	port->ci = 1<<slot;
	while (1)
	{
		if ((port->ci & (1<<slot)) == 0) 
			break;
		if (port->is & HBA_PxIS_TFES)
		{
			printf("Read disk error\n");
			return FALSE;
		}
	}
	if (port->is & HBA_PxIS_TFES)
	{
		printf("Read disk error\n");
		return FALSE;
	}
	return true;
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

int main() 
{
	char sector[512];
	if (init_ahci())
	{
		printf("AHCI HBA: 0x%X\n", get_ahci_hba());
		probe_port();
		int dt = check_type(ahci_port);
		if (dt == AHCI_DEV_SATA)
		{
			printf("SATA Found.\n");
			if (ahci_read(ahci_port, 63, 0, 1, (uint16_t*)&sector[0]))
			{
				dump_hex(sector, 512);
			}
		}
	}
	return 0;
}


