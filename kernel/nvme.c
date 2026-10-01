// Fast System Kernel - Non-Volatile Memory Express
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2026 DSP Interactive.

#include <stdarg.h>
#include "fskrnl.h"
#include "enum.h"

unsigned char *nvme_ptr  = (unsigned char *)NVME_ADDRESS;

unsigned long nvme_bar_address;

nvme_controller_t *nvme_controller;

nvme_t *nvme;

int nvme_initialized = 0;

int init_nvme(void)
{
	return 0;
}

int init_nvme_ports()
{
	return 0;
}

int get_nvme_ident(int id, char *buffer)
{
	return 0;
}

int nvme_read(int id, void *buffer, unsigned long sector, unsigned short count)
{
	return 0;
}

int nvme_write(int id, const void *buffer, unsigned long sector, unsigned short count)
{
	return 0;
}

void detectnvme(void)
{	
	int i;
	char buffer[256];
	if (nvme_initialized == 1)
	{
		if (nvme_bar_address != 0)
		{
			if (nvme->count == 0) return;
			for(i=0;i<nvme->count;i++)
			{
				if (get_nvme_ident(i, buffer))
				{
					if (strlen(buffer) == 0)
					{
						strcpy(buffer, "Virtual HD");
					}
					printk("NVME M.2 Drive %d: %s\n", i, buffer);
				}
			}
		}
	}
}
