// Fast System Memory
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2024 DSP Interactive.

#include <stdarg.h>
#include "fskrnl.h"
#include "enum.h"


void detect_memory_map()
{
	memory_map_info_t *mmap_info_smap = (memory_map_info_t*)0x5A00;
	
	size_t m_sm_n = sizeof(memory_map_info_t);	
	unsigned char *m_sm_dest = (unsigned char*)mmap_info_smap;
	while (m_sm_n--)
	{
		*m_sm_dest++ = (unsigned char)0x00;
	}
	
	mmap_info_smap->address_ptr = 0x1000;
	mmap_info_smap->size = 0x2000;
	
	memory_map_entry_t* smap = (memory_map_entry_t*)mmap_info_smap->address_ptr;
	const int smap_size = mmap_info_smap->size;
	int max_entries = smap_size / sizeof(memory_map_entry_t);
	int entry_count = 0;
	mmap_info_smap->max_entries = max_entries;
	
	int signature;
	int bytes;
	int entries = 0;
	unsigned long cnt_id = 0;
	do 
	{
		registers32_t r_in,r_out;
		memset(&r_in, 0, sizeof(registers32_t));
		memset(&r_out, 0, sizeof(registers32_t));
		r_in.eax = 0xE820;
		r_in.ebx = cnt_id;
		r_in.ecx = 24;
		r_in.edx = 0x534D4150;
		r_in.edi = (unsigned long)smap;
		r_in.es = 0;
		int386(0x15, &r_in, &r_out);
		signature = r_out.eax;
		cnt_id = r_out.ebx;
		bytes = r_out.ecx;		
		if (signature != 0x534D4150) 
		{
			entry_count = -1;
			break;
		}
		if (bytes > 20 && (smap->acpi & 0x0001) == 0)
		{
			// ignore
		}
		else {
			smap++;
			entries++;
			entry_count = entries;
		}
	} 
	while (cnt_id != 0 && entries < max_entries);
	
	mmap_info_smap->entry_count = entry_count;
}

void copy_function_to_base(void *function_base, unsigned long function_size, unsigned long new_base)
{
	unsigned long sz = function_size-32;
	unsigned char  *code_base_16 = (unsigned char *)new_base;
	unsigned char  *code_base_32 = (unsigned char *)function_base;
	memset(code_base_16, 0, sz);
	memcpy(code_base_16, code_base_32, sz);
}

void loadmmap()
{
	detect_memory_map();
	memory_map_info = (memory_map_info_t*)system_memory_map_info;
	memory_map = (memory_map_t*)system_memory_map;
	strcpy(memory_map->signature, "MMAP");
	memory_map->version = 1;
	if ((memory_map_info->entry_count == 0xFFFFFFFF) || (memory_map_info->entry_count == 0xFFFF) || (((signed int)memory_map_info->entry_count) == -1))
	{
		printk("Memory Error\n");
		memory_map->count = 0;
		memset(&memory_map->entry[0], 0, 0x2000);
	}
	else {
		memory_map->count = memory_map_info->entry_count;
		volatile unsigned long mmap_addr = memory_map_info->address_ptr;
		void *entry_ptr = (void*)mmap_addr;
		for(int i=0;i<memory_map->count;i++)
		{		
			memcpy(memory_map->entry+i, entry_ptr, sizeof(memory_map_entry_t));
			entry_ptr += sizeof(memory_map_entry_t);
		}
	}
}

void printmemorymap(unsigned long opts)
{
	volatile unsigned long mmap_addr = memory_map_info->address_ptr;
	unsigned long sys_mmap_addr = (unsigned long)system_memory_map;
	
	if ((opts & PRINT_MEMORY_MAP_BIOS_MMAP) != 0) printk("BIOS Memory Map Address: 0x%X\n", mmap_addr);
	if ((opts & PRINT_MEMORY_MAP_SYSTEM_MMAP) != 0) printk("System Memory Map Address: 0x%X\n", sys_mmap_addr);
	if ((opts & PRINT_MEMORY_MAP_ENTRY_COUNT) != 0) printk("Memory Map Entry Count: %d\n", memory_map->count);
    if ((opts & PRINT_MEMORY_MAP_RAM_AVAILABLE) != 0) printk("Usable RAM: %d KB\n", getavailablememorymap()/1024);
    if ((opts & PRINT_MEMORY_MAP_RAM_RESERVED) != 0) printk("Free RAM: %d KB\n", getreservedmemorymap()/1024);
	
	if ((opts & PRINT_MEMORY_MAP_LIST_OFFSET) != 0)
	{
		printk("\n");
		printk("Start    End        Type Extended\n");
		for(int i=0;i<memory_map->count;i++)
		{
			unsigned long base_lo = memory_map->entry[i].base_lo;
			unsigned long len_lo = memory_map->entry[i].length_lo;
			unsigned long type = memory_map->entry[i].type;
			unsigned long acpi = memory_map->entry[i].acpi;		
			unsigned long offset_start = base_lo;
			unsigned long offset_end = offset_start+(len_lo-1);
			printk("%08X-%08X : %04d-%04d\n", offset_start, offset_end, type, acpi);
		}
		printk("\n");
	}
	
	if ((opts & PRINT_MEMORY_MAP_LIST_DETAIL) != 0) printmemory();
}

void printmemory()
{
	unsigned short entry_count = memory_map->count;
	const char *mmap_name[6] = {
		"",
		"Available RAM",
		"Reserved RAM",
		"ACPI RAM",
		"NVS RAM",
		"Bad Memory"
	};
    printk("Base     Length   Type     Extended    Name\n");
	for(int i=0;i<entry_count;i++)
    {
		unsigned long base_lo = memory_map->entry[i].base_lo;
		unsigned long len_lo = memory_map->entry[i].length_lo;
		unsigned long type = memory_map->entry[i].type;
		unsigned long acpi = memory_map->entry[i].acpi;
		char name[64];
		strcpy(name, mmap_name[(unsigned char)type]);
		printk("%08X %08X %08X %08X    ", base_lo, len_lo, type, acpi);
		printk("%s\n", name);
    }
    printk("\n");
}

unsigned long getavailablememorymap()
{
	unsigned short entry_count = memory_map->count;
	unsigned long memory_size = 0;
	for(int i=0;i<entry_count;i++)
    {
		unsigned long type = memory_map->entry[i].type;
        if (type == MEMORY_MAP_AVAILABLE)
		{
			memory_size += memory_map->entry[i].length_lo;
		}
    }
    return memory_size;
}

unsigned long getreservedmemorymap()
{
	unsigned short entry_count = memory_map->count;
	unsigned long free_memory_size = 0;
	for(int i=0;i<entry_count;i++)
    {
		unsigned long type = memory_map->entry[i].type;
        if (type == MEMORY_MAP_RESERVED)
		{
			free_memory_size += memory_map->entry[i].length_lo;
		}
    }
    return free_memory_size;
}
