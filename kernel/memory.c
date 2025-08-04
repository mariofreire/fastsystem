// Fast System Memory
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2024 DSP Interactive.

#include <stdarg.h>
#include "fskrnl.h"
#include "enum.h"

unsigned long page_directory[1024] __attribute__((aligned(4096)));
unsigned long page_table[1024][1024] __attribute__((aligned(4096)));

void * _heap_start;
void * _heap_end;
void * _heap_current;
void * _heap_prev;

unsigned long _heap_prev_position;
unsigned long _heap_last_size_alloc;
unsigned long _heap_last_position;
unsigned long _heap_position;
unsigned long _heap_size;
unsigned long _heap_last_size;

unsigned long _heap_alloc_last_clean_start;
unsigned long _heap_alloc_last_clean_end;

unsigned long _heap_alloc_available;

unsigned long malloc_count = 0;
unsigned long free_count = 0;

unsigned long malloc_history[65536];
unsigned long free_history[65536];

void init_heap(void)
{
	_heap_last_size_alloc = 0;
	_heap_position = HEAP_START + PAGE_SIZE + ALLOC_SIZE_HEADER;
	_heap_last_position = _heap_position;
	_heap_start = (void*)HEAP_START;
	_heap_end = (void*)HEAP_END;
	_heap_current = (void*)_heap_position;
	_heap_size = HEAP_END-HEAP_START;
	_heap_last_size = _heap_size;
	_heap_alloc_last_clean_start = 0;
	_heap_alloc_last_clean_end = 0;
	_heap_alloc_available = 0;
}

void *get_ptr(unsigned long offset)
{
	return ((void*)offset);
}

unsigned long get_addr(void *ptr)
{
	return ((unsigned long)ptr);
}

void *sbrk(size_t len)
{
	unsigned long addr;
	addr = get_addr(_heap_current);
	_heap_current += len;
	_heap_last_size -= len;
	//memset(get_ptr(addr), 0, len);
	return get_ptr(addr);
}

void *malloc(size_t size)
{
	unsigned char *alloc_header;
	unsigned long alloc_pos;
	unsigned long alloc_size;
	union hdr {
		struct
		{
			unsigned char d[4];
		};
		struct
		{
			unsigned long l;
		};
	};
	union hdr pos;
	union hdr siz;
	if ((_heap_alloc_available) >= info->physical_memory) 
	{
		panic(0);
		return NULL;
	}
	if (_heap_last_position < ((HEAP_START + PAGE_SIZE) - ALLOC_SIZE_HEADER)) return NULL;
	if (_heap_last_position >= (HEAP_END-ALLOC_SIZE_HEADER)) return NULL;
	if (size == 0) return NULL;
	_heap_position += _heap_last_size_alloc+ALLOC_SIZE_HEADER;
	alloc_pos = _heap_position;
	alloc_size = size;
	alloc_header = (unsigned char*)_heap_current-ALLOC_SIZE_HEADER;
	if ((size-_heap_last_size_alloc) < 0)
	{
		*alloc_header++ = UCHAR8A(alloc_pos);
		*alloc_header++ = UCHAR8B(alloc_pos);
		*alloc_header++ = UCHAR8C(alloc_pos);
		*alloc_header++ = UCHAR8D(alloc_pos);
		*alloc_header++ = UCHAR8A(alloc_size);
		*alloc_header++ = UCHAR8B(alloc_size);
		*alloc_header++ = UCHAR8C(alloc_size);
		*alloc_header++ = UCHAR8D(alloc_size);
	} else
	{
		pos.l = alloc_pos;
		siz.l = alloc_size;
		for(int i=0;i<4;i++) alloc_header[i] = pos.d[i];
		for(int i=0;i<4;i++) alloc_header[4+i] = siz.d[i];
	}
	_heap_prev = sbrk(_heap_last_size_alloc+ALLOC_SIZE_HEADER);
	_heap_prev_position = get_addr(_heap_prev);
	_heap_last_size_alloc = size;
	_heap_last_position += _heap_last_size_alloc+ALLOC_SIZE_HEADER;
	_heap_alloc_available += size;
	if (_heap_prev_position == 0) 
	{
		panic(_heap_prev_position);
		return NULL;
	}
	if (_heap_current == 0) 
	{
		panic(get_addr(_heap_current));
		return NULL;
	}
	if (_heap_last_size < HEAP_START)
	{
		panic(_heap_last_size);
		return NULL;
	}
	if (malloc_count < 65536) 
	{
		malloc_history[malloc_count] = size;
		malloc_count++;
	}
	else 
	{
		malloc_count = 0;
		malloc_history[malloc_count] = size;
		malloc_count++;
	}
	unsigned long limit = info->physical_memory-1;
	if ((_heap_alloc_available) >= limit) 
	{
		unsigned long current_esp = get_esp();
		/*
		if (malloc_count != 0)
		{
			printk("size: %d, %d, esp: 0x%X, limit %d, count %d\n", malloc_history[malloc_count-1], free_history[free_count-1], current_esp, limit, malloc_count-1);
			printk("at %s %d %s\n", __FILE__, __LINE__, __FUNCTION__ );
		}
		*/
		panic(current_esp);
		return NULL;
	}
	
	return _heap_current;//-ALLOC_SIZE_HEADER;
}

void free(void *ptr)
{
	static unsigned char *alloc_ptr;
	const unsigned char *alloc_header;
	unsigned long alloc_pos;
	unsigned long alloc_size;
	union hdr {
		struct
		{
			unsigned char d[4];
		};
		struct
		{
			unsigned long l;
		};
	};
	union hdr pos;
	union hdr siz;
	alloc_ptr = (unsigned char*)ptr;
	alloc_header = (unsigned char*)ptr-ALLOC_SIZE_HEADER;
	unsigned char pos_a = *alloc_header++;
	unsigned char pos_b = *alloc_header++;
	unsigned char pos_c = *alloc_header++;
	unsigned char pos_d = *alloc_header++;
	unsigned char siz_a = *alloc_header++;
	unsigned char siz_b = *alloc_header++;
	unsigned char siz_c = *alloc_header++;
	unsigned char siz_d = *alloc_header++;
	alloc_pos = UINT32(pos_a, pos_b, pos_c, pos_d);
	alloc_size = UINT32(siz_a, siz_b, siz_c, siz_d);
	_heap_alloc_available -= alloc_size;
	_heap_last_size += alloc_size;
	if (free_count < 65536) 
	{
		free_history[free_count] = alloc_size;
		free_count++;
	}
	else 
	{
		free_count = 0;
		free_history[free_count] = alloc_size;
		free_count++;
	}
	if (_heap_last_size > _heap_size)
	{
		panic(_heap_last_size);
		return;
	}
	
	unsigned long limit = info->physical_memory-1;
	if ((_heap_alloc_available) >= limit) 
	{
		/*
		if (free_count != 0)
		{
			printk("size: %d, %d, esp: 0x%X, limit %d, count %d\n", malloc_history[malloc_count-1], free_history[free_count-1], get_esp(), limit, malloc_count);
			printk("at %s %d %s\n", __FILE__, __LINE__, __FUNCTION__ );
		}
		*/
		panic(0);
		return;
	}
	
	if (alloc_size == 0 || alloc_pos == 0)
	{
		for(int i=0;i<4;i++) pos.d[i] = alloc_header[i];
		for(int i=0;i<4;i++) siz.d[i] = alloc_header[i+4];
		alloc_pos = pos.l;
		alloc_size = siz.l;
	}
	//print_int(alloc_size);
	//putch('\n');
	int i;
	i = 0;
	while(i < alloc_size+ALLOC_SIZE_HEADER)
	{
		if (*alloc_ptr == 0) break;
		*alloc_ptr++ = 0;
		alloc_ptr--;
		i++;
	};
	//_heap_current -= alloc_size;
	//_heap_position -= alloc_size;
	_heap_alloc_last_clean_start = alloc_pos;
	_heap_alloc_last_clean_end = alloc_pos+alloc_size;
}

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
		panic(0x1000);
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

void loadmemmgr()
{
	loadmmap();
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

void createpageblank(void)
{
	int i;
	for(i=0;i<1024;i++)
	{
		page_directory[i] = 0x02;
	}
}

void setpagetables(void)
{
	int i,j;	 
	unsigned long k=0;
	for(i=0;i<1;i++)
	{
		for(j=0;j<1;j++)
		{
			page_table[i][j] = (k * PAGE_SIZE) | PAGE_PRESENT | PAGE_USER;                      // IVT 0x0-0x400  0x0000-0xFFF
			k++;
		}
		for(j=1;j<7;j++)
		{
			page_table[i][j] = (k * PAGE_SIZE) | PAGE_PRESENT | PAGE_READWRITE | PAGE_USER;     //                 PAGE_SIZE-0x6FFF
			k++;
		}
		for(j=7;j<10;j++)
		{
			page_table[i][j] = (k * PAGE_SIZE) | PAGE_PRESENT | PAGE_READWRITE | PAGE_USER;     //                 0x7000-0x9FFF
			k++;
		}
		for(j=10;j<16;j++)
		{
			page_table[i][j] = (k * PAGE_SIZE) | PAGE_PRESENT | PAGE_USER;                      // LOADER KERNEL   0xA000-0xFFFF
			k++;
		}
		for(j=16;j<160;j++)
		{
			page_table[i][j] = (k * PAGE_SIZE) | PAGE_PRESENT | PAGE_READWRITE;                 // KERNEL-RAM      PAGE_SIZE0-0x9FFFF
			k++;
		}
		for(j=160;j<192;j++)
		{
			page_table[i][j] = (k * PAGE_SIZE) | PAGE_PRESENT | PAGE_READWRITE | PAGE_USER;     // VGA-RAM         0xA0000-0xBFFFF
			k++;
		}
		for(j=192;j<256;j++)
		{
			page_table[i][j] = (k * PAGE_SIZE) | PAGE_PRESENT | PAGE_USER;                      // BIOS-ROM        0xC0000-0xFFFFF
			k++;
		}
		for(j=256;j<1024;j++)
		{
			page_table[i][j] = (k * PAGE_SIZE) | PAGE_PRESENT | PAGE_READWRITE | PAGE_USER;     //                 0x100000-0x3FFFFF
			k++;
		}
	}
	for(i=1;i<2;i++)
	{
		for(j=0;j<1024;j++)
		{
			page_table[i][j] = (k * PAGE_SIZE) | PAGE_PRESENT | PAGE_READWRITE | PAGE_USER;     //                 0x400000-0x7FFFFF
			k++;
		}
	}
	for(i=2;i<3;i++)
	{
		for(j=0;j<80;j++)
		{
			page_table[i][j] = (k * PAGE_SIZE) | PAGE_PRESENT | PAGE_USER;                      // SYS_VARS        0x800000-0x84FFFF
			k++;
		}
		for(j=80;j<1024;j++)
		{
			page_table[i][j] = (k * PAGE_SIZE) | PAGE_PRESENT | PAGE_READWRITE | PAGE_USER;     //                 0x850000-0xFFFFFF
			k++;
		}
	}
	for(i=3;i<48;i++)
	{
		for(j=0;j<1024;j++)
		{
			page_table[i][j] = (k * PAGE_SIZE) | PAGE_PRESENT | PAGE_READWRITE | PAGE_USER;     //                 0x1000000-0xBFFFFFF
			k++;
		}
	}	
	for(i=48;i<49;i++)
	{
		for(j=0;j<8;j++)
		{
			page_table[i][j] = (k * PAGE_SIZE) | PAGE_PRESENT | PAGE_USER;                      // KERNEL          0xC000000-0xC008000
			k++;
		}
		for(j=8;j<1024;j++)
		{
			page_table[i][j] = (k * PAGE_SIZE) | PAGE_PRESENT | PAGE_READWRITE | PAGE_USER;     //                 0xC008000-*
			k++;
		}
	}	
	for(i=49;i<1024;i++)
	{
		for(j=0;j<1024;j++)
		{
			page_table[i][j] = (k * PAGE_SIZE) | PAGE_PRESENT | PAGE_READWRITE | PAGE_USER;     //                 0x850000-*
			k++;
		}
	}	
}

void setpagedirs(void)
{	
	int i;
	for(i=0;i<1024;i++)
	{
		page_directory[i] = ((unsigned long)page_table[i]) | PAGE_PRESENT | PAGE_READWRITE | PAGE_USER;  
	}
}

void flushpage(unsigned long addr) 
{
   __asm__ volatile("invlpg (%0)" ::"r" (addr) : "memory");
}

unsigned long placement_address = (unsigned long)&kernel_end;

unsigned long kmalloc_int(unsigned long sz, int a, unsigned long *pa)
{
    if (a == 1 && (placement_address & 0xFFFFF000) )
    {
        placement_address &= 0xFFFFF000;
        placement_address += PAGE_SIZE;
    }
    if (pa)
    {
        *pa = placement_address;
    }
    unsigned long m = placement_address;
    placement_address += sz;
    return m;
}

unsigned long kmalloc_a(unsigned long sz)
{
    return kmalloc_int(sz, 1, 0);
}

unsigned long kmalloc_ap(unsigned long sz, unsigned long *pa)
{
    return kmalloc_int(sz, 1, pa);
}

unsigned long kmalloc_p(unsigned long sz, unsigned long *pa)
{
    return kmalloc_int(sz, 0, pa);
}

unsigned long kmalloc(unsigned long sz)
{
    return kmalloc_int(sz, 0, 0);
}

void page_fault(registers_t *registers)
{
	unsigned long exc = USHORT16(registers->int_no, registers->eip);
	unsigned long faulting_address;
	if (enter_kernelmode == 1) 
	{
		kernelmode_start();
		shell_code_exit(0);
		return;
	}
	__asm__ volatile ("mov %%cr2, %0" : "=r" (faulting_address));	
	printk("Faulting address 0x%X\n", faulting_address);
	printk("Exception at interrupt %d", registers->int_no);
	if (registers->int_no < 32)
	{
		printk(": %s", exception_messages[registers->int_no]);
	}
	putch('\n');
	panic(exc);
	halt();
    while(1);
}

#ifdef ONLY_C

void load_page_directory(unsigned long *page_dir)
{
    __asm__ volatile("mov %0, %%cr3":: "r"(page_dir));
}

void enable_paging(void)
{
    unsigned long cr0;
    __asm__ volatile("mov %%cr0, %0": "=r"(cr0));
    cr0 |= 0x80000000; // Set paging bit
    __asm__ volatile("mov %0, %%cr0":: "r"(cr0));	
}

void enable_pse(void)
{
    unsigned long cr4;
    __asm__ volatile("mov %%cr4, %0": "=r"(cr4));
    cr4 |= 0x00000010; // Set pse bit
    __asm__ volatile("mov %0, %%cr4":: "r"(cr4));	
}

void enable_sse(void)
{
    unsigned long cr0, cr4;
    __asm__ volatile("mov %%cr0, %0": "=r"(cr0));
	cr0 &= 0xFFFB;
    cr0 |= 2;
    __asm__ volatile("mov %0, %%cr0":: "r"(cr0));
    __asm__ volatile("mov %%cr4, %0": "=r"(cr4));
    cr4 |= 1536;
    __asm__ volatile("mov %0, %%cr4":: "r"(cr4));
}

#else

extern void load_page_directory(unsigned long *page_dir);
extern void enable_paging(void);
extern void enable_pse(void);
extern void enable_sse(void);

#endif


void loadpages(void)
{
	register_interrupt_handler(14, page_fault);	
	disable_interrupt();
	createpageblank();
	setpagetables();	
	setpagedirs();
	load_page_directory(page_directory);	
	flushpage((unsigned long)page_directory);
	enable_pse();
	enable_paging();	
	enable_sse();
	enable_interrupt();
}


/*
void *get_physaddr(void *virtualaddr) 
{
    unsigned long pdindex = (unsigned long)virtualaddr >> 22;
    unsigned long ptindex = (unsigned long)virtualaddr >> 12 & 0x03FF;
 
    unsigned long *pd = (unsigned long *)0xFFFFF000;
    // Here you need to check whether the PD entry is present.
 
    unsigned long *pt = ((unsigned long *)0xFFC00000) + (0x400 * pdindex);
    // Here you need to check whether the PT entry is present.
 
    return (void *)((pt[ptindex] & ~0xFFF) + ((unsigned long)virtualaddr & 0xFFF));
}

void map_page(void *physaddr, void *virtualaddr, unsigned int flags) 
{
    // Make sure that both addresses are page-aligned.
 
    unsigned long pdindex = (unsigned long)virtualaddr >> 22;
    unsigned long ptindex = (unsigned long)virtualaddr >> 12 & 0x03FF;
 
    unsigned long *pd = (unsigned long *)0xFFFFF000;
    // Here you need to check whether the PD entry is present.
    // When it is not present, you need to create a new empty PT and
    // adjust the PDE accordingly.
 
    unsigned long *pt = ((unsigned long *)0xFFC00000) + (0x400 * pdindex);
    // Here you need to check whether the PT entry is present.
    // When it is, then there is already a mapping present. What do you do now?
 
    pt[ptindex] = ((unsigned long)physaddr) | (flags & 0xFFF) | 0x01; // Present
 
    // Now you need to flush the entry in the TLB
    // or you might not notice the change.
}
*/

