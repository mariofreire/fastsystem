// Fast System USB OHCI
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2024 DSP Interactive.

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "pci.h"

#define USB_OHCI_ADDRESS 0x842A00

#define TRUE 1
#define FALSE 0

#pragma pack (push, 1)

typedef struct
{
	char signature[4];
	uint8_t version;
} usb_ohci_t;

typedef struct
{
    uint32_t revision;
    uint32_t control;
    uint32_t cmd_status;
    uint32_t intr_status;
    uint32_t intr_enable;
    uint32_t intr_disable;
    uint32_t hcca;
    uint32_t period_current_ed;
    uint32_t control_head_ed;
    uint32_t control_current_ed;
    uint32_t bulk_head_ed;
    uint32_t bulk_current_ed;
    uint32_t done_head;
    uint32_t fm_interval;
    uint32_t fm_remaining;
    uint32_t fm_number;
    uint32_t periodic_start;
    uint32_t ls_threshold;
    uint32_t rh_descriptor_a;
    uint32_t rh_descriptor_b;
    uint32_t rh_status;
    uint32_t rh_port_status[15];
} usb_ohci_hba_t;

#pragma pack (pop)


uint32_t usb_ohci_hba_address = 0;
usb_ohci_hba_t *usb_ohci_hba;
unsigned char *usb_ohci_ptr = (unsigned char *)USB_OHCI_ADDRESS;
usb_ohci_t *usb_ohci;


int init_usb_ohci(void)
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
						if (pci_device[i].pci.bar[0] != 0)
						{
							usb_ohci_hba_address = pci_device[i].pci.bar[0];
							usb_ohci_hba = (usb_ohci_hba_t*)usb_ohci_hba_address;
							if (usb_ohci_hba->revision == 0x10)
							{
								if (usb_ohci_hba->control == 0x200)
								{
									usb_ohci_ptr = (unsigned char *)USB_OHCI_ADDRESS;
									usb_ohci = (usb_ohci_t*)usb_ohci_ptr;
									strcpy(usb_ohci->signature, "OHCI");
									usb_ohci->version = 1;
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

uint32_t get_usb_ohci_hba(void)
{
	return usb_ohci_hba_address;
}

void *get_usb_ohci_hba_ptr(void)
{
	unsigned char *hba_ptr = (unsigned char *)get_usb_ohci_hba();
	return hba_ptr;
}

usb_ohci_hba_t *get_usb_ohci_hba_data(void)
{
	usb_ohci_hba = (usb_ohci_hba_t*)get_usb_ohci_hba();
	return usb_ohci_hba;
}

uint32_t get_usb_ohci_hba_size(void)
{
	return (uint32_t)sizeof(usb_ohci_hba_t);
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
	if (init_usb_ohci())
	{
		printf("usb_load: 1\n");
		printf("\n");
		printf("USB OHCI Address: 0x%08X\n", get_usb_ohci_hba());
		printf("\n");
		dump_hex(get_usb_ohci_hba_ptr(), get_usb_ohci_hba_size());
	}
	return 0;
}

