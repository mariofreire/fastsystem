// Fast System List PCI
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2024 DSP Interactive.

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "pci.h"

int main(int argc, char *argv[]) 
{
	const char *pci_name;
	init_pci();
	if (strcmp(sys_pci->signature, "PCI") == 0)
	{
		if (sys_pci->version == 1)
		{
			pci_count = sys_pci->count;
			if (pci_count > 0)
			{			
				if (argc > 1)
				{
					int i = atol(argv[1]);
					if (i < pci_count)
					{
						// unsigned char _pci_bus = pci_device[i].bus;
						// unsigned char _pci_dev = pci_device[i].slot;
						// unsigned char _pci_fnc = pci_device[i].function;
						// pci_scan_device(_pci_bus, _pci_dev, _pci_fnc, i);
						if (pci_device[i].pci.vendor != 0xFFFF)
						{
							printf("\n");	
							pci_name = get_pci_class_name(pci_device[i].pci.class, pci_device[i].pci.subclass);
							printf("PCI Device: 0x%X:0x%X, bus: %d, device: %d, function: %d\n    Name: %s\n", 
							pci_device[i].pci.vendor, pci_device[i].pci.device, pci_device[i].bus, pci_device[i].slot, pci_device[i].function, pci_name);
							if ((pci_device[i].pci.interrupt_line != 0) && (pci_device[i].pci.interrupt_pin != 0))
							{
								char irq_pin = ('A' + (pci_device[i].pci.interrupt_pin - 1));
								printf("    IRQ: %d, Pin: %c\n", pci_device[i].pci.interrupt_line, irq_pin);
							}
							for(int j=0;j<6;j++)
							{
								 if (pci_device[i].pci.bar[j] != 0)
								 {
									printf("    BAR%d: 0x%X\n", j, pci_device[i].pci.bar[j]);
									if ((pci_device[i].pci.class == 1) && (pci_device[i].pci.subclass == 6) && (pci_device[i].pci.vendor != 0xFFFF))
									{
										if (j == 5)
										{
											//pci_device[i].pic.bar[j]
											//pci_write_byte
										}
									}
								 }
							}
							printf("\n");		
						}	
						else
						{
							printf("Cannot read pci port.\n");
						}
					}
					else
					{
						printf("Cannot read pci port.\n");
					}
				}
				else
				{
					if (pci_count)
					{
						printf("\n");	
						for(int i=0;i<pci_count;i++) 
						{
							if (pci_device[i].pci.vendor != 0xFFFF)
							{
								pci_name = get_pci_class_name(pci_device[i].pci.class, pci_device[i].pci.subclass);
								printf("PCI Device: 0x%X:0x%X, bus: %d, device: %d, function: %d\n    Name: %s\n", 
								pci_device[i].pci.vendor, pci_device[i].pci.device, pci_device[i].bus, pci_device[i].slot, pci_device[i].function, pci_name);
								if ((pci_device[i].pci.interrupt_line != 0) && (pci_device[i].pci.interrupt_pin != 0))
								{
									char irq_pin = ('A' + (pci_device[i].pci.interrupt_pin - 1));
									printf("    IRQ: %d, Pin: %c\n", pci_device[i].pci.interrupt_line, irq_pin);
								}
								for(int j=0;j<6;j++)
								{
									 if (pci_device[i].pci.bar[j] != 0)
									 {
										printf("    BAR%d: 0x%X\n", j, pci_device[i].pci.bar[j]);
										if ((pci_device[i].pci.class == 1) && (pci_device[i].pci.subclass == 6) && (pci_device[i].pci.vendor != 0xFFFF))
										{
											if (j == 5)
											{
												//pci_device[i].pic.bar[j]
												//pci_write_byte
											}
										}
									 }
								}
								printf("\n");		
							}
						}
					}						
				}
			}
			else
			{
				printf("No PCI device found.\n");
			}
		}
		else
		{
			printf("Invalid PCI device version.\n");
		}
	}
	else
	{
		printf("No PCI device found.\n");
	}
	return 0;
}

