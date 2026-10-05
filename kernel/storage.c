// Fast System Kernel - Storage Controller
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2026 DSP Interactive.

#include <stdarg.h>
#include "fskrnl.h"
#include "enum.h"

unsigned long storage_read(int id, void *buffer, int sector, int count)
{
	unsigned long r = 0;
	switch (storage_drive_controller)
	{
		case STORAGE_CONTROLLER_IDE:
		{
			r = ata_read(id, buffer, sector, count);
		}
		break;
		case STORAGE_CONTROLLER_AHCI:
		{
			r = ahci_read(id, buffer, sector, count);
		}
		break;
		case STORAGE_CONTROLLER_EHCI:
		{
			r = ehci_read(id, buffer, sector, count);
		}
		break;
		case STORAGE_CONTROLLER_NVME:
		{
			r = nvme_read(id, buffer, sector, count);
		}
		break;
		case 0:
		{
			if (usermode == 0)
			{
				panic((unsigned long)buffer);
			}
		}
		break;
		default:
		{
			if (usermode == 0)
			{
				panic((unsigned long)buffer);
			}
		}
		break;
	};
	return r;
}

unsigned long storage_write(int id, const void *buffer, int sector, int count)
{
	unsigned long r = 0;
	switch (storage_drive_controller)
	{
		case STORAGE_CONTROLLER_IDE:
		{
			r = ata_write(id, buffer, sector, count);
		}
		break;
		case STORAGE_CONTROLLER_AHCI:
		{
			r = ahci_write(id, buffer, sector, count);
		}
		break;
		case STORAGE_CONTROLLER_EHCI:
		{
			r = ehci_write(id, buffer, sector, count);
		}
		break;
		case STORAGE_CONTROLLER_NVME:
		{
			r = nvme_write(id, buffer, sector, count);
		}
		break;
		case 0:
		{
			if (usermode == 0)
			{
				panic((unsigned long)buffer);
			}
		}
		break;
		default:
		{
			if (usermode == 0)
			{
				panic((unsigned long)buffer);
			}
		}
		break;
	};
	return r;
}

#ifdef USE_DAP

unsigned char disk_read(unsigned char drive, dap_t *dapack)
{
	unsigned char result;
	unsigned char carry;
	registers32_t regs;
	regs.eax = 0x4200;
	regs.edx = (unsigned char)(drive & 0xFF);
	regs.ds = FP_SEG((unsigned long)dapack);
	regs.esi = FP_OFF((unsigned long)dapack);
	int386(0x13, &regs, &regs);
	carry = FLAGBIT(regs.eflags, 0);
	result = 1-carry;
	return result;
}

unsigned char disk_write(unsigned char drive, dap_t *dapack)
{
	unsigned char result;
	unsigned char carry;
	registers32_t regs;
	regs.eax = 0x4300;
	regs.edx = (unsigned char)(drive & 0xFF);
	regs.ds = FP_SEG((unsigned long)dapack);
	regs.esi = FP_OFF((unsigned long)dapack);
	int386(0x13, &regs, &regs);
	carry = FLAGBIT(regs.eflags, 0);
	result = 1-carry;
	return result;
}

void loaddap(void)
{
	dap = (dap_t*)disk_address_packet;
	dap->size = sizeof(dap_t);
	dap->unused = 0;
	dap->sector_count = 1;
	dap->buffer_ptr = 0;
	dap->lba_start_1 = 0;
	dap->lba_start_2 = 0;
}

unsigned char bios_sector_read(int id, void *buffer, unsigned long sector)
{
	unsigned char result;
	unsigned char drive = (0x80 + id);
	unsigned char *old_buffer = (unsigned char*)malloc(SECTORSIZE);
	unsigned char *tmp_buffer = (unsigned char*)0x6400;
	memcpy(old_buffer, tmp_buffer, SECTORSIZE);
	memset(tmp_buffer, 0, SECTORSIZE);
	dap = (dap_t*)disk_address_packet;
	dap->lba_start_1 = sector;
	dap->lba_start_2 = 0;
	dap->sector_count = 1;
	dap->buffer_ptr = (unsigned long)tmp_buffer;	
	result = disk_read(drive, dap);	
	memcpy(buffer, tmp_buffer, SECTORSIZE);
	memcpy(tmp_buffer, old_buffer, SECTORSIZE);
	free(old_buffer);
	return result;
}

unsigned char bios_sector_write(int id, const void *buffer, unsigned long sector)
{
	unsigned char result;
	unsigned char drive = (0x80 + id);
	dap = (dap_t*)disk_address_packet;
	dap->lba_start_1 = sector;
	dap->lba_start_2 = 0;
	dap->sector_count = 1;
	dap->buffer_ptr = (unsigned long)buffer;	
	result = disk_write(drive, dap);	
	return result;
}

unsigned char sector_read(int id, void *buffer, unsigned long sector, int count)
{
	unsigned char result;
	unsigned char drive = (0x80 + id);
	unsigned long offset = (unsigned long)buffer;
	if (offset < 65536)
	{
		dap = (dap_t*)disk_address_packet;
		dap->lba_start_1 = sector;
		dap->lba_start_2 = 0;
		dap->sector_count = count;
		dap->buffer_ptr = (unsigned long)buffer;
		if (storage_drive_controller != STORAGE_CONTROLLER_IDE) return storage_read(id,buffer,sector,count);
		result = disk_read(drive, dap);	
	}
	else
	{
		result = 0;
	}
	return result;
}

unsigned char sector_write(int id, const void *buffer, unsigned long sector, int count)
{
	unsigned char result;
	unsigned char drive = (0x80 + id);
	unsigned long offset = (unsigned long)buffer;
	if (offset < 65536)
	{
		dap = (dap_t*)disk_address_packet;
		dap->lba_start_1 = sector;
		dap->lba_start_2 = 0;
		dap->sector_count = count;
		dap->buffer_ptr = (unsigned long)buffer;
		if (storage_drive_controller != STORAGE_CONTROLLER_IDE) return storage_write(id,buffer,sector,count);
		result = disk_write(drive, dap);	
	}
	else
	{
		result = 0;
	}
	return result;
}

#ifdef DAP_ONLY_TRY_WHEN_ERROR

unsigned char read_sector(unsigned long sector, unsigned char *buffer)
{
	unsigned char result;
	unsigned long offset = (unsigned long)buffer;
	unsigned short max_dap_sector = (65536-SECTORSIZE);
	int id = 0;
	int i = 30;
	
	if (storage_drive_controller != STORAGE_CONTROLLER_IDE) return storage_read(id,buffer,sector,1);

	result = storage_read(id, buffer, sector, 1);
	if (result == 0)
	{
		result = storage_read(id, buffer, sector, 1);
		while ((result == 0) && (i > 0))
		{
			result = storage_read(id, buffer, sector, 1);
			if (i > 0)
			{
				i--;
			}
			else
			{
				break;
			}
		}
	}
	
	if (result == 0)
	{
		i = 30;
		result = sector_read(id, buffer, sector, 1);
		if (result == 0)
		{
			result = sector_read(id, buffer, sector, 1);
			while (i > 0)
			{
				result = sector_read(id, buffer, sector, 1);
				if (result != 0)
				{
					i = 0;
				}
				if (i > 0)
				{
					i--;
				}
				else
				{
					break;
				}
			}
		}
	}
	
	if (mbr_loaded == 1)
	{
		remap_mbr();
	}
	
	return result;
}

unsigned char write_sector(unsigned long sector, const unsigned char *buffer)
{
	unsigned char result;
	unsigned long offset = (unsigned long)buffer;
	unsigned short max_dap_sector = (65536-SECTORSIZE);
	int id = 0;
	int i = 30;
	
	if (storage_drive_controller != STORAGE_CONTROLLER_IDE) return storage_write(id,buffer,sector,1);

	result = storage_write(id, buffer, sector, 1);
	if (result == 0)
	{
		result = storage_write(id, buffer, sector, 1);
		while ((result == 0) && (i > 0))
		{
			result = storage_write(id, buffer, sector, 1);
			if (i > 0)
			{
				i--;
			}
			else
			{
				break;
			}
		}
	}
	
	if (result == 0)
	{
		i = 30;
		result = sector_write(id, buffer, sector, 1);
		if (result == 0)
		{
			result = sector_write(id, buffer, sector, 1);
			while (i > 0)
			{
				result = sector_write(id, buffer, sector, 1);
				if (result != 0)
				{
					i = 0;
				}
				if (i > 0)
				{
					i--;
				}
				else
				{
					break;
				}
			}
		}
	}
	
	if (mbr_loaded == 1)
	{
		remap_mbr();
	}
	
	return result;
}

#else

unsigned char read_sector(unsigned long sector, unsigned char *buffer)
{
	unsigned char result;
	unsigned long offset = (unsigned long)buffer;
	unsigned short max_dap_sector = (65536-SECTORSIZE);
	int id = 0;
	int i = 30;
	
	if (storage_drive_controller != STORAGE_CONTROLLER_IDE) return storage_read(id,buffer,sector,1);

	if ((offset < 65536) && (sector < max_dap_sector))
	{
		result = sector_read(id, buffer, sector, 1);
		if (result == 0)
		{
			result = sector_read(id, buffer, sector, 1);
			while (i > 0)
			{
				result = sector_read(id, buffer, sector, 1);
				if (result != 0)
				{
					i = 0;
				}
				if (i > 0)
				{
					i--;
				}
				else
				{
					break;
				}
			}
		}
	}
	else
	{
		result = storage_read(id, buffer, sector, 1);
		if (result == 0)
		{
			result = storage_read(id, buffer, sector, 1);
			while (i > 0)
			{
				result = storage_read(id, buffer, sector, 1);
				if (result != 0)
				{
					i = 0;
				}
				if (i > 0)
				{
					i--;
				}
				else
				{
					break;
				}
			}
		}
	}
	
	if (mbr_loaded == 1)
	{
		remap_mbr();
	}
	
	return result;
}

unsigned char write_sector(unsigned long sector, const unsigned char *buffer)
{
	unsigned char result;
	unsigned long offset = (unsigned long)buffer;
	unsigned short max_dap_sector = (65536-SECTORSIZE);
	int id = 0;
	int i = 30;
	
	if (storage_drive_controller != STORAGE_CONTROLLER_IDE) return storage_write(id,buffer,sector,1);

	if ((offset < 65536) && (sector < max_dap_sector))
	{
		result = sector_write(id, buffer, sector, 1);
		if (result == 0)
		{
			result = sector_write(id, buffer, sector, 1);
			while (i > 0)
			{
				result = sector_write(id, buffer, sector, 1);
				if (result != 0)
				{
					i = 0;
				}
				if (i > 0)
				{
					i--;
				}
				else
				{
					break;
				}
			}
		}
	}
	else
	{
		result = storage_write(id, buffer, sector, 1);
		if (result == 0)
		{
			result = storage_write(id, buffer, sector, 1);
			while (i > 0)
			{
				result = storage_write(id, buffer, sector, 1);
				if (result != 0)
				{
					i = 0;
				}
				if (i > 0)
				{
					i--;
				}
				else
				{
					break;
				}
			}
		}
	}
	
	if (mbr_loaded == 1)
	{
		remap_mbr();
	}
	
	return result;
}

#endif

#else

unsigned char read_sector(unsigned long sector, unsigned char *buffer)
{
	unsigned char result;
	unsigned long offset = (unsigned long)buffer;
	int id = 0;
	int i = 30;
	
	if (storage_drive_controller != STORAGE_CONTROLLER_IDE) return storage_read(id,buffer,sector,1);

	result = storage_read(id, buffer, sector, 1);
	if (result == 0)
	{
		result = storage_read(id, buffer, sector, 1);
		while ((result == 0) && (i > 0))
		{
			result = storage_read(id, buffer, sector, 1);
			if (i > 0)
			{
				i--;
			}
			else
			{
				break;
			}
		}
	}
	
	if (mbr_loaded == 1)
	{
		remap_mbr();
	}
	
	return result;
}

unsigned char write_sector(unsigned long sector, const unsigned char *buffer)
{
	unsigned char result;
	unsigned long offset = (unsigned long)buffer;
	int id = 0;
	int i = 30;
	
	if (storage_drive_controller != STORAGE_CONTROLLER_IDE) return storage_write(id,buffer,sector,1);

	result = storage_write(id, buffer, sector, 1);
	if (result == 0)
	{
		result = storage_write(id, buffer, sector, 1);
		while ((result == 0) && (i > 0))
		{
			result = storage_write(id, buffer, sector, 1);
			if (i > 0)
			{
				i--;
			}
			else
			{
				break;
			}
		}
	}
	
	if (mbr_loaded == 1)
	{
		remap_mbr();
	}
	
	return result;
}

#endif
