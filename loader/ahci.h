// Fast System Kernel Loader - Advanced Host Controller Interface
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2024 DSP Interactive.

#ifndef __AHCI_H__
#define __AHCI_H__

#define AHCI_ADDRESS 0x841500

#pragma pack (push, 1)

typedef struct
{
	unsigned long clb;
	unsigned long clbu;
	unsigned long fb;
	unsigned long fbu;
	unsigned long is;
	unsigned long ie;
	unsigned long cmd;
	unsigned long rsv0;
	unsigned long tfd;
	unsigned long sig;
	unsigned long ssts;
	unsigned long sctl;
	unsigned long serr;
	unsigned long sact;
	unsigned long ci;
	unsigned long sntf;
	unsigned long fbs;
	unsigned long rsv1[11];
	unsigned long vendor[4];
} ahci_hba_port_t;

typedef struct
{
	unsigned long cap;
	unsigned long ghc;
	unsigned long is;
	unsigned long pi;
	unsigned long vs;
	unsigned long ccc_ctl;
	unsigned long ccc_pts;
	unsigned long em_loc;
	unsigned long em_ctl;
	unsigned long cap2;
	unsigned long bohc;
	unsigned char  rsv[0xA0-0x2C];
	unsigned char  vendor[0x100-0xA0];
	ahci_hba_port_t	ports[32];
} ahci_hba_memory_t;

typedef struct
{
	unsigned long dba;
	unsigned long dbau;
	unsigned long rsv0;
	unsigned long dbc:22;
	unsigned long rsv1:9;
	unsigned long i:1;
} ahci_hba_prdt_entry_t;

typedef struct
{
	unsigned char cfis[64];
	unsigned char acmd[16];
	unsigned char rsv[48];
	ahci_hba_prdt_entry_t prdt_entry[1];
} ahci_hba_cmd_tbl_t;

typedef struct
{
	unsigned char  cfl:5;
	unsigned char  a:1;
	unsigned char  w:1;
	unsigned char  p:1;
	unsigned char  r:1;
	unsigned char  b:1;
	unsigned char  c:1;
	unsigned char  rsv0:1;
	unsigned char  pmp:4;
	unsigned short prdtl;
	volatile unsigned long prdbc;
	unsigned long ctba;
	unsigned long ctbau;
	unsigned long rsv1[4];
} ahci_hba_cmd_header_t;

typedef struct
{
	unsigned char fis_type;
	unsigned char pmport:4;
	unsigned char rsv0:3;
	unsigned char c:1;
	unsigned char command;
	unsigned char featurel;
	unsigned char lba0;
	unsigned char lba1;
	unsigned char lba2;
	unsigned char device;
	unsigned char lba3;
	unsigned char lba4;
	unsigned char lba5;
	unsigned char featureh;
	unsigned char countl;
	unsigned char counth;
	unsigned char icc;
	unsigned char control;
	unsigned char rsv1[4];
} ahci_fis_reg_h2d_t;

typedef struct
{
	unsigned long type;
} ahci_port_t;

#pragma pack (pop)

extern unsigned long ahci_hba_address ENHANCED_DATA_SECTION;
extern ahci_hba_port_t *ahci_hba_port ENHANCED_DATA_SECTION;
extern ahci_hba_memory_t *ahci_hba ENHANCED_DATA_SECTION;
extern unsigned char ahci_port_count ENHANCED_DATA_SECTION;
extern ahci_port_t ahci_port[32] ENHANCED_DATA_SECTION;
extern unsigned char ahci_list_count ENHANCED_DATA_SECTION;
extern unsigned char ahci_list[32] ENHANCED_DATA_SECTION;

extern int init_ahci(void) ENHANCED_CODE_SECTION;
extern int init_ahci_ports() ENHANCED_CODE_SECTION;
extern void detectahci(void) ENHANCED_CODE_SECTION;
extern int check_ahci_ports(void) ENHANCED_CODE_SECTION;
extern int check_ahci_type(ahci_hba_port_t *port) ENHANCED_CODE_SECTION;
extern void ahci_start_cmd(ahci_hba_port_t *port) ENHANCED_CODE_SECTION;
extern void ahci_stop_cmd(ahci_hba_port_t *port) ENHANCED_CODE_SECTION;
extern unsigned long sata_read(int id, void *buffer, unsigned long sector, unsigned long count) ENHANCED_CODE_SECTION;
extern unsigned char get_sata_ident(ahci_hba_port_t *port, void *buffer) ENHANCED_CODE_SECTION;

#endif // __AHCI_H__
