// Fast System PCI
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2024 DSP Interactive.

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "pci.h"

uint8_t *system_pci;
system_pci_t *sys_pci;
pci_device_t *pci_device;
int pci_count = 0;

unsigned long pci_config_address(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset)
{
	unsigned long r = 0LL;
	__asm__ volatile ( "int $0x80" : "=a"(r) : "a" (400), "b" (bus), "c" (slot), "d" (func), "S" (offset) );
	return r;
}

unsigned char pci_read_byte(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset)
{
	unsigned char r = 0LL;
	__asm__ volatile ( "int $0x80" : "=a"(r) : "a" (401), "b" (bus), "c" (slot), "d" (func), "S" (offset) );
	return r;
}

unsigned short pci_read_word(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset)
{
	unsigned short r = 0LL;
	__asm__ volatile ( "int $0x80" : "=a"(r) : "a" (402), "b" (bus), "c" (slot), "d" (func), "S" (offset) );
	return r;
}

unsigned long pci_read_long(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset)
{
	unsigned long r = 0LL;
	__asm__ volatile ( "int $0x80" : "=a"(r) : "a" (403), "b" (bus), "c" (slot), "d" (func), "S" (offset) );
	return r;
}

void pci_write_byte(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset, unsigned char value)
{
	__asm__ volatile ( "int $0x80" :: "a" (404), "b" (bus), "c" (slot), "d" (func), "S" (offset), "D" (value) );
}

void pci_write_word(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset, unsigned short value)
{
	__asm__ volatile ( "int $0x80" :: "a" (405), "b" (bus), "c" (slot), "d" (func), "S" (offset), "D" (value) );
}

void pci_write_long(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset, unsigned long value)
{
	__asm__ volatile ( "int $0x80" :: "a" (406), "b" (bus), "c" (slot), "d" (func), "S" (offset), "D" (value) );
}

const char *get_pci_class_name_string(uint8_t class, uint8_t subclass) 
{
	if ((class == 0x00) && (subclass == 0x00))
	{
	    return "Non-VGA-Compatible Unclassified Device";
	}
	else if ((class == 0x00) && (subclass == 0x01))
	{
	    return "VGA-Compatible Unclassified Device";
	}
	else if ((class == 0x01) && (subclass == 0x00))
	{
	    return "SCSI Bus Controller";
	}
	else if ((class == 0x01) && (subclass == 0x01))
	{
	    return "IDE Controller";
	}
	else if ((class == 0x01) && (subclass == 0x02))
	{
	    return "Floppy Disk Controller";
	}
	else if ((class == 0x01) && (subclass == 0x03))
	{
	    return "IPI Bus Controller";
	}
	else if ((class == 0x01) && (subclass == 0x04))
	{
	    return "RAID Controller";
	}
	else if ((class == 0x01) && (subclass == 0x05))
	{
	    return "ATA Controller";
	}
	else if ((class == 0x01) && (subclass == 0x06))
	{
	    return "Serial ATA Controller";
	}
	else if ((class == 0x01) && (subclass == 0x07))
	{
	    return "Serial Attached SCSI Controller";
	}
	else if ((class == 0x01) && (subclass == 0x08))
	{
	    return "Non-Volatile Memory Controller";
	}
	else if ((class == 0x01) && (subclass == 0x80))
	{
	    return "Other Mass Storage Controller";
	}
	else if ((class == 0x02) && (subclass == 0x00))
	{
	    return "Ethernet Controller";
	}
	else if ((class == 0x02) && (subclass == 0x01))
	{
	    return "Token Ring Controller";
	}
	else if ((class == 0x02) && (subclass == 0x02))
	{
	    return "FDDI Controller";
	}
	else if ((class == 0x02) && (subclass == 0x03))
	{
	    return "ATM Controller";
	}
	else if ((class == 0x02) && (subclass == 0x04))
	{
	    return "ISDN Controller";
	}
	else if ((class == 0x02) && (subclass == 0x05))
	{
	    return "WorldFip Controller";
	}
	else if ((class == 0x02) && (subclass == 0x06))
	{
	    return "PICMG 2.14 Multi Computing Controller";
	}
	else if ((class == 0x02) && (subclass == 0x07))
	{
	    return "Infiniband Controller";
	}
	else if ((class == 0x02) && (subclass == 0x08))
	{
	    return "Fabric Controller";
	}
	else if ((class == 0x02) && (subclass == 0x80))
	{
	    return "Other Network Controller";
	}
	else if ((class == 0x03) && (subclass == 0x00))
	{
	    return "VGA Compatible Controller";
	}
	else if ((class == 0x03) && (subclass == 0x01))
	{
	    return "XGA Controller";
	}
	else if ((class == 0x03) && (subclass == 0x02))
	{
	    return "3D Controller (Not VGA-Compatible)";
	}
	else if ((class == 0x03) && (subclass == 0x80))
	{
	    return "Other Display Controller";
	}
	else if ((class == 0x04) && (subclass == 0x00))
	{
	    return "Multimedia Video Controller";
	}
	else if ((class == 0x04) && (subclass == 0x01))
	{
	    return "Multimedia Audio Controller";
	}
	else if ((class == 0x04) && (subclass == 0x02))
	{
	    return "Computer Telephony Device";
	}
	else if ((class == 0x04) && (subclass == 0x03))
	{
	    return "Audio Device";
	}
	else if ((class == 0x04) && (subclass == 0x80))
	{
	    return "Other Multimedia Controller";
	}
	else if ((class == 0x05) && (subclass == 0x00))
	{
	    return "RAM Controller";
	}
	else if ((class == 0x05) && (subclass == 0x01))
	{
	    return "Flash Controller";
	}
	else if ((class == 0x05) && (subclass == 0x80))
	{
	    return "Other Memory Controller";
	}
	else if ((class == 0x06) && (subclass == 0x00))
	{
	    return "Host Bridge";
	}
	else if ((class == 0x06) && (subclass == 0x01))
	{
	    return "ISA Bridge";
	}
	else if ((class == 0x06) && (subclass == 0x02))
	{
	    return "EISA Bridge";
	}
	else if ((class == 0x06) && (subclass == 0x03))
	{
	    return "MCA Bridge";
	}
	else if ((class == 0x06) && (subclass == 0x04))
	{
	    return "PCI-to-PCI Brige";
	}
	else if ((class == 0x06) && (subclass == 0x05))
	{
	    return "PCMCIA Bridge";
	}
	else if ((class == 0x06) && (subclass == 0x06))
	{
	    return "NuBus Bridge";
	}
	else if ((class == 0x06) && (subclass == 0x07))
	{
	    return "CardBus Bridge";
	}
	else if ((class == 0x06) && (subclass == 0x08))
	{
	    return "RACEway Bridge";
	}
	else if ((class == 0x06) && (subclass == 0x09))
	{
	    return "PCI-to-PCI Bridge";
	}
	else if ((class == 0x06) && (subclass == 0x0A))
	{
	    return "Infiniband-to-PCI Host Bridge";
	}
	else if ((class == 0x06) && (subclass == 0x80))
	{
	    return "Other Bridge";
	}
	else if ((class == 0x07) && (subclass == 0x00))
	{
	    return "Serial Controller";
	}
	else if ((class == 0x07) && (subclass == 0x01))
	{
	    return "Parallel Controller";
	}
	else if ((class == 0x07) && (subclass == 0x02))
	{
	    return "Multiport Serial Controller";
	}
	else if ((class == 0x07) && (subclass == 0x03))
	{
	    return "Modem";
	}
	else if ((class == 0x07) && (subclass == 0x04))
	{
	    return "IEEE 488.1/2 (GPIB) Controller";
	}
	else if ((class == 0x07) && (subclass == 0x05))
	{
	    return "Smart Card Controller";
	}
	else if ((class == 0x07) && (subclass == 0x80))
	{
	    return "Other Simple Communication Controller";
	}
	else if ((class == 0x08) && (subclass == 0x00))
	{
	    return "PIC";
	}
	else if ((class == 0x08) && (subclass == 0x01))
	{
	    return "DMA Controller";
	}
	else if ((class == 0x08) && (subclass == 0x02))
	{
	    return "Timer";
	}
	else if ((class == 0x08) && (subclass == 0x03))
	{
	    return "RTC Controller";
	}
	else if ((class == 0x08) && (subclass == 0x04))
	{
	    return "PCI Hot-Plug Controller";
	}
	else if ((class == 0x08) && (subclass == 0x05))
	{
	    return "SD Host Controller";
	}
	else if ((class == 0x08) && (subclass == 0x07))
	{
	    return "IOMMU";
	}
	else if ((class == 0x08) && (subclass == 0x80))
	{
	    return "Other Base System Peripheral";
	}
	else if ((class == 0x09) && (subclass == 0x00))
	{
	    return "Keyboard Controller";
	}
	else if ((class == 0x09) && (subclass == 0x01))
	{
	    return "Digitizer Pen";
	}
	else if ((class == 0x09) && (subclass == 0x02))
	{
	    return "Mouse Controller";
	}
	else if ((class == 0x09) && (subclass == 0x03))
	{
	    return "Scanner Controller";
	}
	else if ((class == 0x09) && (subclass == 0x04))
	{
	    return "Gameport Controller";
	}
	else if ((class == 0x09) && (subclass == 0x80))
	{
	    return "Other Input Device Controller";
	}
	else if ((class == 0x0A) && (subclass == 0x00))
	{
	    return "Generic Docking Station";
	}
	else if ((class == 0x0A) && (subclass == 0x80))
	{
	    return "Other Docking Station";
	}
	else if ((class == 0x0B) && (subclass == 0x00))
	{
	    return "386 Processor";
	}
	else if ((class == 0x0B) && (subclass == 0x01))
	{
	    return "486 Processor";
	}
	else if ((class == 0x0B) && (subclass == 0x02))
	{
	    return "Pentium Processor";
	}
	else if ((class == 0x0B) && (subclass == 0x03))
	{
	    return "Pentioum Pro Processor";
	}
	else if ((class == 0x0B) && (subclass == 0x10))
	{
	    return "Alpha Processor";
	}
	else if ((class == 0x0B) && (subclass == 0x20))
	{
	    return "PowerPC Processor";
	}
	else if ((class == 0x0B) && (subclass == 0x30))
	{
	    return "MIPS Processor";
	}
	else if ((class == 0x0B) && (subclass == 0x40))
	{
	    return "Co-Processor";
	}
	else if ((class == 0x0B) && (subclass == 0x80))
	{
	    return "Other Processor";
	}
	else if ((class == 0x0C) && (subclass == 0x00))
	{
	    return "FireWire (IEEE 1394) Controller";
	}
	else if ((class == 0x0C) && (subclass == 0x01))
	{
	    return "ACCESS Bus Controller";
	}
	else if ((class == 0x0C) && (subclass == 0x02))
	{
	    return "SSA";
	}
	else if ((class == 0x0C) && (subclass == 0x03))
	{
	    return "USB Controller";
	}
	else if ((class == 0x0C) && (subclass == 0x04))
	{
	    return "Fibre Channel";
	}
	else if ((class == 0x0C) && (subclass == 0x05))
	{
	    return "SMBus Controller";
	}
	else if ((class == 0x0C) && (subclass == 0x06))
	{
	    return "InfiniBand Controller";
	}
	else if ((class == 0x0C) && (subclass == 0x07))
	{
	    return "IPMI Interface";
	}
	else if ((class == 0x0C) && (subclass == 0x08))
	{
	    return "SERCOS Interface (IEC 61491)";
	}
	else if ((class == 0x0C) && (subclass == 0x09))
	{
	    return "CANbus Controller";
	}
	else if ((class == 0x0C) && (subclass == 0x80))
	{
	    return "Other Serial Bus Controller";
	}
	else if ((class == 0x0D) && (subclass == 0x00))
	{
	    return "iRDA Compatible Controller";
	}
	else if ((class == 0x0D) && (subclass == 0x00))
	{
	    return "Consumer IR Controller";
	}
	else if ((class == 0x0D) && (subclass == 0x00))
	{
	    return "RF Controller";
	}
	else if ((class == 0x0D) && (subclass == 0x00))
	{
	    return "Bluetooth Controller";
	}
	else if ((class == 0x0D) && (subclass == 0x00))
	{
	    return "Broadband Controller";
	}
	else if ((class == 0x0D) && (subclass == 0x00))
	{
	    return "Ethernet Controller (802.1a)";
	}
	else if ((class == 0x0D) && (subclass == 0x00))
	{
	    return "Ethernet Controller (802.1b)";
	}
	else if ((class == 0x0D) && (subclass == 0x00))
	{
	    return "Other Wireless Controller";
	}
	else if ((class == 0x0E) && (subclass == 0x00))
	{
	    return "I20";
	}
	else if ((class == 0x0F) && (subclass == 0x01))
	{
	    return "Satellite TV Controller";
	}
	else if ((class == 0x0F) && (subclass == 0x02))
	{
	    return "Satellite Audio Controller";
	}
	else if ((class == 0x0F) && (subclass == 0x03))
	{
	    return "Satellite Voice Controller";
	}
	else if ((class == 0x0F) && (subclass == 0x04))
	{
	    return "Satellite Data Controller";
	}
	else if ((class == 0x10) && (subclass == 0x00))
	{
	    return "Network and Computing Encryption/Decryption";
	}
	else if ((class == 0x10) && (subclass == 0x10))
	{
	    return "Entertainment Encryption/Decryption";
	}
	else if ((class == 0x10) && (subclass == 0x80))
	{
	    return "Other Encryption Controller";
	}
	else if ((class == 0x11) && (subclass == 0x00))
	{
	    return "DPIO Modules";
	}
	else if ((class == 0x11) && (subclass == 0x01))
	{
	    return "Performance Counters";
	}
	else if ((class == 0x11) && (subclass == 0x10))
	{
	    return "Communication Synchronizer";
	}
	else if ((class == 0x11) && (subclass == 0x20))
	{
	    return "Signal Processing Management";
	}
	else if ((class == 0x11) && (subclass == 0x80))
	{
	    return "Other Signal Processing Controller";
	}
	return "Unknown PCI Device";
}

const char *get_pci_class_name(unsigned char class, unsigned char subclass) 
{
	const char *unknown_class_name = "Unknown PCI Device";	
	switch(class) 
	{
		case 0x12:
		{
			return "Processing Accelerator";
		}
		break;
		case 0x13:
		{
			return "Non-Essential Instrumentation";
		}
		break;
		case 0x40:
		{
			return "Co-Processor";
		}
		break;
		case 0xFF:
		{
			return "Vendor Specific";
		}
		break;
		default:
		{			
			return get_pci_class_name_string(class, subclass);
		}
		break;
	}
	return unknown_class_name;
}

int init_pci(void)
{
	system_pci = (uint8_t*)SYSTEM_PCI_ADDRESS;
	sys_pci = (system_pci_t*)system_pci;
	pci_device = (pci_device_t*)&sys_pci->device[0];
	if (strcmp(sys_pci->signature, "PCI") == 0)
	{
		if (sys_pci->version == 1)
		{
			pci_count = sys_pci->count;
			return 1;
		}
		else
		{
			return 0;
		}
	}
	return 0;	
}
