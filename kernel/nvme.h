// Fast System Kernel - Non-Volatile Memory Express
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2025 DSP Interactive.

#ifndef __NVME_H__
#define __NVME_H__

#define NVME_ADDRESS 0x843000

#define NVME_CMD_READ 0x02
#define NVME_QUEUE_ENTRY_SIZE 64
#define NVME_PRP_SIZE 4096

#define NVME_SUCCESS 0x0000
#define NVME_NO_COMPLETE 0xFFFF


#pragma pack (push, 1)

typedef struct 
{
    uint8_t opcode;
    uint8_t flags;
    uint16_t cid;
    uint32_t nsid;
    uint32_t rsvd1;
    uint32_t rsvd2;
    uint32_t mptr_lo;
    uint32_t mptr_hi;
    uint32_t dptr_prp1_lo;
    uint32_t dptr_prp1_hi;
    uint32_t dptr_prp2_lo;
    uint32_t dptr_prp2_hi;
    uint32_t cdw10;
    uint32_t cdw11;
    uint32_t cdw12;
    uint32_t cdw13;
    uint32_t cdw14;
    uint32_t cdw15;
} nvme_command_t;

typedef struct 
{
    uint32_t address_lo;
    uint32_t address_hi;
    uint32_t size_lo;
    uint32_t size_hi;
} nvme_queue_t;

typedef struct 
{
    uint32_t dw0;
    uint32_t dw1;
    uint32_t dw2;
    uint32_t dw3;
} nvme_completion_t;

typedef struct 
{
	uint32_t    result;
	uint32_t    rsvd;
	uint16_t    sq_head;
	uint16_t    sq_id;
	uint16_t    cid;
	uint16_t    status;
} nvme_cqe_t;

typedef struct 
{
	uint32_t cap_lo;      
	uint32_t cap_hi;      
    uint32_t vs;          
    uint32_t intms;       
    uint32_t intmc;       
    uint32_t cc;          
    uint32_t csts;        
    uint32_t aqa;         
    uint32_t asq;         
    uint32_t acq;         
    uint32_t sqxtdbl_lo;  
	uint32_t sqxtdbl_hi;  
    uint32_t cqxhdbl_lo;  
	uint32_t cqxhdbl_hi;	
} nvme_controller_t;

typedef struct
{
	char signature[4];
	unsigned char version;
	unsigned char count;
} nvme_t;

#pragma pack (pop)

extern unsigned long nvme_bar_address;
extern nvme_controller_t *nvme_controller;
extern unsigned char *nvme_ptr;
extern nvme_t *nvme;

extern int init_nvme(void);
extern int init_nvme_ports();
extern nvme_controller_t *get_nvme_controller(void);
extern void *get_nvme_bar_ptr(void);
extern unsigned long get_nvme_bar(void);

#endif // __NVME_H__
