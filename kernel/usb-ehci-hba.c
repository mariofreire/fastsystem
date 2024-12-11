// Fast System USB EHCI
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2024 DSP Interactive.

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "pci.h"

#define USB_EHCI_ADDRESS 0x842B00

#define TRUE 1
#define FALSE 0

#pragma pack (push, 1)

typedef struct
{
	char signature[4];
	uint8_t version;
} usb_ehci_t;

typedef struct
{
    uint8_t cap_length;
	uint8_t reserved;
    uint16_t version;
    uint32_t sparams;
    uint32_t cparams;
    uint32_t sp_portroute_h;
    uint32_t sp_portroute_l;
} usb_ehci_hba_t;

typedef struct
{
    uint32_t usb_cmd;
    uint32_t usb_sts;
    uint32_t usb_intr;
    uint32_t frameindex;
    uint32_t ctrl_ds_segment;
    uint32_t periodiclistbase;
    uint32_t asynclistaddr;
    uint32_t configflag;
    uint32_t portsc[15];
} usb_ehci_op_regs_t;

typedef struct
{
    unsigned int run_stop : 1;
    unsigned int hcreset : 1;
    unsigned int fls : 1;
    unsigned int frame_list_size : 2;
    unsigned int periodic_schedule_enable : 1;
    unsigned int async_schedule_enable : 1;
    unsigned int reserved : 25;
} usb_ehci_cmd_reg_t;

typedef struct
{
    uint32_t HorizontalLinkPointer;
    uint32_t EndpointCharacteristics;
    uint32_t EndpointCapabilities;
    uint32_t CurrentTDAddress;
    uint32_t CurrentTDWorkingArea;
} usb_ehci_queue_head_t;

typedef struct
{
    uint32_t NextQueueHead : 27;
    uint32_t Reserved : 2;
    uint32_t NextQueueType : 2;
    uint32_t Terminate : 1;
} usb_ehci_horizontal_link_ptr_t;

typedef struct
{
    uint32_t NAKReload : 4;
    uint32_t ControlEndpoint : 1;
    uint32_t MaximumPacketLength : 11;
    uint32_t HeadOfReclamationList : 1;
    uint32_t DataToggleControl : 1;
    uint32_t EndpointSpeed : 2;
    uint32_t EndpointNumber : 4;
    uint32_t Inactivate : 1;
    uint32_t DeviceAddress : 7;
} usb_ehci_endpoint_characteristics_t;

typedef struct
{
    uint32_t HighBandwidthPipeMultiplier : 2;
    uint32_t PortNumber : 7;
    uint32_t HubAddress : 7;
    uint32_t SplitCompletionMask : 8;
    uint32_t InterruptScheduleMask : 8;
} usb_ehci_endpoint_capabilities_t;

#pragma pack (pop)


uint32_t usb_ehci_hba_address = 0;
usb_ehci_hba_t *usb_ehci_hba;
uint32_t usb_ehci_op_regs_address = 0;
usb_ehci_op_regs_t *usb_ehci_op_regs;
unsigned char *usb_ehci_ptr = (unsigned char *)USB_EHCI_ADDRESS;
usb_ehci_t *usb_ehci;
uint8_t usb_ehci_available_ports = 0;

uint32_t periodiclist[1024] __attribute__ ((aligned (0x1000)));

uint32_t get_usb_ehci_hba(void);
void *get_usb_ehci_hba_ptr(void);
usb_ehci_hba_t *get_usb_ehci_hba_data(void);
uint32_t get_usb_ehci_hba_size(void);
void *get_usb_ehci_op_regs_ptr(void);
usb_ehci_op_regs_t *get_usb_ehci_op_regs_data(void);
uint32_t get_usb_ehci_op_regs_size(void);
uint8_t get_usb_ehci_available_ports(void);
bool get_ehci_handshake(uint32_t register_value, uint32_t mask, uint32_t result, unsigned long ms);
bool enable_ehci_async_list(const bool enable);

extern void msleep(unsigned int milliseconds);

int init_usb_ehci(void)
{
	if (init_pci())
	{
		if (pci_count > 0)
		{	
			for(int i=0;i<pci_count;i++) 
			{
				if (pci_device[i].pci.vendor != 0xFFFF)
				{
					if ((pci_device[i].pci.class == 0x0C) && (pci_device[i].pci.subclass == 0x03))
					{
						if (pci_device[i].pci.progif == 0x20)
						{
							if (pci_device[i].pci.bar[0] != 0)
							{
								usb_ehci_hba_address = pci_device[i].pci.bar[0];
								usb_ehci_hba = (usb_ehci_hba_t*)usb_ehci_hba_address;
								uint8_t cap_length = usb_ehci_hba->cap_length;
								if (usb_ehci_hba->version == 0x100)
								{
									usb_ehci_op_regs_address = usb_ehci_hba_address + cap_length;
									usb_ehci_op_regs = (usb_ehci_op_regs_t*)usb_ehci_op_regs_address;
									usb_ehci_ptr = (unsigned char *)USB_EHCI_ADDRESS;
									usb_ehci = (usb_ehci_t*)usb_ehci_ptr;
									strcpy(usb_ehci->signature, "EHCI");
									usb_ehci->version = 1;
									pci_write_word(pci_device[i].bus, pci_device[i].slot, pci_device[i].function, 0x04, 0x0006);
									uint32_t hc_sparams = usb_ehci_hba->sparams;
									uint32_t hc_cparams = usb_ehci_hba->cparams;
									usb_ehci_available_ports = (hc_sparams & 0x0F);
									uint8_t usb_ehci_available_port_info_index = 0xFF;
									
									printf("USB EHCI Found %d Hub Ports.\n", usb_ehci_available_ports);
									
									/*
									uint8_t cap_ptr = ((hc_cparams & 0b1111111100000000) >> 8);
									if (cap_ptr)
									{
										uint32_t cap_addr = cap_ptr;
										while (1)
										{
											uint32_t cap = (usb_ehci_hba_address) + cap_addr;
											uint8_t cid = cap & 0xff;
											if (cid == 0x01)
											{
												if ((cap) & (1<<16))
												{
													uint32_t* cap_ptr_val = (uint32_t*)cap;
													*cap_ptr_val = 0x1000000;
													recheck_cap:
													msleep(2);
													if ((cap) & (1<<16))
													{
														goto recheck_cap;
													}
												}
											}
											uint8_t cxt = ((cap & 0xFF00) >> 8);
											if (cxt == 0)
											{
												break;
											}
											cap_addr += cxt;
										}
									}
									*/
									
									
									uint32_t ext_caps = (hc_cparams & (255 << 8)) >> 8;
									if (ext_caps >= 0x40)
									{
										uint32_t legacy_support = pci_read_long(pci_device[i].bus, pci_device[i].slot, pci_device[i].function, ext_caps);
										if (legacy_support & 0x00010000)
										{
											pci_write_long(pci_device[i].bus, pci_device[i].slot, pci_device[i].function, ext_caps, legacy_support | 0x01000000);
											while(1)
											{
												legacy_support = pci_read_long(pci_device[i].bus, pci_device[i].slot, pci_device[i].function, ext_caps);
												if (~legacy_support & 0x00010000 && legacy_support & 0x01000000)
												{
													break;
												}
											}
										}
									}
									
									uint32_t default_cmd = usb_ehci_op_regs->usb_cmd;
									if (default_cmd & 1)
									{
										usb_ehci_op_regs->usb_cmd &= ~1;
										while (1)
										{
											if ((usb_ehci_op_regs->usb_cmd & 1) == 0)
											{
												break;
											}
										}
									}
									int timeout = 50;
									usb_ehci_op_regs->usb_cmd = 2;
									while (1)
									{
										msleep(1);
										if (--timeout == 0)
										{
											break;
										}
									}
									hc_sparams = usb_ehci_hba->sparams;
									hc_cparams = usb_ehci_hba->cparams;
									usb_ehci_available_ports = (hc_sparams & 0x0F);
									
									/*
									usb_ehci_op_regs->usb_cmd |= 2;
									while (1)
									{
										msleep(1);
										if ((usb_ehci_op_regs->usb_cmd & 2) == 0)
										{
											break;
										}
									}
									*/
									for(int k=0;k<1024;k++)
									{
										periodiclist[i] |= 1;
									}
									
									//if ((hc_cparams & (1<<0)) == 1)
									//{
										usb_ehci_op_regs->ctrl_ds_segment = 0;
									//}
									
									usb_ehci_op_regs->usb_intr = 0;
									usb_ehci_op_regs->usb_sts = 0x3f;
									usb_ehci_op_regs->periodiclistbase = (uint32_t)&periodiclist;
									
									usb_ehci_op_regs->usb_cmd |= (0x40 << 16);
									usb_ehci_op_regs->usb_cmd |= (0 << 2);
									usb_ehci_op_regs->usb_cmd |= 1;
									
									usb_ehci_op_regs->frameindex = 0;
									//usb_ehci_op_regs->asynclistaddr = 0;
									//usb_ehci_op_regs->usb_cmd |= ((8<<16) | (1<<0));
									/*
									if (!enable_ehci_async_list(true))
									{
										return 0;
									}
									*/	
									usb_ehci_op_regs->configflag |= 1;
									//msleep(20);
									
									for(int j=0;j<usb_ehci_available_ports;j++)
									{
										uint8_t usb_ehci_available_port = ((uint8_t)usb_ehci_op_regs->portsc[j] & 0xFF); //(usb_ehci_hba_address + (0x44 - 0x10) + (i*4));
										if ((usb_ehci_available_port != 0xFF) && (usb_ehci_available_port & 3))
										{
											usb_ehci_available_port_info_index = j;
										}
									}
									
									
									for(int j=0;j<usb_ehci_available_ports;j++)
									{
										uint32_t usb_ehci_available_port = usb_ehci_op_regs->portsc[j]; //(usb_ehci_hba_address + (0x44 - 0x10) + (i*4));
										if (usb_ehci_available_port & 3)
										{
											uint32_t usb_ehci_available_port_info = usb_ehci_op_regs->portsc[j];
											if (usb_ehci_available_port != 0xFFFFFFFF) printf("USB EHCI Available Port %d: 0x%X\n", j, usb_ehci_available_port_info);
										}
									}
									
									if (hc_sparams & (1<<4))
									{
										for(int j=0;j<usb_ehci_available_ports;j++)
										{
											usb_ehci_op_regs->portsc[j] |= (1<<12);
										}
									}
									
									//usb_ehci_hba->sparams = 0x200;
									//usb_ehci_hba->cparams = 0x4000;
									
									hc_sparams = usb_ehci_hba->sparams;
									hc_cparams = usb_ehci_hba->cparams;
									//usb_ehci_op_regs->usb_cmd = 1;
									//usb_ehci_op_regs->usb_cmd |= (1 << 0);
									
									usb_ehci_available_ports = (hc_sparams & 0x0F);
									
									msleep(9);
									printf("");
									for(int k=0;k<80000000;k++);
									printf("");
									
									if (usb_ehci_available_port_info_index != 0xFF)
									{
										// OK
									}
									
									
									
									return 1;	
								}
							}							
						}
					}
				}
			}
		}
	}
	return 0;
}

uint32_t get_usb_ehci_hba(void)
{
	return usb_ehci_hba_address;
}

void *get_usb_ehci_hba_ptr(void)
{
	unsigned char *hba_ptr = (unsigned char *)get_usb_ehci_hba();
	return hba_ptr;
}

usb_ehci_hba_t *get_usb_ehci_hba_data(void)
{
	usb_ehci_hba = (usb_ehci_hba_t*)get_usb_ehci_hba();
	return usb_ehci_hba;
}

uint32_t get_usb_ehci_hba_size(void)
{
	return (uint32_t)sizeof(usb_ehci_hba_t);
}

uint32_t get_usb_ehci_op_regs(void)
{
	return usb_ehci_op_regs_address;
}

void *get_usb_ehci_op_regs_ptr(void)
{
	unsigned char *op_regs_ptr = (unsigned char *)get_usb_ehci_op_regs();
	return op_regs_ptr;
}

usb_ehci_op_regs_t *get_usb_ehci_op_regs_data(void)
{
	return (usb_ehci_op_regs_t*)get_usb_ehci_op_regs();
}

uint32_t get_usb_ehci_op_regs_size(void)
{
	return (uint32_t)sizeof(usb_ehci_op_regs_t);
}

uint8_t get_usb_ehci_available_ports(void)
{
	return usb_ehci_available_ports;
}

bool get_ehci_handshake(uint32_t register_value, uint32_t mask, uint32_t result, unsigned long ms)
{
  do 
  {
    if ((register_value & mask) == result)
      return true;    
    msleep(1);
  } while (--ms);  
  return false;
}

bool enable_ehci_async_list(const bool enable)
{
  uint32_t cmd;
  cmd = usb_ehci_op_regs->usb_cmd;
  if (get_ehci_handshake(usb_ehci_op_regs->usb_sts, (1<<15), (cmd & (1<<5)) ? (1<<15) : 0, 100)) 
  {
    if (enable) 
	{
      if (!(cmd & (1<<5)))
	  {
        usb_ehci_op_regs->usb_cmd = cmd | (1<<5);
	  }
      return get_ehci_handshake(usb_ehci_op_regs->usb_sts, (1<<15), (1<<15), 100);
    } else 
	{
      if (cmd & (1<<5))
	  {
        usb_ehci_op_regs->usb_cmd = cmd & ~(1<<5);
	  }
      return get_ehci_handshake(usb_ehci_op_regs->usb_sts, (1<<15), 0, 100);
    }
  }  
  return false;
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

void dump_char(const void *data, size_t size) 
{
    const unsigned char *buffer = (const unsigned char *)data;
    size_t i;
    for (i = 0; i < size; i++) 
	{
		if ((buffer[i] >= 0x20) && (buffer[i] < 0x7F))
		{
			printf("%c", buffer[i]);
		}
    }
}

int main() 
{
	if (init_usb_ehci())
	{
		printf("usb_load: 1\n");
		printf("\n");
		printf("USB EHCI Address: 0x%08X\n", get_usb_ehci_hba());
		printf("\n");
		dump_hex(get_usb_ehci_hba_ptr(), get_usb_ehci_hba_size());
		printf("\n");
		printf("USB EHCI Registers Address: 0x%08X\n", get_usb_ehci_op_regs());
		printf("\n");
		dump_hex(get_usb_ehci_op_regs_ptr(), get_usb_ehci_op_regs_size());
	}
	return 0;
}

