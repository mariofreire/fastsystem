// Fast System AHCI HBA
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2024 DSP Interactive.

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "pci.h"

uint32_t ahci_hba_address = 0;

int init_ahci(void)
{
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
						for(int j=0;j<6;j++)
						{
							 if (pci_device[i].pci.bar[j] != 0)
							 {								
								if ((pci_device[i].pci.class == 1) && (pci_device[i].pci.subclass == 6) && (pci_device[i].pci.vendor != 0xFFFF))
								{
									if (j == 5)
									{
										ahci_hba_address = pci_device[i].pci.bar[j];
										return 1;
									}
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

uint32_t get_ahci_hba(void)
{
	return ahci_hba_address;
}

int main() 
{
	if (init_ahci())
	{
		printf("AHCI HBA: 0x%X\n", get_ahci_hba());
	}
	return 0;
}

