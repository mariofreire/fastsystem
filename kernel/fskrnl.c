// Fast System Kernel
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2024 DSP Interactive.

#include <stdarg.h>
#include "fskrnl.h"
#include "enum.h"


unsigned char *root_sector = (unsigned char *)SYSTEM_ROOT_SECTOR;
unsigned char *mbr_sector = (unsigned char *)SYSTEM_MBR_SECTOR;
unsigned char *boot_sector = (unsigned char *)SYSTEM_BOOT_SECTOR;
unsigned char *disk_address_packet = (unsigned char *)SYSTEM_DISK_ADDRESS_PACKET;
unsigned char *vga_memory = (unsigned char *)SYSTEM_VGA_MEMORY;
unsigned char *video_memory = (unsigned char *)SYSTEM_VIDEO_MEMORY;
unsigned char *vesa_info_buffer = (unsigned char *)SYSTEM_VESA_INFO_BUFFER;
unsigned char *vesa_mode_buffer = (unsigned char *)SYSTEM_VESA_MODE_BUFFER;
unsigned char *fat32_fsinfo = (unsigned char *)SYSTEM_FAT32_FSINFO;
unsigned char *loader_kernel = (unsigned char *)SYSTEM_LOADER_KERNEL;
unsigned char *external_kernel = (unsigned char *)SYSTEM_EXTERNAL_KERNEL;
unsigned char *kernel = (unsigned char *)SYSTEM_KERNEL;
unsigned char *system_variables = (unsigned char *)SYSTEM_ADDRESS_VARIABLES;
unsigned char *system_variables_info = (unsigned char *)SYSTEM_ADDRESS_VARIABLES_INFO;
unsigned char *system_variables_enum = (unsigned char *)SYSTEM_ADDRESS_VARIABLES_ENUM;
unsigned char *system_variables_enum_info = (unsigned char *)SYSTEM_ADDRESS_VARIABLES_ENUM_INFO;
unsigned char *system_info = (unsigned char *)SYSTEM_ADDRESS_INFO;
unsigned char *system_pci = (unsigned char *)SYSTEM_ADDRESS_PCI;
unsigned char *system_errno = (unsigned char *)SYSTEM_ADDRESS_ERRNO;
unsigned char *system_memory_map_info = (unsigned char *)SYSTEM_ADDRESS_MEMORY_MAP_INFO;
unsigned char *system_memory_map = (unsigned char *)SYSTEM_ADDRESS_MEMORY_MAP;
unsigned char *ahci_ptr = (unsigned char *)SYSTEM_AHCI_PTR;


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

dap_t* dap;
mbr_t* mbr;
sys_vars_t *sys_vars;
sys_vars_info_t *sys_vars_info;
sys_enum_t *sys_enum;
sys_enum_info_t *sys_enum_info;
system_info_t *info;
multiboot_info_t *multiboot_info;
partition_entry_t* partition;
partition_entry_t *main_partition;
fat_t *fat;
fat32_fsinfo_t *fsinfo;
system_pci_t *sys_pci;
memory_map_t *memory_map;
memory_map_info_t *memory_map_info;
int active_partition=-1;
file_entry_t file_dir_sector[16];
char volume_id[11];
unsigned short storage_drive_controller = STORAGE_CONTROLLER_NONE;

external_kernel_t extern_kernel;

gdt_t gdt;
idt_t idt;
tss_t tss;

vesa_info_t *vesa_info;
vesa_mode_t *vesa_mode;

unsigned char textattr = TEXTCOLOR_DEFAULT;

unsigned long kernel_stack=0;

int mbr_loaded=0;
int mbr_active=0;

unsigned char bootstrap[FAT32_BOOTSTRAP_SIZE];

pci_device_t pci_device[32];
unsigned char pci_count=0;

int pci_video_memory_found=0;
unsigned long pci_video_memory_address = 0;

int sys_vars_loaded = 0;
int sys_enum_loaded = 0;

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

extern void isr48();
extern void isr49();
extern void isr50();
extern void isr51();
extern void isr52();
extern void isr53();
extern void isr54();
extern void isr55();
extern void isr56();
extern void isr57();
extern void isr58();
extern void isr59();
extern void isr60();
extern void isr61();
extern void isr62();
extern void isr63();
extern void isr64();
extern void isr65();
extern void isr66();
extern void isr67();
extern void isr68();
extern void isr69();
extern void isr70();
extern void isr71();
extern void isr72();
extern void isr73();
extern void isr74();
extern void isr75();
extern void isr76();
extern void isr77();
extern void isr78();
extern void isr79();
extern void isr80();
extern void isr81();
extern void isr82();
extern void isr83();
extern void isr84();
extern void isr85();
extern void isr86();
extern void isr87();
extern void isr88();
extern void isr89();
extern void isr90();
extern void isr91();
extern void isr92();
extern void isr93();
extern void isr94();
extern void isr95();
extern void isr96();
extern void isr97();
extern void isr98();
extern void isr99();
extern void isr100();
extern void isr101();
extern void isr102();
extern void isr103();
extern void isr104();
extern void isr105();
extern void isr106();
extern void isr107();
extern void isr108();
extern void isr109();
extern void isr110();
extern void isr111();
extern void isr112();
extern void isr113();
extern void isr114();
extern void isr115();
extern void isr116();
extern void isr117();
extern void isr118();
extern void isr119();
extern void isr120();
extern void isr121();
extern void isr122();
extern void isr123();
extern void isr124();
extern void isr125();
extern void isr126();
extern void isr127();
extern void isr128();
extern void isr129();
extern void isr130();
extern void isr131();
extern void isr132();
extern void isr133();
extern void isr134();
extern void isr135();
extern void isr136();
extern void isr137();
extern void isr138();
extern void isr139();
extern void isr140();
extern void isr141();
extern void isr142();
extern void isr143();
extern void isr144();
extern void isr145();
extern void isr146();
extern void isr147();
extern void isr148();
extern void isr149();
extern void isr150();
extern void isr151();
extern void isr152();
extern void isr153();
extern void isr154();
extern void isr155();
extern void isr156();
extern void isr157();
extern void isr158();
extern void isr159();
extern void isr160();
extern void isr161();
extern void isr162();
extern void isr163();
extern void isr164();
extern void isr165();
extern void isr166();
extern void isr167();
extern void isr168();
extern void isr169();
extern void isr170();
extern void isr171();
extern void isr172();
extern void isr173();
extern void isr174();
extern void isr175();
extern void isr176();
extern void isr177();
extern void isr178();
extern void isr179();
extern void isr180();
extern void isr181();
extern void isr182();
extern void isr183();
extern void isr184();
extern void isr185();
extern void isr186();
extern void isr187();
extern void isr188();
extern void isr189();
extern void isr190();
extern void isr191();
extern void isr192();
extern void isr193();
extern void isr194();
extern void isr195();
extern void isr196();
extern void isr197();
extern void isr198();
extern void isr199();
extern void isr200();
extern void isr201();
extern void isr202();
extern void isr203();
extern void isr204();
extern void isr205();
extern void isr206();
extern void isr207();
extern void isr208();
extern void isr209();
extern void isr210();
extern void isr211();
extern void isr212();
extern void isr213();
extern void isr214();
extern void isr215();
extern void isr216();
extern void isr217();
extern void isr218();
extern void isr219();
extern void isr220();
extern void isr221();
extern void isr222();
extern void isr223();
extern void isr224();
extern void isr225();
extern void isr226();
extern void isr227();
extern void isr228();
extern void isr229();
extern void isr230();
extern void isr231();
extern void isr232();
extern void isr233();
extern void isr234();
extern void isr235();
extern void isr236();
extern void isr237();
extern void isr238();
extern void isr239();
extern void isr240();
extern void isr241();
extern void isr242();
extern void isr243();
extern void isr244();
extern void isr245();
extern void isr246();
extern void isr247();
extern void isr248();
extern void isr249();
extern void isr250();
extern void isr251();
extern void isr252();
extern void isr253();
extern void isr254();
extern void isr255();

void reload_devices(void);

extern void halt(void);

void settss_entries(void);

void msleep(unsigned int milliseconds);

unsigned long getrootdirsector(void);
unsigned char readsector(unsigned long sector, unsigned char *buffer);
unsigned long get_total_files_size(unsigned long sector);

unsigned long ahci_hba_address = 0;
ahci_hba_port_t *ahci_port;
ahci_hba_memory_t *ahci_hba;
ahci_t *ahci;

dma_channel_t dma_channels[DMA_CHANNELS];

unsigned long total_size_files = 0;

unsigned char kernelmode = 1;
unsigned char usermode = 0;
unsigned char enter_kernelmode = 0;

unsigned char restart_init = 0;
unsigned char shutdown_init = 0;

unsigned long multiboot_address = 0;

typedef void (*isr_t)(registers_t *);
void register_interrupt_handler(unsigned char n, isr_t handler);

isr_t interrupt_handlers[256];

char *loaderfilename = "loader";
char *kernelfilename = "fskrnl";

int current_text_x = 0;
int current_text_y = 0;

unsigned short timer_frequency = 100;

void* memcpy(void *s1, const void *s2, size_t n);
void* memset(void *s, int c, size_t n);

int uuidv4_validate(const char *str);

unsigned long current_sector_pwd = 0;

unsigned long int rand_next = 1;

int syscall_excv_on = 0;
int syscall_exit_on = 0;
int kb_flush_require = 0;

char root_dev[256];
char mount_p[256];
char dir_path[1024];
char pwd[1024];

char current_module_name[256];

char *getcwd_sector(unsigned long sector);

int getargc(const char *s);
void getargv(char *s1, const char *s2, int n);

void syscall_exit(unsigned long exitcode);

char* get_current_module(void);

char *push_module(const char *module_name);
char *pop_module(void);


void file_dir_to_file_entry(file_dir_t* dir, file_entry_t* entry);
void file_entry_to_file_dir(file_entry_t* entry, file_dir_t* dir);
void file_entry_to_file_dir_sector(file_entry_t* entry, file_dir_t* dir, unsigned long sector);
void getshortfilename(char *filename11, char *sfilename);
unsigned char getlongfilename(char *filename, unsigned long sector);
unsigned char getfilepathfromsector(unsigned long sector, char *filename);
unsigned char getfilenamefromsector(unsigned long sector, char *filename);
unsigned char getfileinsector(unsigned long sector, file_dir_t *entry, unsigned long *parent_sector);


int init_ahci(void);
int init_ahci_ports();
void detectahci(void);
int check_ahci_port(int portno);
int check_ahci_ports(unsigned char *ports, unsigned char *portmax);
int check_ahci_type(ahci_hba_port_t *port);
void ahci_start_cmd(ahci_hba_port_t *port);
void ahci_stop_cmd(ahci_hba_port_t *port);
unsigned long sata_read(int id, void *buffer, unsigned long sector, unsigned long count);
unsigned char get_sata_ident(ahci_hba_port_t *port, void *buffer);
ahci_hba_memory_t *get_ahci_hba_memory(void);
void *get_ahci_hba_ptr(void);
unsigned long get_ahci_hba(void);

unsigned long pci_config_address(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset);
unsigned char pci_read_byte(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset);
unsigned short pci_read_word(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset);
unsigned long pci_read_long(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset);
void pci_write_byte(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset, unsigned char value);
void pci_write_word(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset, unsigned short value);
void pci_write_long(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset, unsigned long value);


#define MODULE_STACK_SIZE 4096
#define MODULE_NAME_SIZE 256

char **module_stack;
int module_stack_top=-1;

#define auto_dstr(x) (isnumber(x[0]) ? ((x[1] == 'x') ? StrToHex(x) : atol(x)) : ((((x[0] == '-') && isnumber(x[1])) ? atol(x) : ((unsigned long)x))))


int abs(int x) 
{
	int r = x;
	if (r < 0) 
	{
		r = -r;
	}
	return r;
}


unsigned short get_ds(void)
{
    unsigned short ds_reg;
    asm("mov %%ds, %0" : "=r" (ds_reg));
	return ds_reg;
}

unsigned long get_ebx(void)
{
    unsigned long ebx_reg;
    asm("mov %%ebx, %0" : "=r" (ebx_reg));
	return ebx_reg;
}

unsigned long get_ecx(void)
{
    unsigned long ecx_reg;
    asm("mov %%ecx, %0" : "=r" (ecx_reg));
	return ecx_reg;
}

unsigned long get_edx(void)
{
    unsigned long edx_reg;
    asm("mov %%edx, %0" : "=r" (edx_reg));
	return edx_reg;
}

unsigned long get_eax(void)
{
    unsigned long eax_reg;
    asm("mov %%eax, %0" : "=r" (eax_reg));
	return eax_reg;
}

unsigned long get_ebp(void)
{
    unsigned long ebp_reg;
    asm("mov %%ebp, %0" : "=r" (ebp_reg));
	return ebp_reg;
}

unsigned long get_esp(void)
{
    unsigned long esp_reg;
    asm("mov %%esp, %0" : "=r" (esp_reg));
	return esp_reg;
}

unsigned long get_edi(void)
{
    unsigned long edi_reg;
    asm("mov %%edi, %0" : "=r" (edi_reg));
	return edi_reg;
}

unsigned long get_esi(void)
{
    unsigned long esi_reg;
    asm("mov %%esi, %0" : "=r" (esi_reg));
	return esi_reg;
}


unsigned char inb(unsigned short port)
{
    unsigned char r;
    __asm__ volatile   ( "inb %1, %0"
                   : "=a"(r)
                   : "Nd"(port)
                   : "memory");
    return r;
}

void outb(unsigned short port, unsigned char value)
{
    __asm__ volatile   ( "outb %0, %1"
				   : 
				   : "a"(value), 
				     "Nd"(port) 
				   : "memory");
}

unsigned short inw(unsigned short port)
{
    unsigned short r;
    __asm__ volatile   ( "inw %1, %0"
                   : "=a"(r)
                   : "Nd"(port)
                   : "memory");
    return r;
}

void outw(unsigned short port, unsigned short value)
{
    __asm__ volatile   ( "outw %0, %1"
				   : 
				   : "a"(value), 
				     "Nd"(port) 
				   : "memory");
}


unsigned long inl( unsigned short port )
{
  unsigned long r;
  __asm__ volatile    ("inl %1, %0\n"
				   : "=a"( r )
				   : "dN"( port ));
  return r;
}

void outl(unsigned short port, unsigned long value)
{
  __asm__ volatile ("outl %1, %0\n"
					:
					: "dN"(port),
					  "a"(value));
}


int getcursor(void)
{
	unsigned short offset;
	int r;
	if ((enum_loaded) && (total_enum > 0) && (has_enum(SYSTEM_STDIO_SERIAL))) return 0;
    outb(0x3d4, 14);
    offset = (inb(0x3d5) << 8);
    outb(0x3d4, 15);
    offset += inb(0x3d5);
	r = (offset * 2);
	return r;
}

void setcursor(int x, int y)
{
	unsigned short offset;
	if ((enum_loaded) && (total_enum > 0) && (has_enum(SYSTEM_STDIO_SERIAL))) return;
	offset = textoffset(x,y) / 2;
    outb(0x3d4, 14);
    outb(0x3d5, (offset >> 8));
    outb(0x3d4, 15);
    outb(0x3d5, offset);
}

void setcursoroffset(unsigned short offset)
{
	if ((enum_loaded) && (total_enum > 0) && (has_enum(SYSTEM_STDIO_SERIAL))) return;
    outb(0x3d4, 14);
    outb(0x3d5, (offset >> 8));
    outb(0x3d4, 15);
    outb(0x3d5, offset);
}

void getcursorxy(int *x, int *y)
{
    unsigned short offset;
	if ((enum_loaded) && (total_enum > 0) && (has_enum(SYSTEM_STDIO_SERIAL))) return;
	offset = getcursor();
	*x = textoffsetx(offset);
	*y = textoffsety(offset);
}

unsigned char getcolor(unsigned char attr)
{
        return ((unsigned char)(attr & 0x0f));
}

unsigned char getbackground(unsigned char attr)
{
        return ((unsigned char)((attr >> 4) & 0x0f));
}

unsigned char toattr(unsigned char color, unsigned char background)
{
        return ((unsigned char)((color & 0x0f) | ((background & 0x0f) << 4)));
}

void setcolor(unsigned char color)
{
	unsigned char bgcolor = getbackground(textattr);
	unsigned char fgcolor = color;
	textattr = toattr(fgcolor, bgcolor);
}

void textcolor(unsigned char color)
{
	setcolor(color);
}

void settextcolor(unsigned char color)
{
	setcolor(color);
}

void setfgcolor(unsigned char color)
{
	setcolor(color);
}

void setbkcolor(unsigned char color)
{
	unsigned char bgcolor = color;
	unsigned char fgcolor = getcolor(textattr);
	textattr = toattr(fgcolor, bgcolor);
}

int wherex(void)
{
	unsigned short offset;
	int r;
	if ((enum_loaded) && (total_enum > 0) && (has_enum(SYSTEM_STDIO_SERIAL))) return 0;
	offset = getcursor();
	r = textoffsetx(offset);
	return r;
}

int wherey(void)
{
	unsigned short offset;
	int r;
	if ((enum_loaded) && (total_enum > 0) && (has_enum(SYSTEM_STDIO_SERIAL))) return 0;
	offset = getcursor();
	r = textoffsety(offset);
	return r;
}

void gotoxy(int x, int y)
{
	if ((enum_loaded) && (total_enum > 0) && (has_enum(SYSTEM_STDIO_SERIAL))) return;
	current_text_x = x;
	current_text_y = y;
	setcursor(current_text_x, current_text_y);
}

void clrscr(void)
{
	int i;
	if ((enum_loaded) && (total_enum > 0) && (has_enum(SYSTEM_STDIO_SERIAL))) return;
	for(i=0;i<(TEXT_COLS*TEXT_ROWS*2);i+=2)
	{
		vga_memory[i+0] = 0;
		vga_memory[i+1] = toattr(TEXTCOLOR_DEFAULT, BLACK);
	}
	gotoxy(0, 0);
}

unsigned short putchar_xy(int x, int y, const char c, unsigned char a) 
{
	int i,j;
	unsigned char *vga;
	unsigned short cursor_offset;	
	if ((enum_loaded) && (total_enum > 0) && (has_enum(SYSTEM_STDIO_SERIAL))) return 0;
	if ((x >= TEXT_COLS) || (y >= TEXT_ROWS)) return textoffset(x, y);
	cursor_offset = textoffset(x, y);
	if (c == '\n')
	{
		current_text_y = textoffsety(cursor_offset) + 1;
		cursor_offset = textoffset(0, current_text_y);
	}
	else 
	if (c == '\r')
	{
		cursor_offset = textoffset(0, current_text_y);
	}
	else 
	if (c == '\t')
	{
		current_text_x = textoffsetx(cursor_offset) + 8;
		cursor_offset = textoffset(current_text_x, current_text_y);
	}
	else 
	if (c == '\b')
	{
		current_text_x = textoffsetx(cursor_offset) - 1;
		cursor_offset = textoffset(current_text_x, current_text_y);
	}
	else 
	{
		vga_memory[cursor_offset+0] = c;
		vga_memory[cursor_offset+1] = a;
		cursor_offset += 2;		
	}
	if (cursor_offset >= (TEXT_COLS*TEXT_ROWS*2))
	{
		for (i=1;i<TEXT_ROWS;i++)
		{
			memcpy((vga_memory+textoffset(0,(i-1))), (vga_memory+textoffset(0,i)), TEXT_COLS*2);
		}
		vga = vga_memory+textoffset(0,(TEXT_ROWS-1));
		for(j=0;j<(TEXT_COLS*2);j+=2)
		{
			vga[j+0] = ' ';
			vga[j+1] = a;
		}
		cursor_offset -= (TEXT_COLS*2);
	}
	setcursoroffset(cursor_offset);
	current_text_x = textoffsetx(cursor_offset);
	current_text_y = textoffsety(cursor_offset);
	return cursor_offset;
}

void cputchar_a(unsigned char a, char c) 
{
	unsigned short cursor_offset;
	if ((enum_loaded) && (total_enum > 0) && (has_enum(SYSTEM_STDIO_SERIAL))) return;
	cursor_offset = getcursor();
	current_text_x = textoffsetx(cursor_offset);
	current_text_y = textoffsety(cursor_offset);
	cursor_offset = putchar_xy(current_text_x,current_text_y,c,a);
	current_text_x = textoffsetx(cursor_offset);
	current_text_y = textoffsety(cursor_offset);
	setcursor(current_text_x, current_text_y);
}

void sputchar(char c)
{
	outb(0x3F8, c);
}

void cputchar(unsigned char a, char c)
{
	if ((enum_loaded) && (total_enum > 0) && (has_enum(SYSTEM_STDIO_SERIAL)))
	{
		if (c == '\n')
			sputchar('\r');
		sputchar(c);
	}
	else cputchar_a(a, c);
}

void cputs(unsigned char a, const char *s) 
{
    while(*s) 
	{
		cputchar(a, *s++);
    }
	cputchar(a, '\n');
}

void cputch(unsigned char a, const char c)
{
	cputchar(a, c);
}

void cprint(unsigned char a, const char *s)
{
    while(*s)
	{
		cputch(a, *s++);
    }
}

void putchar(const char c) 
{
	cputchar(TEXTCOLOR_DEFAULT, c);
}

void putch(const char c) 
{
	putchar(c);
}

void puts(const char *s) 
{
    while(*s) 
	{
		putch(*s++);
    }
	putch('\n');
}

void print(const char *s) 
{
    while(*s) 
	{
		putch(*s++);
    }
}


void print_int(int n)
{
	int i=0,j=0,k=0,a=0;
	char buf[16];
	char rbuf[16];
	a = n;
	do
	{
		buf[i++] = a % 10 + '0';
		a /= 10;
	} while (a > 0);
	buf[i] = '\0';
	k=i;
	while(j < i)
	{
		rbuf[j++] = buf[k-1];
		k--;
	}
	rbuf[j] = '\0';
	print(rbuf);
}

unsigned char get_csi_color(unsigned char n, unsigned char c)
{
	unsigned char r = 0;
	switch(n)
	{
		case 0:
		{
			switch (c)
			{
				case BLACK:
					r = BLACK;
					break;
				case RED:
					r = RED;
					break;
				case GREEN:
					r = GREEN;
					break;
				case BROWN:
					r = BROWN;
					break;
				case BLUE:
					r = BLUE;
					break;
				case MAGENTA:
					r = MAGENTA;
					break;
				case CYAN:
					r = CYAN;
					break;
				case GRAY:
					r = GRAY;
					break;
				case SILVER:
					r = SILVER;
					break;
				
			};
		};
		break;
		case 1:
		{
			switch (c)
			{
				case BLACK:
					r = GRAY;
					break;
				case RED:
					r = LIGHTRED;
					break;
				case GREEN:
					r = LIGHTGREEN;
					break;
				case BROWN:
					r = YELLOW;
					break;
				case BLUE:
					r = LIGHTBLUE;
					break;
				case MAGENTA:
					r = LIGHTMAGENTA;
					break;
				case CYAN:
					r = LIGHTCYAN;
					break;
				case GRAY:
					r = SILVER;
					break;
				case SILVER:
					r = WHITE;
					break;
				
			};			
		};
		break;
	};
	return r;
}

void get_sgr_color(unsigned char sgr_code, unsigned char csi_code)
{
	switch (sgr_code) 
	{
		case 30:
		{
			setcolor(get_csi_color(csi_code, BLACK));
			setbkcolor(BLACK);
		}
		break;
		case 31:
		{
			setcolor(get_csi_color(csi_code, RED));
			setbkcolor(BLACK);
		}
		break;
		case 32:
		{
			setcolor(get_csi_color(csi_code, GREEN));
			setbkcolor(BLACK);
		}
		break;
		case 33:
		{
			setcolor(get_csi_color(csi_code, BROWN));
			setbkcolor(BLACK);
		}
		break;
		case 34:
		{
			setcolor(get_csi_color(csi_code, BLUE));
			setbkcolor(BLACK);
		}
		break;
		case 35:
		{
			setcolor(get_csi_color(csi_code, MAGENTA));
			setbkcolor(BLACK);
		}
		break;
		case 36:
		{
			setcolor(get_csi_color(csi_code, CYAN));
			setbkcolor(BLACK);
		}
		break;
		case 37:
		{
			setcolor(get_csi_color(csi_code, SILVER));
			setbkcolor(BLACK);
		}
		break;
		case 39:
		{
			setcolor(get_csi_color(csi_code, SILVER));
			setbkcolor(BLACK);
		}
		break;
		case 40:
		{
			setbkcolor(BLACK);
			setcolor(get_csi_color(csi_code, SILVER));
		}
		break;
		case 41:
		{
			setbkcolor(RED);
			setcolor(get_csi_color(csi_code, SILVER));
		}
		break;
		case 42:
		{
			setbkcolor(GREEN);
			setcolor(get_csi_color(csi_code, SILVER));
		}
		break;
		case 43:
		{
			setbkcolor(BROWN);
			setcolor(get_csi_color(csi_code, SILVER));
		}
		break;
		case 44:
		{
			setbkcolor(BLUE);
			setcolor(get_csi_color(csi_code, SILVER));
		}
		break;
		case 45:
		{
			setbkcolor(MAGENTA);
			setcolor(get_csi_color(csi_code, SILVER));
		}
		break;
		case 46:
		{
			setbkcolor(CYAN);
			setcolor(get_csi_color(csi_code, SILVER));
		}
		break;
		case 47:
		{
			setbkcolor(SILVER);
			setcolor(get_csi_color(csi_code, SILVER));
		}
		break;
		case 49:
		{
			setbkcolor(BLACK);
			setcolor(get_csi_color(csi_code, SILVER));
		}
		break;
		case 90:
		{
			setcolor(GRAY);
			setbkcolor(BLACK);
		}
		break;
		case 91:
		{
			setcolor(LIGHTRED);
			setbkcolor(BLACK);
		}
		break;
		case 92:
		{
			setcolor(LIGHTGREEN);
			setbkcolor(BLACK);
		}
		break;
		case 93:
		{
			setcolor(YELLOW);
			setbkcolor(BLACK);
		}
		break;
		case 94:
		{
			setcolor(LIGHTBLUE);
			setbkcolor(BLACK);
		}
		break;
		case 95:
		{
			setcolor(LIGHTMAGENTA);
			setbkcolor(BLACK);
		}
		break;
		case 96:
		{
			setcolor(LIGHTCYAN);
			setbkcolor(BLACK);
		}
		break;
		case 97:
		{
			setcolor(WHITE);
			setbkcolor(BLACK);
		}
		break;
		case 100:
		{
			setcolor(get_csi_color(csi_code, SILVER));
			setbkcolor(GRAY);
		}
		break;
		case 101:
		{
			setcolor(get_csi_color(csi_code, SILVER));
			setbkcolor(LIGHTRED);
		}
		break;
		case 102:
		{
			setcolor(get_csi_color(csi_code, SILVER));
			setbkcolor(LIGHTGREEN);
		}
		break;
		case 103:
		{
			setcolor(get_csi_color(csi_code, SILVER));
			setbkcolor(YELLOW);
		}
		break;
		case 104:
		{
			setcolor(get_csi_color(csi_code, SILVER));
			setbkcolor(LIGHTBLUE);
		}
		break;
		case 105:
		{
			setcolor(get_csi_color(csi_code, SILVER));
			setbkcolor(LIGHTMAGENTA);
		}
		break;
		case 106:
		{
			setcolor(get_csi_color(csi_code, SILVER));
			setbkcolor(LIGHTCYAN);
		}
		break;
		case 107:
		{
			setcolor(get_csi_color(csi_code, SILVER));
			setbkcolor(WHITE);
		}
		break;
		case 0:
		{
			setbkcolor(BLACK);
			setcolor(SILVER);
		}
		break;
		case 108:
		{
			setbkcolor(BLACK);
			setcolor(SILVER);
		}
		break;
		case 1:
		{
			setbkcolor(BLACK);
			setcolor(WHITE);
		}
		break;
		case 2:
		{
			setbkcolor(BLACK);
			setcolor(GRAY);
		}
		break;
		case 7:
		{
			setbkcolor(SILVER);
			setcolor(BLACK);
		}
		break;
		case 8:
		{
			setbkcolor(BLACK);
			setcolor(BLACK);
		}
		break;
		default:
		{
			
		}
		break;
	}
}

void tprint(const char *s) 
{
    while (*s != '\0') 
	{
        if (*s == '\033') 
		{
            s++;
            if (*s == '[') 
			{
                s++;
                int csi_code = 0;
                int sgr_code = 0;
                int number_code = 0;
                if (*s == ';') 
				{
                    s++;
                }
                if (*s >= '0' && *s <= '9')
                {
                    while (*s >= '0' && *s <= '9') 
					{
                        number_code = number_code * 10 + (*s - '0');
                        s++;
                        if (*s == ';') 
						{
                            s++;
                            csi_code = number_code;
                            number_code = 0;
                        }
                    }
                    sgr_code = number_code;
                }
                if (*s == 'm') 
				{
                    get_sgr_color(sgr_code, csi_code);
                }
            }
        } else 
		{
            cputch(textattr, *s);
        }
        s++;
    }
}

void tprintl(const char *s, unsigned long l) 
{
	unsigned char print_status = 0;
    while ((*s != '\0') && (l--))
	{
        if (*s == '\033') 
		{
			print_status = 1;
            s++;
            if (*s == '[') 
			{
				print_status = 1;
                s++;
                int csi_code = 0;
                int sgr_code = 0;
                int number_code = 0;
                if (*s == ';') 
				{
					print_status = 1;
                    s++;
                }
                if (*s >= '0' && *s <= '9')
                {
					print_status = 1;
                    while (*s >= '0' && *s <= '9') 
					{
                        number_code = number_code * 10 + (*s - '0');
                        s++;
                        if (*s == ';') 
						{
                            s++;
                            csi_code = number_code;
                            number_code = 0;
                        }
                    }
                    sgr_code = number_code;
                }
                if (*s == 'm') 
				{
					print_status = 1;
                    get_sgr_color(sgr_code, csi_code);
					print_status = 0;
                }
            }
        } else 
		{
			if (print_status == 0)
			{
				cputch(textattr, *s);
			}
        }
        s++;
    }
}

unsigned int atoh(char *s)
{
    unsigned int i=0;
    unsigned char c=0;
    while(*s)
    {
        c = (unsigned char)(*s);
        if ((c >= 'a') && (c <= 'f'))
        {
            c -= 'a';
            c += 'A';
        }
        if (((c >= '0') && (c <= '9'))
         || ((c >= 'A') && (c <= 'F')))
        {
            if ((c >= 'A') && (c <= 'F')) 
            {
                c -= 'A';
                c += 10;
            }
            else c -= '0';
            i *= 16;
            i += c;
        }
        s++;
    }
    return i;
}

long int atol(char *s)
{
    int i=0,n=0;
    unsigned char c=0;
    if (*s == '-')
    {
        n = 1;
        s++;
    }
    while(*s)
    {
        c = (unsigned char)(*s);
        if ((c >= '0') && (c <= '9'))
        {
            c -= '0';
            i *= 10;
            i += c;
        }
        s++;
    }
    if (n)
    {
        i = 0-i;
    }
    return i;
}

int atoi(char *s)
{
    return (int)atol(s);
}

char *itob(unsigned long num, unsigned long base)
{
  static char hold[] = "0123456789ABCDEFGHIJKLMNOPQRTSUVWXYZ";
  static char buffer[50];
  char *str;

  str = &buffer[49];
  *str = '\0';

  do {
    *--str = hold[num % base];
    num /= base;
  } while (num != 0);

  return str;
}

char *itob64(unsigned long long num, unsigned long long base)
{
  static char hold[] = "0123456789ABCDEFGHIJKLMNOPQRTSUVWXYZ";
  static char buffer[50];
  char *str;

  str = &buffer[49];
  *str = '\0';

  do {
    *--str = hold[num % base];
    num /= base;
  } while (num != 0);

  return str;
}

char * itoa( int value, char * str, int base )
{
    char * rc;
    char * ptr;
    char * low;
    // Check for supported base.
    if ( base < 2 || base > 36 )
    {
        *str = '\0';
        return str;
    }
    rc = ptr = str;
    // Set '-' for negative decimals.
    if ( value < 0 && base == 10 )
    {
        *ptr++ = '-';
    }
    // Remember where the numbers start.
    low = ptr;
    // The actual conversion.
    do
    {
        // Modulo is negative for negative value. This trick makes abs() unnecessary.
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + value % base];
        value /= base;
    } while ( value );
    // Terminating the string.
    *ptr-- = '\0';
    // Invert the numbers.
    while ( low < ptr )
    {
        char tmp = *low;
        *low++ = *ptr;
        *ptr-- = tmp;
    }
    return rc;
}

void* memcpy(void *s1, const void *s2, size_t n)
{
	char *dest = s1;
	const char *source = s2;
	while (n--)
	{
		*dest++ = *source++;
	}
	return s1;
}

void* memset(void *s, int c, size_t n)
{
	unsigned char *dest = s;
	while (n--)
	{
		*dest++ = (unsigned char)c;
	}
	return s;
}

char *strcpy(char *s1, const char *s2)
{
	char *dest = s1;
	while ((*dest++ = *s2++) != 0);
	return s1;
}

size_t strlen(const char *s)
{
	const char *s1 = s;
	if (s == NULL)
	{
		return 0;
	}
	for (s1 = s; *s1; s1++);
	return (s1 - s);
}

char *strrev(const char *s)
{
	size_t len;
	char *dest = (char*)s;	
	char *s1;	
	char *s2;	
	char c;
	if (s == NULL)
	{
		return 0;
	}	
	len = strlen(s);
	if (len > 0)
	{
		for (s1=(char*)s, s2=((char*)s)+len;s1<s2--;s1++)
		{
			c = *s1;
			*s1 = *s2;
			*s2 = c;
		}
	}
	return dest;
}

int strcmp(const char *s1, const char *s2)
{
	while (*s1 == *s2)
	{
		if (!*s1)
		{
			return 0;
		}
		s1++;
		s2++;
	}
	return *(unsigned const char*)s1 - *(unsigned const char*)s2;	
}

char *strcat(char *s1, const char *s2)
{
	char *dest = s1;
	while (*dest)
	{
		*dest++;
	}
	while ((*dest++ = *s2++) != 0);
	return s1;
}

char* strncpy (char *s1, const char *s2, size_t n)
{
  char *dest = s1;
  while (n--)
  {
    if (!(*dest++ = *s2++))
    {
      while (n--)
        *dest++ = 0;
      break;
    }
  }
  return s1;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
	while (n--)
	{
		if (*s1 != *s2++)
		{
			return *(unsigned const char*)s1 - *(unsigned const char*)--s2;
		}
		if (!*s1++)
		{
			break;
		}
	}
	return 0;
}

char *strupr(const char *s)
{
	size_t len;
	char *dest = (char*)s;
	char *s1;
	const char c = ('a' - 'A');
	if (s == NULL)
	{
		return 0;
	}
	len = strlen(s);
	if (len > 0)
	{
		for (s1=(char*)s;*s1;s1++)
		{
			*s1 = (((*s1 >= 'a') && (*s1 <= 'z')) ? (*s1 - c) : *s1);
		}
	}
	return dest;
}

char *strlwr(const char *s)
{
	size_t len;
	char *dest = (char*)s;
	char *s1;
	const char c = ('a' - 'A');
	if (s == NULL)
	{
		return 0;
	}
	len = strlen(s);
	if (len > 0)
	{
		for (s1=(char*)s;*s1;s1++)
		{
			*s1 = (((*s1 >= 'A') && (*s1 <= 'Z')) ? (*s1 + c) : *s1);
		}
	}
	return dest;
}

char *strchr (const char *s, int c)
{
  char ch = (char)c;

  while (*s)
  {
    if (*s == ch)
	{
      return (char *)s;
	}
    s++;
  }

  if (!ch)
  {
    return (char *)s;
  }

  return NULL;
}

void strcatb(char* s1, char* s2)
{
	int i, j;
    int rlen = strlen(s1);
    int llen = strlen(s2);
    for(i=rlen;i>=0;i--)
    {
        s1[i+llen] = s1[i];
    }
    for(j=0;j<llen;j++)
    {
        s1[j] = s2[j];
    }
}

void panic(unsigned long exception_code)
{
	printk("System Halted: Panic at %s: KernelException(0x%x).\n", kernelfilename, exception_code);
	// printk("Error: Cann't open a operating system.\n");
	halt();
}

void enable_interrupt(void)
{
	//if (usermode) return;
	__asm__ ("sti");
}

void disable_interrupt(void)
{
	//if (usermode) return;
	__asm__ ("cli");
}

void sound(unsigned long freq)
{
 	unsigned long f_div;
 	unsigned char t;
 	f_div = (1193180 / freq);
 	outb(0x43, 0xB6);
 	outb(0x42, (unsigned char)(f_div));
 	outb(0x42, (unsigned char)(f_div >> 8));
 	t = inb(0x61);
  	if (t != (t | 3)) 
	{
 		outb(0x61, t | 3);
 	}
}

void nosound()
{
	unsigned char t;
	t = (inb(0x61) & 0xFC);
	outb(0x61, t);
}

void restart(void)
{
	unsigned char restartcode;
	restart_init = 1;
	restartcode = 0x02;
	disable_interrupt();
	while (restartcode & 0x02)
		restartcode = inb(0x64);
	outb(0x64, 0xFE);
	printk("System Halted.\n");
	halt();
}

void shutdown_int(void)
{
	registers16_t r1, r2, r3;
	memset(&r1, 0, sizeof(registers16_t));
	memset(&r2, 0, sizeof(registers16_t));
	memset(&r3, 0, sizeof(registers16_t));
	r1.ax = 0x5301;
	r1.bx = 0;
	r2.ax = 0x530E;
	r2.bx = 0;
	r2.cx = 0x102;
	r3.ax = 0x5307;
	r3.bx = 1;
	r3.cx = 3;
	int86(0x15, &r1, &r1);
	int86(0x15, &r2, &r2);
	int86(0x15, &r3, &r3);
}	

void shutdown(void)
{
	shutdown_init = 1;
	disable_interrupt();
	outw(0xB004, 0x2000);
	outw(0x604,  0x2000);
	outw(0x4004, 0x3400);
	outw(0x600,  0x34);
	shutdown_int();
	printk("System Halted.\n");
	halt();
}

/*
int create_page_entry(int base_addr, char present, char writable,
                      char privilege_level, char cache_enabled,
                      char write_through_cache, char accessed, char page_size,
                      char dirty)
{
    int entry = 0;

    entry |= present;
    entry |= writable << 1;
    entry |= privilege_level << 2;
    entry |= write_through_cache << 3;
    entry |= cache_enabled << 4;
    entry |= accessed << 5;
    entry |= dirty << 6;
    entry |= page_size << 7;

    return base_addr | entry;
}
*/

extern unsigned long kernel_start;

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

unsigned long kmalloc(unsigned long size);
unsigned long kmalloc_page();

void flushpage(unsigned long addr) 
{
   __asm__ volatile("invlpg (%0)" ::"r" (addr) : "memory");
}

extern void loadPageDirectory(unsigned long*);
extern void enablePaging();
extern void enablePSE();


extern unsigned long kernel_end;
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

extern char *exception_messages[];

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

gdt_entry_t encodegdt(unsigned long limit, unsigned long base, unsigned char access, unsigned char flags)
{
	gdt_entry_t gdt_entry;
	
	gdt_entry.segment_limit = (limit & 0xFFFF);
	gdt_entry.low_base = (base & 0xFFFF);
	gdt_entry.mid_base = ((base & 0xFF0000) >> 16);
	gdt_entry.access = access;
	//gdt_entry.flags = flags;
	gdt_entry.flags = (limit >> 16) & 0x0F;
	gdt_entry.flags |= flags & 0xF0;
	gdt_entry.high_base = ((base & 0xFF000000) >> 24);

	return gdt_entry;
}


void setgdt(int n, unsigned long limit, unsigned long base, unsigned char access, unsigned char flags)
{
	gdt.entry[n].segment_limit = (limit & 0xFFFF);
	gdt.entry[n].low_base = (base & 0xFFFF);
	gdt.entry[n].mid_base = ((base & 0xFF0000) >> 16);
	gdt.entry[n].access = access;
	//gdt.entry[n].flags = flags;
	gdt.entry[n].flags = (limit >> 16) & 0x0F;
	gdt.entry[n].flags |= flags & 0xF0;
	gdt.entry[n].high_base = ((base & 0xFF000000) >> 24);
	gdt.limit = n + 1;
}

void setgdt_entries(void)
{
	setgdt(0, 0x00000000, 0x00000000, 0x00, 0x00); // null
	setgdt(1, 0xFFFFFFFF, 0x00000000, 0x9A, 0xCF); // kernel mode code
	setgdt(2, 0xFFFFFFFF, 0x00000000, 0x92, 0xCF); // kernel mode data
	setgdt(3, 0xFFFFFFFF, 0x00000000, 0xFA, 0xCF); // user mode code
	setgdt(4, 0xFFFFFFFF, 0x00000000, 0xF2, 0xCF); // user mode data
	setgdt(5, 0x00000000, 0x00000000, 0x00, 0x00); // tss
	setgdt(6, 0xFFFFFFFF, 0x00000000, 0x9A, 0x0F); // real mode code
	setgdt(7, 0xFFFFFFFF, 0x00000000, 0x92, 0x0F); // real mode data
	settss_entries();
}

void loadgdt(void)
{
	memset(gdt.entry, 0, 256*sizeof(gdt_entry_t)-1);
	setgdt_entries();
	gdt.descriptor.limit = gdt.limit*sizeof(gdt_entry_t)-1;
	gdt.descriptor.base = (unsigned long)&gdt.entry;
	__asm__ volatile ("lgdt %[gdtr]"
				 : 
				 : [gdtr] "m" (gdt.descriptor));
}

void setidt(int n, unsigned long handler, unsigned short sel, unsigned char flags)
{
	idt.entry[n].low_offset = LOW16(handler);
	idt.entry[n].segment_sel = sel;
	idt.entry[n].unused = 0x00;
	idt.entry[n].flags = flags | 0x60;
	idt.entry[n].high_offset = HIGH16(handler);
	if (n >= idt.limit)
	{
		idt.limit = n + 1;
	}
}

void loadidt(void)
{
	idt.descriptor.limit = 256*sizeof(idt_gate_t)-1;
	idt.descriptor.base = (unsigned long)&idt.entry;
	__asm__ volatile ("lidt %[idtr]"
				 : 
				 : [idtr] "m" (idt.descriptor));
}

void settss(int n, unsigned short ss, unsigned long esp)
{
	unsigned long gdt_limit = gdt.limit;
	unsigned long base = (unsigned long)&tss;
	unsigned long limit = base + sizeof(tss_t);	
	setgdt(n, limit, base, 0xE9, 0x00);
	gdt.limit = gdt_limit;
	memset(&tss, 0, sizeof(tss_t));	
	tss.ss0 = ss;
	tss.esp0 = esp;
	tss.cs = TSS_CODE_SEGMENT;
	tss.ss = TSS_DATA_SEGMENT;
	tss.ds = TSS_DATA_SEGMENT;
	tss.es = TSS_DATA_SEGMENT;
	tss.fs = TSS_DATA_SEGMENT;
	tss.gs = TSS_DATA_SEGMENT;
	tss.iomap_base = sizeof(tss_t);	
}

void settss_stack(unsigned long ss, unsigned long esp)
{
	tss.ss0 = ss;
	tss.esp0 = esp;
}
extern void end();
void settss_entries(void)
{
	//settss(5, KERNEL_MODE_DATA_SEGMENT, 0xF000000);
	
	settss(5, KERNEL_MODE_DATA_SEGMENT, (unsigned long)kernel_end+KERNEL_STACK_SIZE);
	kernel_stack = kmalloc_a(KERNEL_STACK_SIZE);
	settss_stack(KERNEL_MODE_DATA_SEGMENT, kernel_stack+KERNEL_STACK_SIZE);
}

void loadtss(void)
{
	__asm__ volatile ("movw $0x2B, %ax\n\t"
				  	  "ltr %ax");
}

char *exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

void isr_exception(registers_t *registers)
{
	unsigned long exc = USHORT16(registers->int_no, registers->eip);
	if (enter_kernelmode == 1) return;
	if (usermode && syscall_excv_on)
	{
		printk("Fault at address 0x%X in module '%s'.\n", registers->eip, get_current_module());
		printk("Exception at interrupt %d", registers->int_no);
		if (registers->int_no < 32)
		{
			printk(": %s", exception_messages[registers->int_no]);
		}
		putch('\n');
		pop_module();
		syscall_exit(0);
		return;
	}
	if ((shutdown_init == 0) && (restart_init == 0))
	{
		printk("Exception at interrupt %d", registers->int_no);
		if (registers->int_no < 32)
		{
			printk(": %s", exception_messages[registers->int_no]);
		}
		putch('\n');	
	}
	if (!usermode)
	{
		panic(exc);
	}
	else
	{
		if ((shutdown_init == 1) || (restart_init == 1))
		{
			unsigned char * _eip_ptr = (unsigned char*)registers->esp+8;
			unsigned long _eip = (unsigned long)UINT32(_eip_ptr[0],_eip_ptr[1],_eip_ptr[2],_eip_ptr[3]);
			unsigned long r_eip = registers->eip;
			__asm__ volatile ("jmpl *%0"::"r"(r_eip));
		}
		else
		{
			panic(exc);
		}
	}
}

void isr_handler(registers_t *registers)
{
	if (interrupt_handlers[registers->int_no] != 0)
	{
		isr_t handler = interrupt_handlers[registers->int_no];
		handler(registers);
		if (registers->int_no > 47)
		{
			outb(0x20, 0x20);
		}
	}
	else
	{
		if (registers->int_no < 32)
		{
			isr_exception(registers);
		}
	}
}

void loadisr(void)
{
    setidt(0, (unsigned long)isr0, 0x08, 0x8E);
    setidt(1, (unsigned long)isr1, 0x08, 0x8E);
    setidt(2, (unsigned long)isr2, 0x08, 0x8E);
    setidt(3, (unsigned long)isr3, 0x08, 0x8E);
    setidt(4, (unsigned long)isr4, 0x08, 0x8E);
    setidt(5, (unsigned long)isr5, 0x08, 0x8E);
    setidt(6, (unsigned long)isr6, 0x08, 0x8E);
    setidt(7, (unsigned long)isr7, 0x08, 0x8E);
    setidt(8, (unsigned long)isr8, 0x08, 0x8E);
    setidt(9, (unsigned long)isr9, 0x08, 0x8E);
    setidt(10, (unsigned long)isr10, 0x08, 0x8E);
    setidt(11, (unsigned long)isr11, 0x08, 0x8E);
    setidt(12, (unsigned long)isr12, 0x08, 0x8E);
    setidt(13, (unsigned long)isr13, 0x08, 0x8E);
    setidt(14, (unsigned long)isr14, 0x08, 0x8E);
    setidt(15, (unsigned long)isr15, 0x08, 0x8E);
    setidt(16, (unsigned long)isr16, 0x08, 0x8E);
    setidt(17, (unsigned long)isr17, 0x08, 0x8E);
    setidt(18, (unsigned long)isr18, 0x08, 0x8E);
    setidt(19, (unsigned long)isr19, 0x08, 0x8E);
    setidt(20, (unsigned long)isr20, 0x08, 0x8E);
    setidt(21, (unsigned long)isr21, 0x08, 0x8E);
    setidt(22, (unsigned long)isr22, 0x08, 0x8E);
    setidt(23, (unsigned long)isr23, 0x08, 0x8E);
    setidt(24, (unsigned long)isr24, 0x08, 0x8E);
    setidt(25, (unsigned long)isr25, 0x08, 0x8E);
    setidt(26, (unsigned long)isr26, 0x08, 0x8E);
    setidt(27, (unsigned long)isr27, 0x08, 0x8E);
    setidt(28, (unsigned long)isr28, 0x08, 0x8E);
    setidt(29, (unsigned long)isr29, 0x08, 0x8E);
    setidt(30, (unsigned long)isr30, 0x08, 0x8E);
    setidt(31, (unsigned long)isr31, 0x08, 0x8E);
    setidt(65, (unsigned long)isr65, 0x08, 0x8E);
    setidt(106, (unsigned long)isr106, 0x08, 0x8E);
    setidt(128, (unsigned long)isr128, 0x08, 0x8E);
}

void remap_irq(void)
{
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x00);
    outb(0xA1, 0x00); 
}

void set_isr(unsigned char int_n)
{
	if (int_n == 48)
	{
		setidt(48, (unsigned long)isr48, 0x08, 0x8E);
	}
	if (int_n == 49)
	{
		setidt(49, (unsigned long)isr49, 0x08, 0x8E);
	}
	if (int_n == 50)
	{
		setidt(50, (unsigned long)isr50, 0x08, 0x8E);
	}
	if (int_n == 51)
	{
		setidt(51, (unsigned long)isr51, 0x08, 0x8E);
	}
	if (int_n == 52)
	{
		setidt(52, (unsigned long)isr52, 0x08, 0x8E);
	}
	if (int_n == 53)
	{
		setidt(53, (unsigned long)isr53, 0x08, 0x8E);
	}
	if (int_n == 54)
	{
		setidt(54, (unsigned long)isr54, 0x08, 0x8E);
	}
	if (int_n == 55)
	{
		setidt(55, (unsigned long)isr55, 0x08, 0x8E);
	}
	if (int_n == 56)
	{
		setidt(56, (unsigned long)isr56, 0x08, 0x8E);
	}
	if (int_n == 57)
	{
		setidt(57, (unsigned long)isr57, 0x08, 0x8E);
	}
	if (int_n == 58)
	{
		setidt(58, (unsigned long)isr58, 0x08, 0x8E);
	}
	if (int_n == 59)
	{
		setidt(59, (unsigned long)isr59, 0x08, 0x8E);
	}
	if (int_n == 60)
	{
		setidt(60, (unsigned long)isr60, 0x08, 0x8E);
	}
	if (int_n == 61)
	{
		setidt(61, (unsigned long)isr61, 0x08, 0x8E);
	}
	if (int_n == 62)
	{
		setidt(62, (unsigned long)isr62, 0x08, 0x8E);
	}
	if (int_n == 63)
	{
		setidt(63, (unsigned long)isr63, 0x08, 0x8E);
	}
	if (int_n == 64)
	{
		setidt(64, (unsigned long)isr64, 0x08, 0x8E);
	}
	if (int_n == 65)
	{
		setidt(65, (unsigned long)isr65, 0x08, 0x8E);
	}
	if (int_n == 66)
	{
		setidt(66, (unsigned long)isr66, 0x08, 0x8E);
	}
	if (int_n == 67)
	{
		setidt(67, (unsigned long)isr67, 0x08, 0x8E);
	}
	if (int_n == 68)
	{
		setidt(68, (unsigned long)isr68, 0x08, 0x8E);
	}
	if (int_n == 69)
	{
		setidt(69, (unsigned long)isr69, 0x08, 0x8E);
	}
	if (int_n == 70)
	{
		setidt(70, (unsigned long)isr70, 0x08, 0x8E);
	}
	if (int_n == 71)
	{
		setidt(71, (unsigned long)isr71, 0x08, 0x8E);
	}
	if (int_n == 72)
	{
		setidt(72, (unsigned long)isr72, 0x08, 0x8E);
	}
	if (int_n == 73)
	{
		setidt(73, (unsigned long)isr73, 0x08, 0x8E);
	}
	if (int_n == 74)
	{
		setidt(74, (unsigned long)isr74, 0x08, 0x8E);
	}
	if (int_n == 75)
	{
		setidt(75, (unsigned long)isr75, 0x08, 0x8E);
	}
	if (int_n == 76)
	{
		setidt(76, (unsigned long)isr76, 0x08, 0x8E);
	}
	if (int_n == 77)
	{
		setidt(77, (unsigned long)isr77, 0x08, 0x8E);
	}
	if (int_n == 78)
	{
		setidt(78, (unsigned long)isr78, 0x08, 0x8E);
	}
	if (int_n == 79)
	{
		setidt(79, (unsigned long)isr79, 0x08, 0x8E);
	}
	if (int_n == 80)
	{
		setidt(80, (unsigned long)isr80, 0x08, 0x8E);
	}
	if (int_n == 81)
	{
		setidt(81, (unsigned long)isr81, 0x08, 0x8E);
	}
	if (int_n == 82)
	{
		setidt(82, (unsigned long)isr82, 0x08, 0x8E);
	}
	if (int_n == 83)
	{
		setidt(83, (unsigned long)isr83, 0x08, 0x8E);
	}
	if (int_n == 84)
	{
		setidt(84, (unsigned long)isr84, 0x08, 0x8E);
	}
	if (int_n == 85)
	{
		setidt(85, (unsigned long)isr85, 0x08, 0x8E);
	}
	if (int_n == 86)
	{
		setidt(86, (unsigned long)isr86, 0x08, 0x8E);
	}
	if (int_n == 87)
	{
		setidt(87, (unsigned long)isr87, 0x08, 0x8E);
	}
	if (int_n == 88)
	{
		setidt(88, (unsigned long)isr88, 0x08, 0x8E);
	}
	if (int_n == 89)
	{
		setidt(89, (unsigned long)isr89, 0x08, 0x8E);
	}
	if (int_n == 90)
	{
		setidt(90, (unsigned long)isr90, 0x08, 0x8E);
	}
	if (int_n == 91)
	{
		setidt(91, (unsigned long)isr91, 0x08, 0x8E);
	}
	if (int_n == 92)
	{
		setidt(92, (unsigned long)isr92, 0x08, 0x8E);
	}
	if (int_n == 93)
	{
		setidt(93, (unsigned long)isr93, 0x08, 0x8E);
	}
	if (int_n == 94)
	{
		setidt(94, (unsigned long)isr94, 0x08, 0x8E);
	}
	if (int_n == 95)
	{
		setidt(95, (unsigned long)isr95, 0x08, 0x8E);
	}
	if (int_n == 96)
	{
		setidt(96, (unsigned long)isr96, 0x08, 0x8E);
	}
	if (int_n == 97)
	{
		setidt(97, (unsigned long)isr97, 0x08, 0x8E);
	}
	if (int_n == 98)
	{
		setidt(98, (unsigned long)isr98, 0x08, 0x8E);
	}
	if (int_n == 99)
	{
		setidt(99, (unsigned long)isr99, 0x08, 0x8E);
	}
	if (int_n == 100)
	{
		setidt(100, (unsigned long)isr100, 0x08, 0x8E);
	}
	if (int_n == 101)
	{
		setidt(101, (unsigned long)isr101, 0x08, 0x8E);
	}
	if (int_n == 102)
	{
		setidt(102, (unsigned long)isr102, 0x08, 0x8E);
	}
	if (int_n == 103)
	{
		setidt(103, (unsigned long)isr103, 0x08, 0x8E);
	}
	if (int_n == 104)
	{
		setidt(104, (unsigned long)isr104, 0x08, 0x8E);
	}
	if (int_n == 105)
	{
		setidt(105, (unsigned long)isr105, 0x08, 0x8E);
	}
	if (int_n == 106)
	{
		setidt(106, (unsigned long)isr106, 0x08, 0x8E);
	}
	if (int_n == 107)
	{
		setidt(107, (unsigned long)isr107, 0x08, 0x8E);
	}
	if (int_n == 108)
	{
		setidt(108, (unsigned long)isr108, 0x08, 0x8E);
	}
	if (int_n == 109)
	{
		setidt(109, (unsigned long)isr109, 0x08, 0x8E);
	}
	if (int_n == 110)
	{
		setidt(110, (unsigned long)isr110, 0x08, 0x8E);
	}
	if (int_n == 111)
	{
		setidt(111, (unsigned long)isr111, 0x08, 0x8E);
	}
	if (int_n == 112)
	{
		setidt(112, (unsigned long)isr112, 0x08, 0x8E);
	}
	if (int_n == 113)
	{
		setidt(113, (unsigned long)isr113, 0x08, 0x8E);
	}
	if (int_n == 114)
	{
		setidt(114, (unsigned long)isr114, 0x08, 0x8E);
	}
	if (int_n == 115)
	{
		setidt(115, (unsigned long)isr115, 0x08, 0x8E);
	}
	if (int_n == 116)
	{
		setidt(116, (unsigned long)isr116, 0x08, 0x8E);
	}
	if (int_n == 117)
	{
		setidt(117, (unsigned long)isr117, 0x08, 0x8E);
	}
	if (int_n == 118)
	{
		setidt(118, (unsigned long)isr118, 0x08, 0x8E);
	}
	if (int_n == 119)
	{
		setidt(119, (unsigned long)isr119, 0x08, 0x8E);
	}
	if (int_n == 120)
	{
		setidt(120, (unsigned long)isr120, 0x08, 0x8E);
	}
	if (int_n == 121)
	{
		setidt(121, (unsigned long)isr121, 0x08, 0x8E);
	}
	if (int_n == 122)
	{
		setidt(122, (unsigned long)isr122, 0x08, 0x8E);
	}
	if (int_n == 123)
	{
		setidt(123, (unsigned long)isr123, 0x08, 0x8E);
	}
	if (int_n == 124)
	{
		setidt(124, (unsigned long)isr124, 0x08, 0x8E);
	}
	if (int_n == 125)
	{
		setidt(125, (unsigned long)isr125, 0x08, 0x8E);
	}
	if (int_n == 126)
	{
		setidt(126, (unsigned long)isr126, 0x08, 0x8E);
	}
	if (int_n == 127)
	{
		setidt(127, (unsigned long)isr127, 0x08, 0x8E);
	}
	if (int_n == 128)
	{
		setidt(128, (unsigned long)isr128, 0x08, 0x8E);
	}
	if (int_n == 129)
	{
		setidt(129, (unsigned long)isr129, 0x08, 0x8E);
	}
	if (int_n == 130)
	{
		setidt(130, (unsigned long)isr130, 0x08, 0x8E);
	}
	if (int_n == 131)
	{
		setidt(131, (unsigned long)isr131, 0x08, 0x8E);
	}
	if (int_n == 132)
	{
		setidt(132, (unsigned long)isr132, 0x08, 0x8E);
	}
	if (int_n == 133)
	{
		setidt(133, (unsigned long)isr133, 0x08, 0x8E);
	}
	if (int_n == 134)
	{
		setidt(134, (unsigned long)isr134, 0x08, 0x8E);
	}
	if (int_n == 135)
	{
		setidt(135, (unsigned long)isr135, 0x08, 0x8E);
	}
	if (int_n == 136)
	{
		setidt(136, (unsigned long)isr136, 0x08, 0x8E);
	}
	if (int_n == 137)
	{
		setidt(137, (unsigned long)isr137, 0x08, 0x8E);
	}
	if (int_n == 138)
	{
		setidt(138, (unsigned long)isr138, 0x08, 0x8E);
	}
	if (int_n == 139)
	{
		setidt(139, (unsigned long)isr139, 0x08, 0x8E);
	}
	if (int_n == 140)
	{
		setidt(140, (unsigned long)isr140, 0x08, 0x8E);
	}
	if (int_n == 141)
	{
		setidt(141, (unsigned long)isr141, 0x08, 0x8E);
	}
	if (int_n == 142)
	{
		setidt(142, (unsigned long)isr142, 0x08, 0x8E);
	}
	if (int_n == 143)
	{
		setidt(143, (unsigned long)isr143, 0x08, 0x8E);
	}
	if (int_n == 144)
	{
		setidt(144, (unsigned long)isr144, 0x08, 0x8E);
	}
	if (int_n == 145)
	{
		setidt(145, (unsigned long)isr145, 0x08, 0x8E);
	}
	if (int_n == 146)
	{
		setidt(146, (unsigned long)isr146, 0x08, 0x8E);
	}
	if (int_n == 147)
	{
		setidt(147, (unsigned long)isr147, 0x08, 0x8E);
	}
	if (int_n == 148)
	{
		setidt(148, (unsigned long)isr148, 0x08, 0x8E);
	}
	if (int_n == 149)
	{
		setidt(149, (unsigned long)isr149, 0x08, 0x8E);
	}
	if (int_n == 150)
	{
		setidt(150, (unsigned long)isr150, 0x08, 0x8E);
	}
	if (int_n == 151)
	{
		setidt(151, (unsigned long)isr151, 0x08, 0x8E);
	}
	if (int_n == 152)
	{
		setidt(152, (unsigned long)isr152, 0x08, 0x8E);
	}
	if (int_n == 153)
	{
		setidt(153, (unsigned long)isr153, 0x08, 0x8E);
	}
	if (int_n == 154)
	{
		setidt(154, (unsigned long)isr154, 0x08, 0x8E);
	}
	if (int_n == 155)
	{
		setidt(155, (unsigned long)isr155, 0x08, 0x8E);
	}
	if (int_n == 156)
	{
		setidt(156, (unsigned long)isr156, 0x08, 0x8E);
	}
	if (int_n == 157)
	{
		setidt(157, (unsigned long)isr157, 0x08, 0x8E);
	}
	if (int_n == 158)
	{
		setidt(158, (unsigned long)isr158, 0x08, 0x8E);
	}
	if (int_n == 159)
	{
		setidt(159, (unsigned long)isr159, 0x08, 0x8E);
	}
	if (int_n == 160)
	{
		setidt(160, (unsigned long)isr160, 0x08, 0x8E);
	}
	if (int_n == 161)
	{
		setidt(161, (unsigned long)isr161, 0x08, 0x8E);
	}
	if (int_n == 162)
	{
		setidt(162, (unsigned long)isr162, 0x08, 0x8E);
	}
	if (int_n == 163)
	{
		setidt(163, (unsigned long)isr163, 0x08, 0x8E);
	}
	if (int_n == 164)
	{
		setidt(164, (unsigned long)isr164, 0x08, 0x8E);
	}
	if (int_n == 165)
	{
		setidt(165, (unsigned long)isr165, 0x08, 0x8E);
	}
	if (int_n == 166)
	{
		setidt(166, (unsigned long)isr166, 0x08, 0x8E);
	}
	if (int_n == 167)
	{
		setidt(167, (unsigned long)isr167, 0x08, 0x8E);
	}
	if (int_n == 168)
	{
		setidt(168, (unsigned long)isr168, 0x08, 0x8E);
	}
	if (int_n == 169)
	{
		setidt(169, (unsigned long)isr169, 0x08, 0x8E);
	}
	if (int_n == 170)
	{
		setidt(170, (unsigned long)isr170, 0x08, 0x8E);
	}
	if (int_n == 171)
	{
		setidt(171, (unsigned long)isr171, 0x08, 0x8E);
	}
	if (int_n == 172)
	{
		setidt(172, (unsigned long)isr172, 0x08, 0x8E);
	}
	if (int_n == 173)
	{
		setidt(173, (unsigned long)isr173, 0x08, 0x8E);
	}
	if (int_n == 174)
	{
		setidt(174, (unsigned long)isr174, 0x08, 0x8E);
	}
	if (int_n == 175)
	{
		setidt(175, (unsigned long)isr175, 0x08, 0x8E);
	}
	if (int_n == 176)
	{
		setidt(176, (unsigned long)isr176, 0x08, 0x8E);
	}
	if (int_n == 177)
	{
		setidt(177, (unsigned long)isr177, 0x08, 0x8E);
	}
	if (int_n == 178)
	{
		setidt(178, (unsigned long)isr178, 0x08, 0x8E);
	}
	if (int_n == 179)
	{
		setidt(179, (unsigned long)isr179, 0x08, 0x8E);
	}
	if (int_n == 180)
	{
		setidt(180, (unsigned long)isr180, 0x08, 0x8E);
	}
	if (int_n == 181)
	{
		setidt(181, (unsigned long)isr181, 0x08, 0x8E);
	}
	if (int_n == 182)
	{
		setidt(182, (unsigned long)isr182, 0x08, 0x8E);
	}
	if (int_n == 183)
	{
		setidt(183, (unsigned long)isr183, 0x08, 0x8E);
	}
	if (int_n == 184)
	{
		setidt(184, (unsigned long)isr184, 0x08, 0x8E);
	}
	if (int_n == 185)
	{
		setidt(185, (unsigned long)isr185, 0x08, 0x8E);
	}
	if (int_n == 186)
	{
		setidt(186, (unsigned long)isr186, 0x08, 0x8E);
	}
	if (int_n == 187)
	{
		setidt(187, (unsigned long)isr187, 0x08, 0x8E);
	}
	if (int_n == 188)
	{
		setidt(188, (unsigned long)isr188, 0x08, 0x8E);
	}
	if (int_n == 189)
	{
		setidt(189, (unsigned long)isr189, 0x08, 0x8E);
	}
	if (int_n == 190)
	{
		setidt(190, (unsigned long)isr190, 0x08, 0x8E);
	}
	if (int_n == 191)
	{
		setidt(191, (unsigned long)isr191, 0x08, 0x8E);
	}
	if (int_n == 192)
	{
		setidt(192, (unsigned long)isr192, 0x08, 0x8E);
	}
	if (int_n == 193)
	{
		setidt(193, (unsigned long)isr193, 0x08, 0x8E);
	}
	if (int_n == 194)
	{
		setidt(194, (unsigned long)isr194, 0x08, 0x8E);
	}
	if (int_n == 195)
	{
		setidt(195, (unsigned long)isr195, 0x08, 0x8E);
	}
	if (int_n == 196)
	{
		setidt(196, (unsigned long)isr196, 0x08, 0x8E);
	}
	if (int_n == 197)
	{
		setidt(197, (unsigned long)isr197, 0x08, 0x8E);
	}
	if (int_n == 198)
	{
		setidt(198, (unsigned long)isr198, 0x08, 0x8E);
	}
	if (int_n == 199)
	{
		setidt(199, (unsigned long)isr199, 0x08, 0x8E);
	}
	if (int_n == 200)
	{
		setidt(200, (unsigned long)isr200, 0x08, 0x8E);
	}
	if (int_n == 201)
	{
		setidt(201, (unsigned long)isr201, 0x08, 0x8E);
	}
	if (int_n == 202)
	{
		setidt(202, (unsigned long)isr202, 0x08, 0x8E);
	}
	if (int_n == 203)
	{
		setidt(203, (unsigned long)isr203, 0x08, 0x8E);
	}
	if (int_n == 204)
	{
		setidt(204, (unsigned long)isr204, 0x08, 0x8E);
	}
	if (int_n == 205)
	{
		setidt(205, (unsigned long)isr205, 0x08, 0x8E);
	}
	if (int_n == 206)
	{
		setidt(206, (unsigned long)isr206, 0x08, 0x8E);
	}
	if (int_n == 207)
	{
		setidt(207, (unsigned long)isr207, 0x08, 0x8E);
	}
	if (int_n == 208)
	{
		setidt(208, (unsigned long)isr208, 0x08, 0x8E);
	}
	if (int_n == 209)
	{
		setidt(209, (unsigned long)isr209, 0x08, 0x8E);
	}
	if (int_n == 210)
	{
		setidt(210, (unsigned long)isr210, 0x08, 0x8E);
	}
	if (int_n == 211)
	{
		setidt(211, (unsigned long)isr211, 0x08, 0x8E);
	}
	if (int_n == 212)
	{
		setidt(212, (unsigned long)isr212, 0x08, 0x8E);
	}
	if (int_n == 213)
	{
		setidt(213, (unsigned long)isr213, 0x08, 0x8E);
	}
	if (int_n == 214)
	{
		setidt(214, (unsigned long)isr214, 0x08, 0x8E);
	}
	if (int_n == 215)
	{
		setidt(215, (unsigned long)isr215, 0x08, 0x8E);
	}
	if (int_n == 216)
	{
		setidt(216, (unsigned long)isr216, 0x08, 0x8E);
	}
	if (int_n == 217)
	{
		setidt(217, (unsigned long)isr217, 0x08, 0x8E);
	}
	if (int_n == 218)
	{
		setidt(218, (unsigned long)isr218, 0x08, 0x8E);
	}
	if (int_n == 219)
	{
		setidt(219, (unsigned long)isr219, 0x08, 0x8E);
	}
	if (int_n == 220)
	{
		setidt(220, (unsigned long)isr220, 0x08, 0x8E);
	}
	if (int_n == 221)
	{
		setidt(221, (unsigned long)isr221, 0x08, 0x8E);
	}
	if (int_n == 222)
	{
		setidt(222, (unsigned long)isr222, 0x08, 0x8E);
	}
	if (int_n == 223)
	{
		setidt(223, (unsigned long)isr223, 0x08, 0x8E);
	}
	if (int_n == 224)
	{
		setidt(224, (unsigned long)isr224, 0x08, 0x8E);
	}
	if (int_n == 225)
	{
		setidt(225, (unsigned long)isr225, 0x08, 0x8E);
	}
	if (int_n == 226)
	{
		setidt(226, (unsigned long)isr226, 0x08, 0x8E);
	}
	if (int_n == 227)
	{
		setidt(227, (unsigned long)isr227, 0x08, 0x8E);
	}
	if (int_n == 228)
	{
		setidt(228, (unsigned long)isr228, 0x08, 0x8E);
	}
	if (int_n == 229)
	{
		setidt(229, (unsigned long)isr229, 0x08, 0x8E);
	}
	if (int_n == 230)
	{
		setidt(230, (unsigned long)isr230, 0x08, 0x8E);
	}
	if (int_n == 231)
	{
		setidt(231, (unsigned long)isr231, 0x08, 0x8E);
	}
	if (int_n == 232)
	{
		setidt(232, (unsigned long)isr232, 0x08, 0x8E);
	}
	if (int_n == 233)
	{
		setidt(233, (unsigned long)isr233, 0x08, 0x8E);
	}
	if (int_n == 234)
	{
		setidt(234, (unsigned long)isr234, 0x08, 0x8E);
	}
	if (int_n == 235)
	{
		setidt(235, (unsigned long)isr235, 0x08, 0x8E);
	}
	if (int_n == 236)
	{
		setidt(236, (unsigned long)isr236, 0x08, 0x8E);
	}
	if (int_n == 237)
	{
		setidt(237, (unsigned long)isr237, 0x08, 0x8E);
	}
	if (int_n == 238)
	{
		setidt(238, (unsigned long)isr238, 0x08, 0x8E);
	}
	if (int_n == 239)
	{
		setidt(239, (unsigned long)isr239, 0x08, 0x8E);
	}
	if (int_n == 240)
	{
		setidt(240, (unsigned long)isr240, 0x08, 0x8E);
	}
	if (int_n == 241)
	{
		setidt(241, (unsigned long)isr241, 0x08, 0x8E);
	}
	if (int_n == 242)
	{
		setidt(242, (unsigned long)isr242, 0x08, 0x8E);
	}
	if (int_n == 243)
	{
		setidt(243, (unsigned long)isr243, 0x08, 0x8E);
	}
	if (int_n == 244)
	{
		setidt(244, (unsigned long)isr244, 0x08, 0x8E);
	}
	if (int_n == 245)
	{
		setidt(245, (unsigned long)isr245, 0x08, 0x8E);
	}
	if (int_n == 246)
	{
		setidt(246, (unsigned long)isr246, 0x08, 0x8E);
	}
	if (int_n == 247)
	{
		setidt(247, (unsigned long)isr247, 0x08, 0x8E);
	}
	if (int_n == 248)
	{
		setidt(248, (unsigned long)isr248, 0x08, 0x8E);
	}
	if (int_n == 249)
	{
		setidt(249, (unsigned long)isr249, 0x08, 0x8E);
	}
	if (int_n == 250)
	{
		setidt(250, (unsigned long)isr250, 0x08, 0x8E);
	}
	if (int_n == 251)
	{
		setidt(251, (unsigned long)isr251, 0x08, 0x8E);
	}
	if (int_n == 252)
	{
		setidt(252, (unsigned long)isr252, 0x08, 0x8E);
	}
	if (int_n == 253)
	{
		setidt(253, (unsigned long)isr253, 0x08, 0x8E);
	}
	if (int_n == 254)
	{
		setidt(254, (unsigned long)isr254, 0x08, 0x8E);
	}
	if (int_n == 255)
	{
		setidt(255, (unsigned long)isr255, 0x08, 0x8E);
	}	
}

void set_irq(unsigned char n, unsigned long base)
{
	setidt(32+n, (unsigned long)base, 0x08, 0x8E);
}

void loadirq(void)
{
	remap_irq();
    set_irq(0, (unsigned long)irq0);
    set_irq(1, (unsigned long)irq1);
    set_irq(2, (unsigned long)irq2);
    set_irq(3, (unsigned long)irq3);
    set_irq(4, (unsigned long)irq4);
    set_irq(5, (unsigned long)irq5);
    set_irq(6, (unsigned long)irq6);
    set_irq(7, (unsigned long)irq7);
    set_irq(8, (unsigned long)irq8);
    set_irq(9, (unsigned long)irq9);
    set_irq(10, (unsigned long)irq10);
    set_irq(11, (unsigned long)irq11);
    set_irq(12, (unsigned long)irq12);
    set_irq(13, (unsigned long)irq13);
    set_irq(14, (unsigned long)irq14);
    set_irq(15, (unsigned long)irq15);
}

void register_interrupt_handler(unsigned char n, isr_t handler)
{
	interrupt_handlers[n] = handler;
}

void irq_ack(unsigned char irq) {
    if(irq >= 0x28)
        outb(0xA0, 0x20);
    outb(0x20, 0x20);
}

void irq_handler(registers_t *registers)
{
	/*
	if (registers->int_no >= 40)
	{
		outb(0xA0, 0x20);
	}
	outb(0x20, 0x20);
	*/
	irq_ack(registers->int_no);

	if (interrupt_handlers[registers->int_no] != 0)
	{
		isr_t handler = interrupt_handlers[registers->int_no];
		handler(registers);
	}
	outb(0x20, 0x20);
}

volatile unsigned long timertick = 0;

void timer_handler(registers_t *registers)
{
	timertick++;
}

void reset_timer(void)
{
	timertick = 0;
}

void init_timer(unsigned long frequency)
{
	unsigned long d;
	unsigned char h;
	unsigned char l;
	register_interrupt_handler(IRQ0, timer_handler);
	d = (1193180/frequency);
	l = (unsigned char)(d&0xFF);
	h = (unsigned char)((d>>8)&0xFF);
	disable_interrupt();
	outb(0x43, 0x36);
	outb(0x40, l);
	outb(0x40, h);
	enable_interrupt();
}

volatile unsigned char key_code = 0;
volatile unsigned char key_status = 0;
volatile unsigned char key_shift = 0;
volatile unsigned char kb_scan = 0;

volatile unsigned char tmp_key_code = 0;
volatile unsigned char tmp_key_status = 0;
volatile unsigned char tmp_key_shift = 0;
volatile unsigned char tmp_scancode = 0;

#define KB_LED_SCROLL 1
#define KB_LED_NUMPAD 2
#define KB_LED_CAPSLK 4


unsigned short key_handlers_count = 0;

typedef void (*key_handler_t)(unsigned char, unsigned char, unsigned char, unsigned char);
void register_key_handler(key_handler_t handler);

void key_handler(unsigned char key, unsigned char status, unsigned char shift, unsigned char scancode);

key_handler_t key_handlers[MAXKEYSHANDLERS];

void keyboard_restart(void)
{
	if ((enum_loaded) && (total_enum > 0) && (has_enum(SYSTEM_STDIO_SERIAL))) return;
	unsigned char d = inb(0x61);
	outb(0x61, (d | 0x80));
	outb(0x61, (d & 0x7F));
}

void set_keyboard_led(int n, int c, int s)
{
	unsigned char t = 0;
	if ((enum_loaded) && (total_enum > 0) && (has_enum(SYSTEM_STDIO_SERIAL))) return;
	t = (s) ? (t | KB_LED_SCROLL) : (t & KB_LED_SCROLL);
	t = (n) ? (t | KB_LED_NUMPAD) : (t & KB_LED_NUMPAD);
	t = (c) ? (t | KB_LED_CAPSLK) : (t & KB_LED_CAPSLK);
	while((inb(0x64) & 2) != 0);
	outb(0x60, 0xED);
	while((inb(0x64) & 2) != 0);
	outb(0x60, t);
}

int init_serial(void)
{
	outb(0x3F9, 0x00);
	outb(0x3FB, 0x80);
	outb(0x3F8, 0x00);
	outb(0x3F9, 0x00);
	outb(0x3FB, 0x03);
	outb(0x3FA, 0xC7);
	outb(0x3FC, 0x08);
	outb(0x3FC, 0x1E);
	outb(0x3F8, 0xAE);
	if (inb(0x3F8)!=0xAE)
	{
		return 1;
	}
	outb(0x3FC, 0x0F);
	return 0;
}

unsigned char serial_recv(void)
{
	return (inb(0x3F8+5)&0x01);
}

unsigned char get_serial_code(void)
{
	while (serial_recv()==0);
	return inb(0x3F8);
}

unsigned char get_key_code(void)
{
	if ((enum_loaded) && (total_enum > 0) && (has_enum(SYSTEM_STDIO_SERIAL))) 
	{		
		return get_serial_code();
	}
	return inb(0x60);
}

unsigned char get_key_status(void)
{
	if ((enum_loaded) && (total_enum > 0) && (has_enum(SYSTEM_STDIO_SERIAL))) 
	{
		return 0x01;
	}
	return inb(0x64);
}

void keyboard_handler(registers_t *registers)
{
	unsigned char scan_code = 0;
/*
   // Non-Shifted:
   static const unsigned char asciiNonSh[] = { NULL, 0x1B, '1', '2', '3', '4', '5', '6', '7', '8', '9',
   '0', '-', '=', '\b', '\t', 'q', 'w',   'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',   '[', ']', '\n',   0, 'a',
   's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm',
   ',', '.', '/', 0, 0, 0, ' ',   0, KF1,   KF2, KF3, KF4, KF5, KF6, KF7, KF8, KF9,   KF10, 0, 0, KHOME, KUP,
   KPGUP,'-', KLFT, '5',   KRT, '+', KEND, KDN, KPGDN, KINS, KDEL, 0, 0, 0, KF11, KF12 };

   // Shifted:
   static const unsigned char asciiShift[] = { NULL, 0x1B, '!', '@', '#', '$', '%', '^', '&', '*', '(',
   ')', '_', '+', '\b', '\t', 'Q', 'W',   'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',   '{', '}', '\n',   0, 'A',
   'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~', 0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<',
   '>', '?', 0, 0, 0, ' ',   0, KF1,   KF2, KF3, KF4, KF5, KF6, KF7, KF8, KF9,   KF10, 0, 0, KHOME, KUP,   KPGUP,
   '-', KLFT, '5',   KRT, '+', KEND, KDN, KPGDN, KINS, KDEL, 0, 0, 0, KF11, KF12 };
*/
   // Non-Shifted:
   static const unsigned char asciiNonSh[] = { NULL, 0x1B, '1', '2', '3', '4', '5', '6', '7', '8', '9',
   '0', '-', '=', '\b', '\t', 'q', 'w',   'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',   '[', ']', '\n',   0, 'a',
   's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm',
   ',', '.', '/', 0, 0, 0, ' ',   0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0,
   0,'-', 0, '5',   0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

   // Shifted:
   static const unsigned char asciiShift[] = { NULL, 0x1B, '!', '@', '#', '$', '%', '^', '&', '*', '(',
   ')', '_', '+', '\b', '\t', 'Q', 'W',   'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',   '{', '}', '\n',   0, 'A',
   'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~', 0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<',
   '>', '?', 0, 0, 0, ' ',   0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0,   0,
   '-', 0, '5',   0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	if ((enum_loaded) && (total_enum > 0) && (has_enum(SYSTEM_STDIO_SERIAL))) 
	{
		return;
	}
	
	kb_scan = 1;
	
	key_status = get_key_status();
	if (key_status & 0x01)
	{
		scan_code = get_key_code();
		if (scan_code == 0x2A)
		{
			key_shift = 1;
		}
		else
		if (scan_code == 0xAA)
		{
			key_shift = 0;
		}
		else
		if (scan_code & 0x80)
		{
			// skip it
		}
		else
		{
			if (key_shift == 1)
			{
				key_code = asciiShift[scan_code];
			}
			else
			{
				key_code = asciiNonSh[scan_code];
			}
		}
		tmp_key_code = key_code;
		tmp_key_status = key_status;
		tmp_key_shift = key_shift;
		tmp_scancode = scan_code;
		if ((tmp_key_code != 0) || (tmp_key_status != 0) || (tmp_key_shift != 0) || (tmp_scancode != 0))
		{
			key_handler(tmp_key_code, tmp_key_status, tmp_key_shift, tmp_scancode);
		}
		tmp_key_code = 0;
		tmp_key_status = 0;
		tmp_key_shift = 0;
	}
	else
	{
		key_code = 0;
	}
}

void keyboard_flush(void)
{
	registers_t r;
	keyboard_handler(&r);
}

void register_key_handler(key_handler_t handler)
{
	if (key_handlers_count >= MAXKEYSHANDLERS) return;
	key_handlers[key_handlers_count] = handler;
	key_handlers_count++;
}

void key_handler(unsigned char key, unsigned char status, unsigned char shift, unsigned char scancode)
{
	int i=0;
	if ((enum_loaded) && (total_enum > 0) && (has_enum(SYSTEM_STDIO_SERIAL))) 
	{
		return;
	}
	for(i=0;i<key_handlers_count;i++)
	{
		if (key_handlers[i] != 0)
		{
			key_handler_t handler = key_handlers[i];
			handler(key, status, shift, scancode);
		}
	}
}

void init_keyboard(void)
{	
	if ((enum_loaded) && (total_enum > 0) && (has_enum(SYSTEM_STDIO_SERIAL))) 
	{
		memset(key_handlers, 0, MAXKEYSHANDLERS * sizeof(key_handler_t));
		register_interrupt_handler(IRQ1, 0);		
		// register_interrupt_handler(IRQ4, keyboard_handler);	
		init_serial();
		return;
	}
	else
	{
		memset(key_handlers, 0, MAXKEYSHANDLERS * sizeof(key_handler_t));
		register_interrupt_handler(IRQ1, keyboard_handler);
	}
	msleep(10);
	set_keyboard_led(1,1,1);
	msleep(10);
	keyboard_restart();
	msleep(10);
	if((inb(0x64) & 2) != 0) 
	{
		outb(0x60, 0xED);
		outb(0x60, 0x04);
	}
	msleep(10);
	set_keyboard_led(1,0,0);
	msleep(10);
}

void msleep(unsigned int milliseconds)
{
	unsigned long ms;
	ms = timertick + ((timer_frequency * milliseconds) / 1000);
	while (timertick < ms);
}

void sleep(int seconds)
{
	unsigned long ms;
	ms = timertick + (seconds * timer_frequency);
	while (timertick < ms);
}

char getchar(void)
{
	int kb_out = 0;
	char c;
	while(kb_out == 0)
	{
		if ((enum_loaded) && (total_enum > 0) && (has_enum(SYSTEM_STDIO_SERIAL))) 
		{
			c = get_serial_code();
			if (c != 0x00) 
			{
				key_code = c;
				key_status = 0;
				kb_scan = 0;
				kb_out = 1;
			}
		}
		else
		{
			if (key_status & 0x01)
			{
				if (key_code != 0x00)
				{
					c = key_code;
					key_status = 0;
					kb_scan = 0;
					key_code = 0;
					kb_out = 1;
				}
			}
		}
	}
	return c;
}

char getch(void)
{
	return getchar();
}

char *gets(char *s)
{
	int kb_out = 0;
	unsigned char kb_char = 0;
	char *s1 = s;
	int char_x=0;
	int l=strlen(s);
	if (l > 0)
	{
		memset(s1, 0, l);
	}
	while(kb_out == 0)
	{
		kb_char = getch();
		if (kb_char != 0)
		{
			if ((enum_loaded) && (total_enum > 0) && (has_enum(SYSTEM_STDIO_SERIAL))) 
			{
				if (kb_char == '\b')
				{
					if (char_x > 0)
					{
						putch(' ');
						putch('\b');
						char_x--;
						s1[char_x] = '\0';
						key_code = 0;
						key_status = 0;
						kb_scan = 0;		
					}
				}
				else if (kb_char < 0x08)
				{
					s1[char_x] = '\0';
					key_code = 0;
					key_status = 0;
					kb_scan = 0;					
					if (char_x == 0)
					{
						s1[0] = 0;
					}
				}
				else if (kb_char > 0xFA)
				{
					s1[char_x] = '\0';
					key_code = 0;
					key_status = 0;
					kb_scan = 0;					
					if (char_x == 0)
					{
						s1[0] = 0;
					}
				}
				else if (kb_char == '\r')
				{
					s1[char_x] = '\0';
					char_x++;
					key_code = 0;
					key_status = 0;
					kb_scan = 0;					
				}
				else if (kb_char == '\n')
				{
					s1[char_x] = '\0';
					key_code = 0;
					key_status = 0;
					kb_scan = 0;
					kb_out = 1;
				}
				else
				{
					s1[char_x] = kb_char;
					char_x++;
					s1[char_x] = '\0';		
					key_code = 0;
					key_status = 0;
					kb_scan = 0;
				}	
			}
			else
			{
				if (kb_char == '\b')
				{
					if (char_x > 0)
					{
						putch(kb_char);
						putch(' ');
						putch('\b');
						char_x--;
						s1[char_x] = '\0';
						key_code = 0;
						key_status = 0;
						kb_scan = 0;		
					}
				}
				else if (kb_char == '\n')
				{
					if (char_x > 0)
					{
						putch('\n');
					}
					s1[char_x] = '\0';
					key_code = 0;
					key_status = 0;
					kb_scan = 0;
					kb_out = 1;
				}
				else
				{
					s1[char_x] = kb_char;
					putch(s1[char_x]);
					char_x++;
					s1[char_x] = '\0';		
					key_code = 0;
					key_status = 0;
					kb_scan = 0;
				}				
			}
		}
	}	
	return s1;
}

void cpuid(int c, unsigned long *a, unsigned long *d)
{
	__asm__ volatile ("cpuid"
				  :"=a"(*a),
				   "=d"(*d)
				  :"a"(c)
				  :"ecx",
				  "ebx");
}

int getcpuid(int c, unsigned long d[4])
{
	__asm__ volatile ("cpuid"
				  :"=a"(*d),
				   "=b"(*(d+1)),
				   "=c"(*(d+2)),
				   "=d"(*(d+3))
				  :"a"(c));				  
	return (int)d[0];
}

void dwordtobytes(unsigned long dw, unsigned char bytes[4])
{
    unsigned char *c;
    c = (unsigned char*)&dw;
    bytes[0] = c[3];
    bytes[1] = c[2];
    bytes[2] = c[1];
    bytes[3] = c[0];
    return;
}

unsigned long swap32( unsigned long value )
{
    value = ((value << 8) & 0xFF00FF00 ) | ((value >> 8) & 0xFF00FF ); 
    return (value << 16) | (value >> 16);
}

unsigned char has_cpu_support_longmode(void)
{
	unsigned long d[4];
	unsigned char has_longmode = 0;
	getcpuid(0x80000001, d);
	has_longmode = ((d[3] >> 29) & 0x01);
	return has_longmode;
}

void get_cpu_vendor_string(char *s)
{
  unsigned long d[4];
  union
  {
	char c[16];
	int  i[4];
  } v;
  getcpuid(0, d);
  *(&v.i[0]) = d[1];
  *(&v.i[1]) = d[3];
  *(&v.i[2]) = d[2];
  v.c[12] = '\0';
  strcpy(s, v.c);
}


int get_cpu_vendor(void)
{
	char vendor[16];
	unsigned long d[4];
	union
	{
	char c[16];
	int  i[4];
	} v;
	getcpuid(0, d);
	*(&v.i[0]) = d[1];
	*(&v.i[1]) = d[3];
	*(&v.i[2]) = d[2];
	v.c[12] = '\0';
	strcpy(vendor, v.c);
	if (strcmp(vendor, CPUID_VENDOR_INTEL) == 0)
	{
		return VENDOR_INTEL;
	}
	else if (strcmp(vendor, CPUID_VENDOR_AMD) == 0)
	{
		return VENDOR_AMD;
	}
	else if (strcmp(vendor, CPUID_VENDOR_UMC) == 0)
	{
		return VENDOR_UMC;
	}
	else if (strcmp(vendor, CPUID_VENDOR_CYRIX) == 0)
	{
		return VENDOR_CYRIX;
	}
	else if (strcmp(vendor, CPUID_VENDOR_NEXGEN) == 0)
	{
		return VENDOR_NEXGEN;
	}
	else if (strcmp(vendor, CPUID_VENDOR_SIS) == 0)
	{
		return VENDOR_SIS;
	}
	else if (strcmp(vendor, CPUID_VENDOR_RISE) == 0)
	{
		return VENDOR_RISE;
	}
	else if (strcmp(vendor, CPUID_VENDOR_CENTAUR) == 0)
	{
		return VENDOR_CENTAUR;
	}
	else if (strcmp(vendor, CPUID_VENDOR_HYGON) == 0)
	{
		return VENDOR_HYGON;
	}
	else if (strcmp(vendor, CPUID_VENDOR_NSC) == 0)
	{
		return VENDOR_NSC;
	}
	else if (strcmp(vendor, CPUID_VENDOR_TRANSMETA) == 0)
	{
		return VENDOR_TRANSMETA;
	}
	else if (strcmp(vendor, CPUID_VENDOR_ZHAOXIN) == 0)
	{
		return VENDOR_ZHAOXIN;
	}
	else if (strcmp(vendor, CPUID_VENDOR_AMD_OLD) == 0)
	{
		return VENDOR_AMD;
	}
	else if (strcmp(vendor, CPUID_VENDOR_TRANSMETA_OLD) == 0)
	{
		return VENDOR_TRANSMETA;
	}
	if ((d[0] == 0) || ((d[0] & 0x500) != 0)) return VENDOR_INTEL;
	return VENDOR_UNKNOWN;
}


void get_cpu_vendor_id_string(char *s)
{
  int has_64_bits;
  int vendor_id;
  char id_str[32];
  unsigned long d[4];
  getcpuid(0, d);
  vendor_id = get_cpu_vendor();
  has_64_bits = has_cpu_support_longmode();
  switch(vendor_id)
  {
	  case VENDOR_INTEL:
	  {
		  strcpy(id_str, CPUID_ID_INTEL);
		  strcat(id_str, " ");
		  if (has_64_bits)
		  {
			  strcat(id_str, CPUID_ID_GENERIC_X64);
		  }
		  else
		  {
			  strcat(id_str, CPUID_ID_GENERIC_X86);
		  }
	  }
	  break;
	  case VENDOR_AMD:
	  {
		  strcpy(id_str, CPUID_ID_AMD);
		  if (has_64_bits)
		  {
			  strcat(id_str, "64");
		  }
	  }
	  break;
	  case VENDOR_CYRIX:
	  {
		  strcpy(id_str, CPUID_ID_CYRIX);
		  strcat(id_str, " ");
		  if (has_64_bits)
		  {
			  strcat(id_str, CPUID_ID_GENERIC_X64);
		  }
		  else
		  {
			  strcat(id_str, CPUID_ID_GENERIC_X86);
		  }
	  }
	  break;
	  case VENDOR_NEXGEN:
	  {
		  strcpy(id_str, CPUID_ID_NEXGEN);
		  strcat(id_str, " ");
		  if (has_64_bits)
		  {
			  strcat(id_str, CPUID_ID_GENERIC_X64);
		  }
		  else
		  {
			  strcat(id_str, CPUID_ID_GENERIC_X86);
		  }
	  }
	  break;
	  case VENDOR_CENTAUR:
	  {
		  strcpy(id_str, CPUID_ID_CENTAUR);
		  strcat(id_str, " ");
		  if (has_64_bits)
		  {
			  strcat(id_str, CPUID_ID_GENERIC_X64);
		  }
		  else
		  {
			  strcat(id_str, CPUID_ID_GENERIC_X86);
		  }
	  }
	  break;
	  case VENDOR_RISE:
	  {
		  strcpy(id_str, CPUID_ID_RISE);
		  strcat(id_str, " ");
		  if (has_64_bits)
		  {
			  strcat(id_str, CPUID_ID_GENERIC_X64);
		  }
		  else
		  {
			  strcat(id_str, CPUID_ID_GENERIC_X86);
		  }
	  }
	  break;
	  case VENDOR_SIS:
	  {
		  strcpy(id_str, CPUID_ID_SIS);
		  strcat(id_str, " ");
		  if (has_64_bits)
		  {
			  strcat(id_str, CPUID_ID_GENERIC_X64);
		  }
		  else
		  {
			  strcat(id_str, CPUID_ID_GENERIC_X86);
		  }
	  }
	  break;
	  case VENDOR_TRANSMETA:
	  {
		  strcpy(id_str, CPUID_ID_TRANSMETA);
		  strcat(id_str, " ");
		  if (has_64_bits)
		  {
			  strcat(id_str, CPUID_ID_GENERIC_X64);
		  }
		  else
		  {
			  strcat(id_str, CPUID_ID_GENERIC_X86);
		  }
	  }
	  break;
	  case VENDOR_NSC:
	  {
		  strcpy(id_str, CPUID_ID_NSC);
		  strcat(id_str, " ");
		  if (has_64_bits)
		  {
			  strcat(id_str, CPUID_ID_GENERIC_X64);
		  }
		  else
		  {
			  strcat(id_str, CPUID_ID_GENERIC_X86);
		  }
	  }
	  break;
	  case VENDOR_HYGON:
	  {
		  strcpy(id_str, CPUID_ID_HYGON);
		  strcat(id_str, " ");
		  if (has_64_bits)
		  {
			  strcat(id_str, CPUID_ID_GENERIC_X64);
		  }
		  else
		  {
			  strcat(id_str, CPUID_ID_GENERIC_X86);
		  }
	  }
	  break;
	  case VENDOR_ZHAOXIN:
	  {
		  strcpy(id_str, CPUID_ID_ZHAOXIN);
		  strcat(id_str, " ");
		  if (has_64_bits)
		  {
			  strcat(id_str, CPUID_ID_GENERIC_X64);
		  }
		  else
		  {
			  strcat(id_str, CPUID_ID_GENERIC_X86);
		  }
	  }
	  break;
	  case VENDOR_UNKNOWN:
	  {
		  if (has_64_bits)
		  {
			  strcpy(id_str, CPUID_ID_GENERIC_X64);
		  }
		  else
		  {
			  strcpy(id_str, CPUID_ID_GENERIC_X86);
		  }
	  }
	  break;
	  default:
	  {
		  if (has_64_bits)
		  {
			  strcpy(id_str, CPUID_ID_GENERIC_X64);
		  }
		  else
		  {
			  strcpy(id_str, CPUID_ID_GENERIC_X86);
		  }
	  }
	  break;
  };
  strcpy(s, id_str);
}


void get_cpu_str_eax(unsigned long eax, char *s)
{
  char str[17];
  unsigned long d[4];
  union
  {
	char c[16];
	int  i[4];
  } v;
  getcpuid(eax, d);
  *(&v.i[0]) = d[0];
  *(&v.i[1]) = d[1];
  *(&v.i[2]) = d[2];
  *(&v.i[3]) = d[3];
  strcpy(str, v.c);
  str[16] = '\0';
  strcpy(s, str);
}

void get_cpu_brand_str(char *s)
{
	char cpu_str[64];
	char cpu_str_1[16];
	char cpu_str_2[16];
	char cpu_str_3[16];
	get_cpu_str_eax(0x80000002,cpu_str_1);
	strcpy(cpu_str, cpu_str_1);
	get_cpu_str_eax(0x80000003,cpu_str_2);
	strcat(cpu_str, cpu_str_2);
	get_cpu_str_eax(0x80000004,cpu_str_3);
	strcat(cpu_str, cpu_str_3);
	strcpy(s, cpu_str);
}

void get_cpu_info(int *type, int *family, int *model, int *stepping)
{
	unsigned long d[4];
	getcpuid(1, d);
	*type = (((d[0] >> 12) & 0x03) + ((d[0] >> 12) & 0x0f));
	*family = (((d[0] >> 8) & 0x0f) + ((d[0] >> 20) & 0xff));
	*model = (((d[0] >> 4) & 0x0f));
	if (*model == 0)
	{
		*model = ((d[0] >> 12) & 0xf0);
	}
	*stepping = ((d[0]) & 0x0f);
}


static unsigned cycles_high = 0, cycles_low = 0;

static void get_cpu_timestamp(unsigned *high, unsigned *low)
{
    __asm__ volatile ("rdtsc; movl %%edx,%0; movl %%eax,%1"
					: "=r" (*high), "=r" (*low)
					:
					: "%edx", "%eax");
}

void start_cpu_counter(void)
{
    get_cpu_timestamp(&cycles_high, &cycles_low);
}

double get_cpu_counter(void)
{
    unsigned nh, nl, h, l, b;
    double r;
    get_cpu_timestamp(&nh, &nl);
    l = nl - cycles_low;
    b = l > nl;
    h = nh - cycles_high - b;
    r = (double) h * (1 << 30) * 4 + l;
    return r;
}

unsigned long get_cpu_speed(void)
{
	unsigned long f=0;
    double c1, c2;
	double ff;
    start_cpu_counter();
    c1 = get_cpu_counter();
    sleep(1);
    c2 = get_cpu_counter();
	ff = (c2-c1)/1E6;
	f = (unsigned long)(ff);
	return f;
}

unsigned long cpu_speed = 100;
unsigned short video_mode = 0;
unsigned char has_vesa = 0;
unsigned char has_vesa_mode = 0;
unsigned char has_vesa_framebuffer = 0;



void int41_handler(registers_t *registers)
{
	printk("interrupt 0x%X called.\n", registers->int_no);
	//__asm__ ("hlt");
}

void int6a_handler(registers_t *registers)
{
	printk("interrupt 0x%X called.\n", registers->int_no);
}

void protection_fault_handler(registers_t *registers)
{
	printk("eax: %X\nebx: %X\necx: %X\nedx: %X\nesp: %X\nebp: %X\nesi: %X\nedi: %X\n", registers->eax, registers->ebx, registers->ecx, registers->edx, registers->esp, registers->ebp, registers->esi, registers->edi);
	isr_exception(registers);
}

void syscall_exit(unsigned long exitcode)
{
	syscall_exit_on = 1;
	kb_flush_require = 1;
	syscall_excv_on = 0;
	enable_interrupt();
	shell_code_exit(exitcode);
}

void *malloc(size_t size);
void free(void *ptr);


void syscall_exec(void *ptr, int argc, const char **argv)
{
	syscall_excv_on = 1;
	shell_code_exec(ptr, argc, (char **)argv);
}


/*
void syscall_exec(void *ptr, int argc, const char **argv)
{
	char **_argv = (char**)malloc(argc*sizeof(char*));
	for(int i=0;i<argc;i++)
	{
		_argv[i] = (char*)malloc((strlen(argv[i])+1)*sizeof(char));
		memcpy(_argv[i], argv[i], strlen(argv[i])+1);
	}
	syscall_excv_on = 1;
	shell_code_exec(ptr, argc, _argv);

	for(int i=0;i<argc;i++)
	{
		free((char*)argv[i]);
	}
	free(_argv);

}
*/


/*
void syscall_exec(void *ptr, int argc, char **argv)
{
	char **_argv = (char**)malloc(256);
	for(int i=0;i<256;i++)
	{
		argv[i] = (char*)malloc(256);
		strcpy(_argv[i], argv[i]);
	}
	syscall_excv_on = 1;
	shell_code_exec(ptr, argc, _argv);
	for(int i=0;i<256;i++)
	{
		free(argv[i]);
	}
	free(_argv);
}

*/

/*
void syscall_exec(void *ptr, int argc, const char **argv)
{
	char **_argv = (char**)malloc(argc*sizeof(char*));
	for(int i=0;i<argc;i++)
	{
		_argv[i] = (char*)malloc((strlen(argv[i])+1)*sizeof(char));
		memcpy(_argv[i], argv[i], strlen(argv[i])+1);
	}
	syscall_excv_on = 1;
	shell_code_exec(ptr, argc, _argv);

	for(int i=0;i<argc;i++)
	{
		free((char*)argv[i]);
	}
	free(_argv);

}
*/

/*
void syscall_exec(void *ptr, int argc, const char **argv)
{
	char **_argv = (char**)malloc(argc*sizeof(char*));
	for(int i=0;i<argc;i++)
	{
		_argv[i] = (char*)malloc((strlen(argv[i])+1)*sizeof(char));
		memcpy(_argv[i], argv[i], strlen(argv[i])+1);
	}
	syscall_excv_on = 1;
	shell_code_exec(ptr, argc, _argv);
	for(int i=0;i<256;i++)
	{
		free(_argv[i]);
	}
	free(_argv);
}

*/

int execv(const char *path, const char **argv)
{
	char fn[256];
	int is_fn_exec=0;
	int i=0;
	int argc=0;
	char *arg;
	if (path == NULL) return 0;
	if (strlen(path) == 0) return 0;
	arg = (char*)argv;
	if (((void*)argv != NULL) && 
				(argv != 0) && 
				(arg != NULL) && 
				(arg != 0) && 
				(arg[0] != 0) && 
				((unsigned long)arg != 0) && 
				((unsigned long)argv != 0))
	{
		while(i < 256*256)
		{
			if (((unsigned long)arg[i]) == 0) break;
			if ((arg[i] == 0) && (arg[i+1] == 0)) break;
			argc++;
			i += 256;
		}
	}
	strcpy(fn, path);
	
	if (strcmp(strchr(strlwr(fn), '.'), ".bin") == 0)
	{
		if (fileexists(fn))
		{
			is_fn_exec = 1;
		}
	}
	else
	{
		strcat(fn, ".bin");
		if (fileexists(fn))
		{
			is_fn_exec = 1;
		}
	}
	
	if (is_fn_exec)
	{
		//printk("%s: sys_execve: %s\n", kernelfilename, strlwr(fn));
		push_module(strlwr(fn));
		int fsz = getfilesize(fn);
		unsigned char *fdata = (unsigned char*)malloc(fsz);
		getfiledata(fn, fdata);
		syscall_exec(fdata, argc, (const char **)argv);
		free(fdata);
		return 1;
	}
	else
	{
		return 0;
	}
}

void syscall_handler(registers_t *registers)
{
	unsigned long x = registers->eax;
	unsigned long arg1 = registers->ebx;
	unsigned long arg2 = registers->ecx;
	unsigned long arg3 = registers->edx;
	/*
	printk("interrupt 0x%X called.\n", registers->int_no);
	printk("eax: %d\nebx: %d\necx: %d\nedx: %d\n", registers->eax, registers->ebx, registers->ecx, registers->edx);
	*/
	//__asm__ ("hlt");
	
	switch(x)
	{
		case 1:
		{
			//sys_exit			
			pop_module();
			syscall_exit(arg1);
			__asm__ ("hlt");
		}
		break;
		case 4:
		{
			//sys_write
			if (arg1 == 1)
			{
				char *buffer_ptr = (char*)arg2;
				int i=arg3;
				tprintl(buffer_ptr, i);
				/*
				int i=0;
				while(i < arg3)
				{
					putch(buffer_ptr[i]);
					i++;
				}
				*/
			}
		}
		break;
		case 11:
		{
			//sys_execve
			enable_interrupt();
			registers->eax = execv((const char*)arg1, (const char**)arg2);
		}
		break;
		case 12:
		{
			registers->eax = chdir((const char*)arg1);
		}
		break;
		case 29:
		{
			presskey();
		}
		break;
		case 45:
		{
			registers->edx = (unsigned long)sbrk(arg1);
		}
		break;
		case 88:
		{
			restart();
		}
		break;
		case 162:
		{
			enable_interrupt();
			msleep(arg1);
		}
		break;
		case 224:
		{
			//get_video_vesa_mode(arg1);
		}
		break;
		case 225:
		{
			set_video_vesa_mode(arg1);
		}
		break;
		case 385:
		{
			registers->edx = (unsigned long)malloc(arg1);
		}
		break;
		case 386:
		{
			free((unsigned char*)arg1);
		}
		break;
		case 400:
		{
			unsigned char _pci_bus = arg1;
			unsigned char _pci_slot = arg2;
			unsigned char _pci_func = arg3;
			unsigned char _pci_offset = registers->esi;
			registers->eax = pci_config_address(_pci_bus, _pci_slot, _pci_func, _pci_offset);
		}
		break;
		case 401:
		{
			unsigned char _pci_bus = arg1;
			unsigned char _pci_slot = arg2;
			unsigned char _pci_func = arg3;
			unsigned char _pci_offset = registers->esi;
			registers->eax = pci_read_byte(_pci_bus, _pci_slot, _pci_func, _pci_offset);
		}
		break;
		case 402:
		{
			unsigned char _pci_bus = arg1;
			unsigned char _pci_slot = arg2;
			unsigned char _pci_func = arg3;
			unsigned char _pci_offset = registers->esi;
			registers->eax = pci_read_word(_pci_bus, _pci_slot, _pci_func, _pci_offset);
		}
		break;
		case 403:
		{
			unsigned char _pci_bus = arg1;
			unsigned char _pci_slot = arg2;
			unsigned char _pci_func = arg3;
			unsigned char _pci_offset = registers->esi;
			registers->eax = pci_read_long(_pci_bus, _pci_slot, _pci_func, _pci_offset);
		}
		break;
		case 404:
		{
			unsigned char _pci_bus = arg1;
			unsigned char _pci_slot = arg2;
			unsigned char _pci_func = arg3;
			unsigned char _pci_offset = registers->esi;
			unsigned char _pci_data = registers->edi;
			pci_write_byte(_pci_bus, _pci_slot, _pci_func, _pci_offset, _pci_data);
		}
		break;
		case 405:
		{
			unsigned char _pci_bus = arg1;
			unsigned char _pci_slot = arg2;
			unsigned char _pci_func = arg3;
			unsigned char _pci_offset = registers->esi;
			unsigned short _pci_data = registers->edi;
			pci_write_word(_pci_bus, _pci_slot, _pci_func, _pci_offset, _pci_data);
		}
		break;
		case 406:
		{
			unsigned char _pci_bus = arg1;
			unsigned char _pci_slot = arg2;
			unsigned char _pci_func = arg3;
			unsigned char _pci_offset = registers->esi;
			unsigned long _pci_data = registers->edi;
			pci_write_long(_pci_bus, _pci_slot, _pci_func, _pci_offset, _pci_data);
		}
		break;
		case 424:
		{
			registers->eax = get_video_mode();
		}
		break;
		case 486:
		{
			unsigned char int_n = arg2;
			unsigned long int_b = arg1;
			set_isr(int_n);
			register_interrupt_handler(int_n, (isr_t)int_b);
		}
		break;
	};
}


void int10_handler(registers_t *reg)
{
	unsigned long value = reg->int_no;
	switch(value)
	{
		case 0x10:
		{
			unsigned char al = UCHAR8A(reg->eax);
			unsigned char ah = UCHAR8B(reg->eax);
			switch(ah)
			{
				case 0x0f:
				{
					printk("get video mode\n");
				}
				break;
				case 0x4f:
				{
					printk("set video mode\n");
				}
				break;
				default:
				{
					printk("invalid operation.\n");
				}
				break;
			};
		}
		break;
		default:
		{
			printk("invalid interrupt %d called.\n", value);
		}
		break;
	};
}


void init_interrupts(void)
{
	//register_interrupt_handler(0x13, int13_handler);
	
	// register_interrupt_handler(0x0d, protection_fault_handler);
	register_interrupt_handler(0x41, int41_handler);
	register_interrupt_handler(0x6a, int6a_handler);
	register_interrupt_handler(0x80, syscall_handler);
	
}

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



const unsigned short ata_base[4] =
{
	0x1F0,
	0x1F0,
	0x170,
	0x170
};

unsigned int ata_delay = 1;

void ata_reset(int id)
{
	ata_delay = 10;	
	outb(ata_base[id] + 0x206, 4);
	msleep(ata_delay);
	outb(ata_base[id] + 0x206, 0);
	msleep(ata_delay);
}

unsigned char ata_wait(int id, int mask, int state)
{
	int s;
	int t=0;
	while(1)
	{
		s = inb(ata_base[id] + 7);
		if((s & mask) == state)
		{
			return 1;
		}
		if ((s & 0x01) || (t >= 300))
		{
			ata_reset(id);
			return 0;
		}
		ata_delay = 10;
		msleep(ata_delay);
		t++;
	}
}

void ata_pio_read(int id, void *buffer, int size) 
{
	unsigned short *buf = (unsigned short*)buffer;
	while(size > 0) {
		*buf = inw(ata_base[id]);
		buf++;
		size -= 2;
	}
}

unsigned char ata_begin(int id, int command, int sector, int count) 
{
	int base;
	int r;
	int sector_start;
	int cylinder_lo;
	int cylinder_hi;
	int flags;
	base = ata_base[id];	
	flags = 0x80;
	flags |= 0x40;
	flags |= 0x20;	
	if(id % 2) flags |= 0x10; // slave
	sector_start = (sector >> 0) & 0xff;
	cylinder_lo = (sector >> 8) & 0xff;
	cylinder_hi = (sector >> 16) & 0xff;
	flags |= (sector >> 24) & 0x0f;
	if(!ata_wait(id, 0x80, 0)) return 0;
	outb(base + 6, flags);
	if(command == 0xA1) r = ata_wait(id, 0x80, 0);
	else r = ata_wait(id, 0x80|0x40, 0x40);
	if(!r) return 0;
	outb(base + 0x206, 0);
	outb(base + 2, count);
	outb(base + 3, sector_start);
	outb(base + 4, cylinder_lo);
	outb(base + 5, cylinder_hi);
	outb(base + 6, flags);
	outb(base + 7, command);
	return 1;
}

unsigned long ata_read(int id, void *buffer, int sector, int count) 
{
	int i;
	if(!ata_begin(id, 0x20, sector, count)) return 0;
	for(i = 0;i < count; i++) 
	{
		if(!ata_wait(id, 8, 8)) return 0;
		ata_pio_read(id, buffer, SECTORSIZE);
		buffer = ((char*)buffer) + SECTORSIZE;
		sector++;
	}
	if(!ata_wait(id, 0x80, 0)) return 0;
	return count;
}

unsigned char get_ata_ident(int id, int command, void *buffer)
{
	if (!ata_begin(id, command, 0, 0)) return 0;
	if (!ata_wait(id, 8, 8)) return 0;
	ata_pio_read(id, buffer, SECTORSIZE);
	return 1;
}

unsigned char get_ata_name(int id, char *s)
{
	unsigned char is_ata, is_atapi;
	unsigned long i;
	unsigned short buf[256];
	char *b = (char*)buf;
	unsigned char st = inb(ata_base[id] + 7);
	if(st == 0xff) {
		return 0;
	}
	ata_reset(id);
	memset(b,0,512);
	is_ata = get_ata_ident(id, 0xEC, b);
	is_atapi = get_ata_ident(id, 0xA1, b);
	if((!is_ata) && (!is_atapi)) {
		return 0;
	}
	for(i=0;i<512;i+=2) {
		st = b[i];
		b[i] = b[i + 1];
		b[i + 1] = st;
	}
	b[256]=0;
	strcpy(s, &b[54]);
	s[40] = 0;
	return 1;
}

unsigned char get_ata_type(int id)
{
	unsigned char is_ata, is_atapi;
	unsigned long i;
	unsigned short buf[256];
	char *b = (char*)buf;
	unsigned char st = inb(ata_base[id] + 7);
	if(st == 0xff) {
		return 0;
	}
	ata_reset(id);
	memset(b,0,512);
	is_ata = get_ata_ident(id, 0xEC, b);
	is_atapi = get_ata_ident(id, 0xA1, b);
	if (is_ata)
	{
		return 1;
	}
	if (is_atapi)
	{
		return 2;
	}
	return 0;
}

void detectide(void)
{
	int i;
	char ata_ide_name[40][4];
	char *ata_ide_order[4] = {"Primary IDE Master", "Primary IDE Slave", "Secondary IDE Master", "Secondary IDE Slave"};
	for(i=0;i<4;i++)
	{
		if (get_ata_name(i, ata_ide_name[i]))
		{
			printk("%s: %s\n", ata_ide_order[i], ata_ide_name[i]);
		}
		else
		{
			printk("%s: None\n", ata_ide_order[i]);
		}
	}
}

extern void int86_start();
extern void int86_end();
extern void int86_regs();
extern void int86_int_rsrv();
extern void int86_int_instr();
extern void int86_int_no();

extern void int386_start();
extern void int386_end();
extern void int386_regs();
extern void int386_int_rsrv();
extern void int386_int_instr();
extern void int386_int_no();

#define get_base_address(x)  (void*)(INT86_BASE_ADDRESS + (void*)x - (unsigned long)int86_start)
void (*exec_int86_code16)() = (void *)INT86_BASE_ADDRESS;

#define get_base_address_int386(x)  (void*)(INT86_BASE_ADDRESS + (void*)x - (unsigned long)int386_start)
void (*exec_int386_code16)() = (void *)INT86_BASE_ADDRESS;

void int86(unsigned char int_no, registers16_t *regs_in, registers16_t *regs_out)
{
	unsigned long nop_nop_nop = 0x909090;
	unsigned long int_instr = 0xCD;
	unsigned long sz = (unsigned long)int86_end - (unsigned long)int86_start;
	void *code_base_16 = (void*)INT86_BASE_ADDRESS;
	void *code_regs_1 = (void*)int86_regs;
	void *code_regs_16 = (void*)get_base_address(code_regs_1);
	void *code_int_1 = (void*)int86_int_rsrv;
	void *code_int_1_16 = get_base_address(code_int_1);
	void *code_int_2 = (void*)int86_int_instr;
	void *code_int_2_16 = get_base_address(code_int_2);
	void *code_int_3 = (void*)int86_int_no;
	void *code_int_3_16 = get_base_address(code_int_3);
	disable_interrupt();
	memcpy(code_base_16, int86_start, sz);
	memcpy(code_regs_16, regs_in,  sizeof(registers16_t));
	memcpy(code_int_1_16, &nop_nop_nop, 3);
	memset(code_int_2_16, int_instr, 1);
	memset(code_int_3_16, int_no, 1);
	exec_int86_code16();
	memcpy(regs_out, code_regs_16, sizeof(registers16_t));
	loadisr();
	loadirq();
	loadidt();
	enable_interrupt();
}

void int386(unsigned char int_no, registers32_t *regs_in, registers32_t *regs_out)
{
	unsigned long nop_nop_nop = 0x909090;
	unsigned long int_instr = 0xCD;
	unsigned long sz = (unsigned long)int386_end - (unsigned long)int386_start;
	void *code_base_16 = (void*)INT86_BASE_ADDRESS;
	void *code_regs_1 = (void*)int386_regs;
	void *code_regs_16 = (void*)get_base_address_int386(code_regs_1);
	void *code_int_1 = (void*)int386_int_rsrv;
	void *code_int_1_16 = get_base_address_int386(code_int_1);
	void *code_int_2 = (void*)int386_int_instr;
	void *code_int_2_16 = get_base_address_int386(code_int_2);
	void *code_int_3 = (void*)int386_int_no;
	void *code_int_3_16 = get_base_address_int386(code_int_3);
	disable_interrupt();
	memcpy(code_base_16, int386_start, sz);
	memcpy(code_regs_16, regs_in,  sizeof(registers32_t));
	memcpy(code_int_1_16, &nop_nop_nop, 3);
	memset(code_int_2_16, int_instr, 1);
	memset(code_int_3_16, int_no, 1);
	exec_int386_code16();
	memcpy(regs_out, code_regs_16, sizeof(registers32_t));
	loadisr();
	loadirq();
	loadidt();
	enable_interrupt();
}

void call86(void *function)
{
	unsigned long mov_ax_x6000 = 0x6000B8;
	unsigned char call_word_ax_1 = 0xFF;
	unsigned char call_word_ax_2 = 0xD0;
	registers16_t regs;
	unsigned long sz = (unsigned long)int86_end - (unsigned long)int86_start;
	void *code_base_16 = (void*)INT86_BASE_ADDRESS;
	void *code_regs_1 = (void*)int86_regs;
	void *code_regs_16 = (void*)get_base_address(code_regs_1);
	void *code_int_1 = (void*)int86_int_rsrv;
	void *code_int_1_16 = get_base_address(code_int_1);
	void *code_int_2 = (void*)int86_int_instr;
	void *code_int_2_16 = get_base_address(code_int_2);
	void *code_int_3 = (void*)int86_int_no;
	void *code_int_3_16 = get_base_address(code_int_3);
	disable_interrupt();
	copy_function_to_base(function, 0x1C00, 0x6000);
	memcpy(code_base_16, int86_start, sz);
	memset(&regs, 0, sizeof(registers16_t));
	memcpy(code_regs_16, &regs,  sizeof(registers16_t));
	memcpy(code_int_1_16, &mov_ax_x6000, 3);
	memset(code_int_2_16, call_word_ax_1, 1);
	memset(code_int_3_16, call_word_ax_2, 1);
	exec_int86_code16();
	loadisr();
	loadirq();
	loadidt();
	enable_interrupt();
}

void call386(void *function)
{
	unsigned long mov_ax_x6000 = 0x6000B8;
	unsigned char call_word_ax_1 = 0xFF;
	unsigned char call_word_ax_2 = 0xD0;
	registers32_t regs;
	unsigned long sz = (unsigned long)int386_end - (unsigned long)int386_start;
	void *code_base_16 = (void*)INT86_BASE_ADDRESS;
	void *code_regs_1 = (void*)int386_regs;
	void *code_regs_16 = (void*)get_base_address_int386(code_regs_1);
	void *code_int_1 = (void*)int386_int_rsrv;
	void *code_int_1_16 = get_base_address_int386(code_int_1);
	void *code_int_2 = (void*)int386_int_instr;
	void *code_int_2_16 = get_base_address_int386(code_int_2);
	void *code_int_3 = (void*)int386_int_no;
	void *code_int_3_16 = get_base_address_int386(code_int_3);
	disable_interrupt();
	copy_function_to_base(function, 0x1C00, 0x6000);
	memcpy(code_base_16, int386_start, sz);
	memset(&regs, 0, sizeof(registers32_t));
	memcpy(code_regs_16, &regs,  sizeof(registers32_t));
	memcpy(code_int_1_16, &mov_ax_x6000, 3);
	memset(code_int_2_16, call_word_ax_1, 1);
	memset(code_int_3_16, call_word_ax_2, 1);
	exec_int386_code16();
	loadisr();
	loadirq();
	loadidt();
	enable_interrupt();
}

unsigned char get_video_mode(void)
{
	unsigned char result;
	registers16_t regs_in, regs_out;
	regs_in.ax = UINT16(0, 0x0F);
	int86(0x10, &regs_in, &regs_out);
	result = UCHAR8A(regs_out.ax);
	return result;
}

void set_video_mode(unsigned short mode)
{
	registers16_t regs_in, regs_out;
	regs_in.ax = mode;
	int86(0x10, &regs_in, &regs_out);
	video_mode = mode;
	has_vesa_framebuffer = 0;
	switch(mode)
	{
		case 0x03:
		{
			video_memory = (unsigned char*)0xB8000;
		};
		break;
		case 0x13:
		{
			video_memory = (unsigned char*)0xA0000;
		};
		break;
		default:
		{
			if (mode < 0x0B)
			{
				if (mode < 0x07)
				{
					video_memory = (unsigned char*)0xB8000;
				}
				else
				{
					video_memory = (unsigned char*)0xB0000;
				}
			}
			else
			{
				video_memory = (unsigned char*)0xA0000;
			}
		};
		break;
	};
}


unsigned char get_video_vesa_info(void)
{
	unsigned char result;
	registers16_t regs;
	memset(vesa_info_buffer, 0, 512);
	memset(&regs, 0, sizeof(registers16_t));
	regs.ax = 0x4F00;
	regs.es = 0;
	regs.di = (unsigned long)vesa_info_buffer;
	int86(0x10, &regs, &regs);
	vesa_info = (vesa_info_t*)vesa_info_buffer;
	if (regs.ax == 0x4F)
	{
		result = 1;
	}
	else
	{
		result = 0;
	}
	return result;
}

unsigned char get_video_vesa_mode(unsigned short mode)
{
	unsigned char result;
	registers16_t regs, regs_out;
	memset(vesa_mode_buffer, 0, 256);
	memset(&regs, 0, sizeof(registers16_t));
	memset(&regs_out, 0, sizeof(registers16_t));
	regs.ax = 0x4F01;
	regs.cx = mode;
	regs.es = 0;
	regs.di = (unsigned long)vesa_mode_buffer;
	int86(0x10, &regs, &regs_out);
	vesa_mode = (vesa_mode_t*)vesa_mode_buffer;
	if (regs_out.ax == 0x4F)
	{
		result = 1;
	}
	else
	{
		result = 0;
	}
	return result;
}

unsigned char set_video_vesa_mode(unsigned short mode)
{
	unsigned short m_attrib;
	unsigned long video_addr;
	unsigned char result;
	registers16_t regs_in, regs_out;
	memset(&regs_in, 0, sizeof(registers16_t));
	memset(&regs_out, 0, sizeof(registers16_t));
	regs_in.ax = 0x4F02;
	regs_in.bx = mode;
	int86(0x10, &regs_in, &regs_out);
	if (regs_out.ax == 0x4F)
	{
		if (get_video_vesa_mode(mode))
		{
			video_mode = mode;
			m_attrib = vesa_mode->mode_attributes;
			if (video_mode < 0x100)
			{
				video_addr = (unsigned long)nearptr(0xA000, 0);
				has_vesa_framebuffer = 0;
			}
			else
			{
				if (m_attrib & 0x0080)
				{
					// lfb
					video_addr = vesa_mode->lfb_address;
					m_attrib |= 0x4000;
					has_vesa_framebuffer = 1;
				}
				else
				{
					if (m_attrib & 0x0040)
					{
						// err
						video_addr = 0;
						set_video_vesa_mode(0x3);
						goto vesa_err;
					}
					if (m_attrib & 0x0007)
					{
						// win a
						video_addr = (unsigned long)nearptr(vesa_mode->window_a_segment, 0);
					}
					else
					{
						// win b
						video_addr = (unsigned long)nearptr(vesa_mode->window_b_segment, 0);
					}
					has_vesa_framebuffer = 0;
				}
			}
			video_memory = (unsigned char*)((unsigned long)video_addr);
			result = 1;
		}
		else
		{
			vesa_err:
			result = 0;
		}
	}
	else
	{
		result = 0;
	}
	return result;
}

unsigned long get_video_vesa_buffer(unsigned short mode)
{
	unsigned short m_attrib;
	unsigned long video_addr;
	unsigned long result;
	unsigned short tmp_mode;
	
	tmp_mode = video_mode;
	if (get_video_vesa_mode(mode))
	{
		m_attrib = vesa_mode->mode_attributes;
		if (video_mode < 0x100)
		{
			video_addr = (unsigned long)nearptr(0xA000, 0);
		}
		else
		{
			if (m_attrib & 0x0080)
			{
				// lfb
				video_addr = vesa_mode->lfb_address;
				m_attrib |= 0x4000;
			}
			else
			{
				if (m_attrib & 0x0040)
				{
					// err
					video_addr = 0;
					goto vesa_err;
				}
				if (m_attrib & 0x0007)
				{
					// win a
					video_addr = (unsigned long)nearptr(vesa_mode->window_a_segment, 0);
				}
				else
				{
					// win b
					video_addr = (unsigned long)nearptr(vesa_mode->window_b_segment, 0);
				}
			}
		}
		result = abs(video_addr);
	}
	else
	{
		vesa_err:
		result = 0;
	}
	get_video_vesa_mode(tmp_mode);
	video_mode = tmp_mode;

	return result;
}

unsigned char has_video_vesa_framebuffer(unsigned short mode)
{
	unsigned short m_attrib;
	unsigned char video_framebuffer;
	unsigned long result;
	unsigned short tmp_mode;
	
	tmp_mode = video_mode;
	if (get_video_vesa_mode(mode))
	{
		m_attrib = vesa_mode->mode_attributes;
		if (video_mode < 0x100)
		{
			video_framebuffer = 0;
		}
		else
		{
			if (m_attrib & 0x0080)
			{
				// lfb
				m_attrib |= 0x4000;
				video_framebuffer = 1;
			}
			else
			{
				video_framebuffer = 0;
			}
		}
		result = video_framebuffer;
	}
	else
	{
		result = 0;
	}
	get_video_vesa_mode(tmp_mode);
	video_mode = tmp_mode;

	return result;
}

void set_vesa_pixel(int x, int y, unsigned long c)
{
	if (!has_vesa) return;
	if (!has_vesa_mode) return;
	if (x < 0) return;
	if (y < 0) return;
	if (x >= vesa_mode->width) return;
	if (y >= vesa_mode->height) return;

	switch (vesa_mode->depth)
	{
		case 1:
		{
			video_memory[xyoffset(x,y,vesa_mode->scan_line_size)] = (unsigned char)(c & 0x01);
		};
		break;
		case 2:
		{
			video_memory[xyoffset(x,y,vesa_mode->scan_line_size)] = (unsigned char)(c & 0x03);
		};
		break;
		case 4:
		{
			video_memory[xyoffset(x,y,vesa_mode->scan_line_size)] = (unsigned char)(c & 0x0F);
		};
		break;
		case 8:
		{
			video_memory[xyoffset(x,y,vesa_mode->scan_line_size)] = (unsigned char)(c & 0xFF);
		};
		break;
		case 16:
		{
			*(unsigned long *)(video_memory + xyoffset16(x, y, vesa_mode->scan_line_size)) = (unsigned short)(c & 0xFFFF);
		};
		break;
		case 24:
		{
			*(unsigned long *)(video_memory + xyoffset24(x, y, vesa_mode->scan_line_size)) = (unsigned long)(c & 0xFFFFFF);
		};
		break;
		case 32:
		{
			*(unsigned long *)(video_memory + xyoffset32(x, y, vesa_mode->scan_line_size)) = c;
		};
		break;
		default:
		{
			return;
		}
		break;
	};

}

unsigned char get_video_pixel(int x, int y)
{
	if (video_mode != 0x13) return 0;
	if (x < 0) return 0;
	if (y < 0) return 0;
	if (x >= 320) return 0;
	if (y >= 200) return 0;

	return video_memory[xyoffset(x,y,320)];
}

void set_video_pixel(int x, int y, unsigned char c)
{
	if (video_mode != 0x13) return;
	if (x < 0) return;
	if (y < 0) return;
	if (x >= 320) return;
	if (y >= 200) return;

	video_memory[xyoffset(x,y,320)] = c;
}

#define rgb16(r,g,b) ((unsigned long)((unsigned char)(b)|((unsigned char)(g) << 4)|((unsigned char)(r) << 8)))
#define rgb24(r,g,b) ((unsigned long)((unsigned char)(b)|((unsigned char)(g) << 8)|((unsigned char)(r) << 16)))
#define rgb32(r,g,b) ((unsigned long)((unsigned char)(b)|((unsigned char)(g) << 8)|((unsigned char)(r) << 16)|((unsigned char)(0xFF) << 24)))

unsigned long rgb(unsigned char r, unsigned char g, unsigned char b)
{
	unsigned long result;
	if (!has_vesa) return 0;
	if (!has_vesa_mode) return 0;
	switch (vesa_mode->depth)
	{
		case 16:
		{
			result = rgb16(r,g,b);
		};
		break;
		case 24:
		{
			result = rgb24(r,g,b);
		};
		break;
		case 32:
		{
			result = rgb32(r,g,b);
		};
		break;
		default:
		{
			result = 0;
		}
		break;
	};
	return result;
}


#define MAXCMDLINES 10

typedef struct
{
	int cmdline_length;
	char *cmdline;
} cmd_line_text_t;

cmd_line_text_t *history;
int history_count = 0;
int history_index = 0;

void push_cmd_line(void)
{
	history_count++;
	if (history_count > MAXCMDLINES-1)
	{
		history_count = 0;
	}
}

void pop_cmd_line(void)
{
	history_count--;
	if (history_count < 0)
	{
		history_count = MAXCMDLINES-1;
	}
}

void push_cmd_line_index(void)
{
	history_index++;
	if (history_index > MAXCMDLINES-1)
	{
		history_index = 0;
	}
}

void pop_cmd_line_index(void)
{
	history_index--;
	if (history_index < 0)
	{
		history_index = MAXCMDLINES-1;
	}
}

void load_cmd_line_str(char *cmdline)
{
	if (history_count == 0)
	{
		return;
	}
	if (history_count > MAXCMDLINES-1)
	{
		history_count = MAXCMDLINES-1;
		return;
	}
	if (history_index > MAXCMDLINES-1)
	{
		push_cmd_line_index();
	}
	if (history_index < 0)
	{
		pop_cmd_line_index();
	}
	memset(cmdline, 0, 1024);
	if (history[history_index].cmdline_length > 0)
	{
		if (strlen(history[history_index].cmdline) > 0)
		{
			strcpy(cmdline, history[history_index].cmdline);
		}
	}
	else
	{
		if (history[history_index].cmdline_length > 1)
		{
			if (strlen(history[history_index].cmdline) > 0)
			{
				strcpy(cmdline, history[history_index].cmdline);
			}
		}
	}
}

void store_cmd_line_str(char *cmdline)
{
	if (strlen(cmdline) > 0)
	{
		if (history_index < 0)
		{
			history_index = 0;
		}
		if (history_count < 0)
		{
			history_count = 0;
		}
		if (history_count >= MAXCMDLINES-1)
		{
			history_count = 0;
		}
		if (history_index >= MAXCMDLINES-1)
		{
			history_count = 0;
		}
		if (strlen(history[history_count].cmdline) > 0)
		{
			strcpy(history[history_count].cmdline, cmdline);
			history[history_count].cmdline_length = strlen(cmdline);
			push_cmd_line();
		}
		else
		{
			if (history[history_count].cmdline == NULL) history[history_count].cmdline = (char*)malloc(1024);
			strcpy(history[history_count].cmdline, cmdline);
			history[history_count].cmdline_length = strlen(cmdline);
			push_cmd_line();
		}
		if (history_index < 0)
		{
			history_index = 0;
		}
		if (history_count < 0)
		{
			history_count = 0;
		}
		if (history_count >= MAXCMDLINES-1)
		{
			history_count = 0;
		}
		if (history_index >= MAXCMDLINES-1)
		{
			history_count = 0;
		}
	}
}

char *lastcmd;
int sx=0,lx=0;

void sh_keydown(unsigned char key, unsigned char status, unsigned char shift, unsigned char scancode)
{
	if ((enum_loaded) && (total_enum > 0) && (has_enum(SYSTEM_STDIO_SERIAL))) 
	{
		return;
	}
	if (history_count == 0)
	{
		return;
	}
	if (history_count > MAXCMDLINES-1)
	{
		return;
	}
	if (history_index > MAXCMDLINES-1)
	{
		push_cmd_line_index();
		return;
	}
	if (scancode == 200) // UP
	{
		int cx;
		char cmdline[1024];
		strcpy(cmdline, "");
		while (strlen(cmdline) == 0)
		{
			pop_cmd_line_index();
			if (history_index < 0)
			{
				history_index = MAXCMDLINES-1;
			}
			load_cmd_line_str(cmdline);
		}
		cx = (TEXT_COLS - wherex()) - 3;
		for(int i=0;i<cx;i++) putch(' ');
		putch('\r');
		print("                                                                               ");
		putch('\r');
		print("/]");
		sx = wherex();
		print(cmdline);
		strcpy(lastcmd, cmdline);
	}
	else
	if (scancode == 208) // DOWN
	{
		int cx;
		char cmdline[1024];
		strcpy(cmdline, "");
		while (strlen(cmdline) == 0)
		{
			push_cmd_line_index();
			if (history_index > MAXCMDLINES-1)
			{
				history_index = 0;
			}
			load_cmd_line_str(cmdline);
		}
		cx = (TEXT_COLS - wherex()) - 3;
		for(int i=0;i<cx;i++) putch(' ');
		putch('\r');
		print("                                                                               ");
		putch('\r');
		print("/]");
		sx = wherex();
		print(cmdline);
		strcpy(lastcmd, cmdline);
		lx = wherex();
	}
	else if (key == '\b')
	{
		if (strlen(lastcmd) > 0)
		{
			if ((wherex() > sx) && (lx < wherex()))
			{
				int bx = wherex();
				keyboard_flush();
				putch(key);
				putch(' ');
				putch(key);
				gotoxy(bx-1, wherey());
				key_code = 0;
				key_status = 0;
				if ((bx - sx) > 0) lastcmd[(bx - sx)-1] = '\0';
			}
		}
	}
}


void init_keys(void)
{
	register_key_handler(sh_keydown);
}


void io_wait(void)
{
	inb(0x80);
	inb(0x80);
	inb(0x80);
	inb(0x80);
}

void str_pad_left(const char *s1, char *s2, int padding)
{
	char *s = s2;
	if (!padding) return;
	int slen = strlen(s1);
	memset(s, ' ', padding);
	memcpy(s, s1, slen);
}

int pvsnprintf(char* str, size_t size, const char* format, va_list ap)
{
	char buf[64];
	char *s;
	int i = 0;
	int z_n = 0;
	int p_n = 0;
	int s_n = 0;
	for (;*format;++format)
	{
		switch (*format)
		{
			case '%':
			{
				z_n = 0;
				p_n = 0;
				s_n = 0;
				s_format_chk:
				switch(*(++format))
				{
					case '-':
					{
						s_n = 1;
						goto s_format_chk;
					}
					break;
					case '0':
					{
						z_n = 0;
						while (isnumber(*format))
						{
							char n_c = (*format);
							n_c -= '0';
							z_n *= 10;
							z_n += n_c;
							format++;
						}
						*(--format);
						goto s_format_chk;
					}
					break;
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
					{
						p_n = 0;
						while (isnumber(*format))
						{
							char n_c = (*format);
							n_c -= '0';
							p_n *= 10;
							p_n += n_c;
							format++;
						}
						*(--format);
						goto s_format_chk;
					}
					break;
					case 's':
					{
						int l=0;
						int p_ln = p_n;
						s = va_arg(ap, char*);
						if (p_n > 0)
						{
							int s_ln = strlen(s);
							for(int j=0;j<s_ln;j++) p_n--;
							for(int k=0;k<p_n;k++) *str++ = ' ';
							if (s_n)
							{
								for(l=0;l<p_n;l++)
								{
									str--;
								}
							}
							else
							{
								if (p_n >= strlen(s)) *str -= strlen(s);
							}
						}
						while(*s)
						{
							*str++ = *s++;
						}
						if ((s_n) && ((p_n > 0) && (p_ln > 0) && (l > 0)))
						{							
							for(int n=0;n<(p_n);n++)
							{
								*str++ = ' ';
							}							
						}
						*s++ = '\0';
					}
					break;
					case 'c':
					{
						int l=0;
						int p_ln = p_n;
						if (p_n > 0)
						{
							int s_ln = 1;
							for(int j=0;j<s_ln;j++) p_n--;
							for(int k=0;k<p_n;k++) *str++ = ' ';
							if (s_n)
							{
								for(l=0;l<p_n;l++)
								{
									str--;
								}
							}
							else
							{
								if (p_n >= 1) *str -= 1;
							}
						}
						*str++ = va_arg(ap, int);
						if ((s_n) && ((p_n > 0) && (p_ln > 0) && (l > 0)))
						{
							for(int n=0;n<(p_ln-l);n++)
							{
								*str++ = ' ';
							}
						}
						*s++ = '\0';
					}
					break;
					case 'd':
					case 'i':
					{
						int l=0;
						int p_ln = p_n;
						itoa(va_arg(ap, long), buf, 10);
						s = buf;
						if (p_n > 0)
						{
							int s_ln = strlen(s);
							for(int j=0;j<s_ln;j++) p_n--;
							for(int k=0;k<p_n;k++) *str++ = ' ';
							if (s_n)
							{
								for(l=0;l<p_n;l++)
								{
									str--;
								}
							}
							else
							{
								if (p_n >= strlen(buf)) *str -= strlen(buf);
							}
						}
						if (z_n > 0)
						{
							int s_ln = strlen(s);
							for(int j=0;j<s_ln;j++) z_n--;
							for(int k=0;k<z_n;k++) *str++ = '0';
						}
						while(*s)
						{
							*str++ = *s++;
						}
						if ((s_n) && ((p_n > 0) && (p_ln > 0) && (l > 0)))
						{
							for(int n=0;n<(p_ln-l);n++)
							{
								*str++ = ' ';
							}
						}
						*s++ = '\0';
					}
					break;
					case 'u':
					{
						int l=0;
						int p_ln = p_n;
						s = itob(va_arg(ap, long), 10);
						if (p_n > 0)
						{
							int s_ln = strlen(s);
							for(int j=0;j<s_ln;j++) p_n--;
							for(int k=0;k<p_n;k++) *str++ = ' ';
							if (s_n)
							{
								for(l=0;l<p_n;l++)
								{
									str--;
								}
							}
							else
							{
								if (p_n >= strlen(s)) *str -= strlen(s);
							}
						}
						if (z_n > 0)
						{
							int s_ln = strlen(s);
							for(int j=0;j<s_ln;j++) z_n--;
							for(int k=0;k<z_n;k++) *str++ = '0';
						}
						while(*s)
						{
							*str++ = *s++;
						}
						if ((s_n) && ((p_n > 0) && (p_ln > 0) && (l > 0)))
						{
							for(int n=0;n<(p_ln-l);n++)
							{
								*str++ = ' ';
							}
						}
						*s++ = '\0';
					}
					break;
					case 'x':
					{
						int l=0;
						int p_ln = p_n;
						s = itob(va_arg(ap, long), 16);
						if (p_n > 0)
						{
							int s_ln = strlen(s);
							for(int j=0;j<s_ln;j++) p_n--;
							for(int k=0;k<p_n;k++) *str++ = ' ';
							if (s_n)
							{
								for(l=0;l<p_n;l++)
								{
									str--;
								}
							}
							else
							{
								if (p_n >= strlen(s)) *str -= strlen(s);
							}
						}
						if (z_n > 0)
						{
							int s_ln = strlen(s);
							for(int j=0;j<s_ln;j++) z_n--;
							for(int k=0;k<z_n;k++) *str++ = '0';
						}
						while(*s)
						{
							*str++ = *s++;
						}
						if ((s_n) && ((p_n > 0) && (p_ln > 0) && (l > 0)))
						{
							for(int n=0;n<(p_ln-l);n++)
							{
								*str++ = ' ';
							}
						}
						*s++ = '\0';
					}
					break;
					case 'X':
					{
						int l=0;
						int p_ln = p_n;
						s = strupr(itob(va_arg(ap, long), 16));
						if (p_n > 0)
						{
							int s_ln = strlen(s);
							for(int j=0;j<s_ln;j++) p_n--;
							for(int k=0;k<p_n;k++) *str++ = ' ';
							if (s_n)
							{
								for(l=0;l<p_n;l++)
								{
									str--;
								}
							}
							else
							{
								if (p_n >= strlen(s)) *str -= strlen(s);
							}
						}
						if (z_n > 0)
						{
							int s_ln = strlen(s);
							for(int j=0;j<s_ln;j++) z_n--;
							for(int k=0;k<z_n;k++) *str++ = '0';
						}						
						while(*s)
						{
							*str++ = *s++;
						}
						if ((s_n) && ((p_n > 0) && (p_ln > 0) && (l > 0)))
						{
							for(int n=0;n<(p_ln-l);n++)
							{
								*str++ = ' ';
							}
						}
						*s++ = '\0';
					}
					break;
					case '%':
					{
						*str++ = '%';
					}
					break;
					default:
					{
						--format;
					}
					break;
				};
				continue;
			}
			break;
			default:
			{
				*str++ = *format;
			}
			break;
		};
	}
	*str++ = '\0';
	return 0;
}


int vsprintf (char* str, const char* format, va_list arglist)
{
  return pvsnprintf (str, 1000, format, arglist);
}

void cprintf(int color, const char *msg, ...)
{
	//int slen = strlen(msg)*4+1;
	char msgBuf[1024];// = (char*)malloc(slen);
	va_list va_alist;

	if (!msg) return;

	//msgBuf[slen - 1] = '\0';
	msgBuf[1024 - 1] = '\0';
	va_start(va_alist, msg);
	vsprintf(msgBuf, msg, va_alist);
	va_end(va_alist);

	cprint(color, msgBuf);
}
/*
void printf(const char *msg, ...)
{
	//int slen = strlen(msg)*4+1;
	char msgBuf[1024];// = (char*)malloc(slen);
	va_list va_alist;

	if (!msg) return;

	//msgBuf[slen - 1] = '\0';
	msgBuf[1024 - 1] = '\0';
	va_start(va_alist, msg);
	vsprintf(msgBuf, msg, va_alist);
	va_end(va_alist);

	cprint(TEXTCOLOR_DEFAULT, msgBuf);
}
*/

void printk(const char *msg, ...)
{
	//int slen = strlen(msg)*4+1;
	char msgBuf[1024];// = (char*)malloc(slen);
	va_list va_alist;

	if (!msg) return;

	//msgBuf[slen - 1] = '\0';
	msgBuf[1024 - 1] = '\0';
	va_start(va_alist, msg);
	vsprintf(msgBuf, msg, va_alist);
	va_end(va_alist);

	tprint(msgBuf);
	//cprint(TEXTCOLOR_DEFAULT, msgBuf);
}


int init_ahci(void)
{
	unsigned long portcnt = 0;
	if (pci_count > 0)
	{
		for(int i=0;i<pci_count;i++)
		{
			if (pci_device[i].pci.vendor != 0xFFFF)
			{
				if ((pci_device[i].pci.class == 0x01) && (pci_device[i].pci.subclass == 0x06))
				{
					if (pci_device[i].pci.bar[5] != 0)
					{
						ahci_hba_address = pci_device[i].pci.bar[5];
						ahci_hba = (ahci_hba_memory_t*)ahci_hba_address;
						ahci_ptr = (unsigned char *)0x841500;
						ahci = (ahci_t*)ahci_ptr;
						strcpy(ahci->signature, "AHCI");
						ahci->version = 1;
						portcnt = check_ahci_ports(&ahci->list[0], &ahci->count);
						ahci->list_count = portcnt;
						init_ahci_ports();
						return 1;
					}
				}
			}
		}
	}
	return 0;
}

unsigned long get_ahci_hba(void)
{
	return ahci_hba_address;
}

void *get_ahci_hba_ptr(void)
{
	unsigned char *hba_ptr = (unsigned char *)get_ahci_hba();
	return hba_ptr;
}

ahci_hba_memory_t *get_ahci_hba_memory(void)
{
	ahci_hba = (ahci_hba_memory_t*)get_ahci_hba();
	return ahci_hba;
}

int check_ahci_type(ahci_hba_port_t *port)
{
	unsigned long ssts = port->ssts;

	unsigned char ipm = (ssts >> 8) & 0x0F;
	unsigned char det = ssts & 0x0F;

	if (det != 3)
		return 0;
	if (ipm != 1)
		return 0;

	switch (port->sig)
	{
	case 0xEB140101:
		return 4;
	case 0xC33C0101:
		return 2;
	case 0x96690101:
		return 3;
	default:
		return 1;
	}
}

int check_ahci_ports(unsigned char *ports, unsigned char *portmax)
{
	unsigned long pi = ahci_hba->pi;
	int i = 0;
	int portcount = 0;
	while (i < 32)
	{
		if (pi & 1)
		{
			int dt = check_ahci_type(&ahci_hba->ports[i]);
			if (dt == 1)
			{
				ports[portcount] = i;
				portcount++;
				*portmax = i;
				ahci->port[i].type = dt;
			}
			else if (dt == 4)
			{
				ports[portcount] = i;
				portcount++;
				*portmax = i;
				ahci->port[i].type = dt;
			}
			else if (dt == 2)
			{
				ports[portcount] = i;
				portcount++;
				*portmax = i;
				ahci->port[i].type = dt;
			}
			else if (dt == 3)
			{
				ports[portcount] = i;
				portcount++;
				*portmax = i;
				ahci->port[i].type = dt;
			}
			else
			{
				ahci->port[i].type = dt;
			}
		}
		pi >>= 1;
		i++;
	}
	return portcount;
}

int check_ahci_port(int portno)
{
	int portcnt = ahci->list_count;
	int i = 0;
	if (portcnt > 0)
	{
		while (i < portcnt)
		{
			if (ahci->list[i] == portno)
			{
				int dt = check_ahci_type(&ahci_hba->ports[portno]);
				ahci->port[portno].type = dt;
				return 1;
			}
			i++;
		}
	}
	return 0;
}

void ahci_start_cmd(ahci_hba_port_t *port)
{
	while (port->cmd & 0x8000)
		printk("");
		;
	port->cmd |= 0x0010;
	port->cmd |= 0x0001;
}

void ahci_stop_cmd(ahci_hba_port_t *port)
{
	port->cmd &= ~0x0001;
	port->cmd &= ~0x0010;
	while(1)
	{
		printk("");
		if (port->cmd & 0x4000)
			continue;
		if (port->cmd & 0x8000)
			continue;
		break;
	}
}

void ahci_port_rebase(ahci_hba_port_t *port, int portno)
{
	ahci_stop_cmd(port);
	port->clb = 0x400000 + (portno<<10);
	port->clbu = 0;
	memset((void*)(port->clb), 0, 1024);
	port->fb = 0x400000 + (32<<10) + (portno<<8);
	port->fbu = 0;
	memset((void*)(port->fb), 0, 256);
	ahci_hba_cmd_header_t *cmdheader = (ahci_hba_cmd_header_t*)(port->clb);
	for (int i=0; i<32; i++)
	{
		cmdheader[i].prdtl = 8;
		cmdheader[i].ctba = 0x400000 + (40<<10) + (portno<<13) + (i<<8);
		cmdheader[i].ctbau = 0;
		memset((void*)cmdheader[i].ctba, 0, 256);
	}
	ahci_start_cmd(port);
}

int init_ahci_ports()
{
	unsigned long pi = ahci_hba->pi;
	int i = 0;
	int r = 0;
	int f = 0;
	while (i < 32)
	{
		if (pi & 1)
		{
			printk("");
			int dt = check_ahci_type(&ahci_hba->ports[i]);
			if (dt == 1)
			{
				ahci->port[i].type = dt;
				ahci_port_rebase(&ahci_hba->ports[i], i);
				if (f == 0)
				{
					ahci_port = &ahci_hba->ports[i];
					f = 1;
				}
				r = 1;
			}
			else if (dt == 4)
			{
				ahci->port[i].type = dt;
				ahci_port_rebase(&ahci_hba->ports[i], i);
				r = 1;
			}
			else
			{
				ahci->port[i].type = dt;
			}
		}
		pi >>= 1;
		i++;
	}
	return r;
}

int ahci_find_cmdslot(ahci_hba_port_t *port)
{
	unsigned long slots = (port->sact | port->ci);
	int num_of_slots = (ahci_hba->cap & 0x0f00) >> 8;
	for (int i=0; i<num_of_slots; i++)
	{
		if ((slots&1) == 0)
			return i;
		slots >>= 1;
	}
	return -1;
}

unsigned char get_sata_ident(ahci_hba_port_t *port, void *buffer)
{
	int i=0,sp=0;
	port->is = (unsigned long)-1;
	int slot = ahci_find_cmdslot(port);
	if (slot == -1) return 0;
	int dt = check_ahci_type(port);
	ahci_hba_cmd_header_t *cmdheader = (ahci_hba_cmd_header_t*)port->clb;
	cmdheader += slot;
	cmdheader->cfl = sizeof(ahci_fis_reg_h2d_t)/sizeof(unsigned long);
	cmdheader->w = 0;
	cmdheader->prdtl = 1;
	ahci_hba_cmd_tbl_t *cmdtbl = (ahci_hba_cmd_tbl_t*)(cmdheader->ctba);
	memset(cmdtbl, 0, sizeof(ahci_hba_cmd_tbl_t)+(cmdheader->prdtl-1)*sizeof(ahci_hba_prdt_entry_t));
	cmdtbl->prdt_entry[i].dba = (unsigned long)buffer;
	cmdtbl->prdt_entry[i].dbc = 2048;
	cmdtbl->prdt_entry[i].i = 1;
	ahci_fis_reg_h2d_t *cmdfis = (ahci_fis_reg_h2d_t*)(&cmdtbl->cfis);
	cmdfis->fis_type = 0x27;
	cmdfis->c = 1;
	if (dt == 4) cmdfis->command = 0xA1;
	else cmdfis->command = 0xEC;
	cmdfis->device = 0;
	cmdfis->countl = 1;
	cmdfis->counth = 0;
	while ((port->tfd & (0x80|0x08)) && (sp<1000000))
	{
		printk("");
		sp++;
	}
	if (sp==1000000) return 0;
	port->ci = 1<<slot;
	while (1)
	{
		printk("");
		if ((port->ci & (1<<slot)) == 0) break;
		if (port->is & (1 << 30)) return 0;
	}
	if (port->is & (1 << 30)) return 0;
	return 1;
}

unsigned long sata_read(int id, void *buffer, unsigned long sector, unsigned long count)
{
	int i=0,sp=0;
	if (ahci == NULL) return 0;
	if (ahci->list == NULL) return 0;
	int index = ahci->list[id];
	ahci_hba_port_t *port = &ahci_hba->ports[index];
	if (port == NULL) return 0;
	port->is = (unsigned long)-1;
	int slot = ahci_find_cmdslot(port);
	if (slot == -1) return 0;
	ahci_hba_cmd_header_t *cmdheader = (ahci_hba_cmd_header_t*)port->clb;
	cmdheader += slot;
	cmdheader->cfl = sizeof(ahci_fis_reg_h2d_t)/sizeof(unsigned long);
	cmdheader->w = 0;
	cmdheader->prdtl = (unsigned short)((count-1)>>4)+1;
	ahci_hba_cmd_tbl_t *cmdtbl = (ahci_hba_cmd_tbl_t*)(cmdheader->ctba);
	memset(cmdtbl, 0, sizeof(ahci_hba_cmd_tbl_t)+(cmdheader->prdtl-1)*sizeof(ahci_hba_prdt_entry_t));
	for (i=0;i<(cmdheader->prdtl-1);i++)
	{
		cmdtbl->prdt_entry[i].dba = (unsigned long)buffer;
		cmdtbl->prdt_entry[i].dbc = 8*1024-1;
		cmdtbl->prdt_entry[i].i = 1;
		buffer += 4*1024;
		count -= 16;
	}
	cmdtbl->prdt_entry[i].dba = (unsigned long)buffer;
	cmdtbl->prdt_entry[i].dbc = (count<<9)-1;
	cmdtbl->prdt_entry[i].i = 1;
	ahci_fis_reg_h2d_t *cmdfis = (ahci_fis_reg_h2d_t*)(&cmdtbl->cfis);
	cmdfis->fis_type = 0x27;
	cmdfis->c = 1;
	cmdfis->command = 0x25;
	cmdfis->lba0 = (unsigned char)sector;
	cmdfis->lba1 = (unsigned char)(sector>>8);
	cmdfis->lba2 = (unsigned char)(sector>>16);
	cmdfis->lba3 = (unsigned char)(sector>>24);
	cmdfis->lba4 = 0;
	cmdfis->lba5 = 0;
	cmdfis->device = 1<<6;
	cmdfis->countl = count & 0xFF;
	cmdfis->counth = (count>>8) & 0xFF;
	while ((port->tfd & (0x80|0x08)) && (sp<1000000))
	{
		printk("");
		sp++;
	}
	if (sp==1000000) return 0;
	port->ci = 1<<slot;
	while(1)
	{
		printk("");
		if ((port->ci & (1<<slot)) == 0) break;
		if (port->is & (1 << 30)) return 0;
	}
	if (port->is & (1 << 30)) return 0;
	return count;
}

unsigned char get_sata_name(ahci_hba_port_t *port, char *s)
{
	unsigned long i;
	unsigned char st = 0;
	unsigned short buf[256];
	char *b = (char*)buf;
	int dt = check_ahci_type(port);
	if ((dt == 1) || (dt == 4))
	{
		if (get_sata_ident(port, (void*)&buf[0]))
		{
			for(i=0;i<512;i+=2)
			{
				st = b[i];
				b[i] = b[i + 1];
				b[i + 1] = st;
			}
			b[256]=0;
			strcpy(s, &b[54]);
			s[40] = 0;
		}
	}
	else
	{
		return 0;
	}
	return 1;
}

void detectahci(void)
{
	int i=0;
	char buffer[512];
	unsigned long pi = ahci_hba->pi;
	while (i<32)
	{
		if (pi & 1)
		{
			int dt = ahci->port[i].type;
			if ((dt == 1) || (dt == 4))
			{
				memset(buffer, 0, 512);
				if (get_sata_name(&ahci_hba->ports[i], &buffer[0]))
				{
					if (dt == 1)
					{
						if (strlen(buffer) == 0)
						{
							strcpy(buffer, "Virtual HD");
						}
						printk("AHCI SATA Drive %d: %s\n", i, buffer);
					}
					else if (dt == 4)
					{
						if (strlen(buffer) == 0)
						{
							strcpy(buffer, "Virtual CD");
						}
						printk("AHCI SATA Drive %d: %s\n", i, buffer);
					}
				}
			}
			else
			{
				printk("AHCI SATA Drive %d: None\n", i);
			}
		}
		pi >>= 1;
		i++;
	}
}

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
			r = sata_read(id, buffer, sector, count);
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
	registers16_t regs;
	regs.ax = 0x4200;
	regs.dx = (unsigned char)(drive & 0xFF);
	regs.ds = 0;
	regs.si = (unsigned long)dapack;
	int86(0x13, &regs, &regs);
	carry = flagbit(regs.flags, 0);
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

#ifdef DAP_ONLY_TRY_WHEN_ERROR

unsigned char readsector(unsigned long sector, unsigned char *buffer)
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

#else

unsigned char readsector(unsigned long sector, unsigned char *buffer)
{
	unsigned char result;
	unsigned long offset = (unsigned long)buffer;
	unsigned short max_dap_sector = (65536-SECTORSIZE);
	int id = 0;
	int i = 30;
	
	if (storage_drive_controller != STORAGE_CONTROLLER_IDE) return storage_read(id,buffer,sector,1);

	if ((offset < 65536) && (sector < max_dap_sector) && (usermode == 0))
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

#endif

#else

unsigned char readsector(unsigned long sector, unsigned char *buffer)
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

#endif

unsigned long sectortobytes(unsigned long sector)
{
	return (sector * SECTORSIZE);
}

unsigned long bytestosector(unsigned long bytes)
{
	return (bytes / SECTORSIZE);
}

unsigned char loadmbr(void)
{
	int i;
	int mbr_inactive;
	fat_t *no_mbr;
	unsigned char no_mbr_fat_type;
	int ata_drive = 0;
	mbr_loaded = 0;
	mbr_active = 0;
	if (storage_drive_controller == STORAGE_CONTROLLER_IDE)
	{
		int ata_type = get_ata_type(ata_drive);
		if (ata_type != 1) return 0;
	}
	if (info->multi_boot != 0) 
	{
		if (!readsector(0, mbr_sector)) return 0;
	}
	else
	{
		if (!readsector(0, mbr_sector)) return 0;
	}
	mbr = (mbr_t*)mbr_sector;
	if (mbr->signature != BOOT_SIGNATURE) return 0;
	active_partition = -1;
	mbr_inactive = 0;	
	no_mbr_fat_type = 0;
	no_mbr = (fat_t*)mbr_sector;
	if (no_mbr->signature == BOOT_SIGNATURE)
	{
		if (strncmp(no_mbr->oem_name, "FASTSYS1", 8) == 0)
		{
			if (strncmp(no_mbr->bpb.bpb2.fat16.type, "FAT16   ", 8) == 0)
			{
				no_mbr_fat_type = PARTITION_FAT16_LBA;
				mbr_inactive = 1;
			}
			if (strncmp(no_mbr->bpb.bpb2.fat32.type, "FAT32   ", 8) == 0)
			{
				no_mbr_fat_type = PARTITION_FAT32_LBA;
				mbr_inactive = 1;
			}
		}
	}
	if (mbr_inactive == 0)
	{	
		for(i=0;i<4;i++)
		{
			if (mbr->partition[i].flag & PARTITION_ACTIVE)
			{
				active_partition = i;
			}
		}
		mbr_active = 1;
	}
	else
	{
		memset(mbr_sector, 0, SECTORSIZE);
		mbr = (mbr_t*)mbr_sector;
		mbr->signature = BOOT_SIGNATURE;
		mbr->partition[0].flag = PARTITION_ACTIVE;
		mbr->partition[0].type = no_mbr_fat_type;
		mbr->partition[0].lba_start = 0;
		mbr->partition[0].lba_end = 0;
		active_partition = 0;
		mbr_active = 0;
	}
	mbr_loaded = 1;
	return 1;
}

bool has_partition_active(void)
{
	if (active_partition >= MAX_PARTITION)
	{
		return false;
	}
	if (active_partition != -1)
	{
		return true;
	}
	return false;
}

void remap_mbr(void)
{
	mbr = (mbr_t*)mbr_sector;
	partition = (partition_entry_t*)mbr->partition;
	if (mbr->signature != BOOT_SIGNATURE)
	{
		panic((unsigned long)mbr);
	}
	if (has_partition_active())
	{
		main_partition = (partition_entry_t*)&partition[active_partition];
	}
	else
	{
		panic((unsigned long)partition);
	}
}

unsigned char isfat16type(void)
{
	unsigned char has_fat16;
	if (!has_partition_active()) return 0;
	has_fat16 = 0;
	switch(main_partition->type)
	{
		case PARTITION_FAT16:
		case PARTITION_FAT16_LBA:
		{
			has_fat16 = 1;
		};
		break;
	}
	return has_fat16;
}

unsigned char hasfat16lba(void)
{
	unsigned char has_lba;
	if (!has_partition_active()) return 0;
	has_lba = 0;
	switch(main_partition->type)
	{
		case PARTITION_FAT16_LBA:
		{
			has_lba = 1;
		};
		break;
	}
	return has_lba;
}

unsigned char isfat32type(void)
{
	unsigned char has_fat32;
	if (!has_partition_active()) return false;
	has_fat32 = 0;
	switch(main_partition->type)
	{
		case PARTITION_FAT32:
		case PARTITION_FAT32_LBA:
		{
			has_fat32 = 1;
		};
		break;
	}
	return has_fat32;
}

unsigned char hasfat32lba(void)
{
	unsigned char has_lba;
	if (!has_partition_active()) return 0;
	has_lba = 0;
	switch(main_partition->type)
	{
		case PARTITION_FAT32_LBA:
		{
			has_lba = 1;
		};
		break;
	}
	return has_lba;
}

unsigned char isfattype(void)
{
	unsigned char has_fat;
	if (!has_partition_active()) return 0;
	has_fat = 0;
	switch(main_partition->type)
	{
		case PARTITION_FAT16:
		case PARTITION_FAT16_LBA:
		case PARTITION_FAT32:
		case PARTITION_FAT32_LBA:
		{
			has_fat = 1;
		};
		break;
	}
	return has_fat;
}

unsigned char hasfatlba(void)
{
	unsigned char has_lba;
	if (!has_partition_active()) return 0;
	has_lba = 0;
	switch(main_partition->type)
	{
		case PARTITION_FAT16_LBA:
		case PARTITION_FAT32_LBA:
		{
			has_lba = 1;
		};
		break;
	}
	return has_lba;
}

unsigned char loadfat(void)
{
	unsigned long sector, fsinfo_sector;
	remap_mbr();
	if (!has_partition_active()) return 0;
	if (!isfattype()) return 0;
	sector = main_partition->lba_start;
	readsector(sector, boot_sector);
	fat = (fat_t*)boot_sector;
	if (fat->signature != BOOT_SIGNATURE) return 0;
	memcpy(bootstrap, fat->bootstrap, FAT32_BOOTSTRAP_SIZE);
	if (isfat32type())
	{
		fsinfo_sector = sector + fat->bpb.bpb2.fat32.fs_info;
		readsector(fsinfo_sector, fat32_fsinfo);
	}
	else
	{
		// total_size_files = get_total_files_size(getrootdirsector());
	}
	fsinfo = (fat32_fsinfo_t*)fat32_fsinfo;	
	return 1;
}

void remap_fat(void)
{
	fat = (fat_t*)boot_sector;
	if (fat->signature != BOOT_SIGNATURE)
	{
		panic((unsigned long)fat);
	}
	fsinfo = (fat32_fsinfo_t*)fat32_fsinfo;
}

void remap_boot(void)
{
	remap_mbr();
	remap_fat();
	if (!has_partition_active()) 
	{
		loadmbr();
		remap_mbr();
	}
	if (!isfattype()) 
	{
		loadfat();
		remap_fat();
	}
}

unsigned long getrootdirsector(void)
{
	unsigned long start;
	unsigned long sectors;
	unsigned long root_dir;
	unsigned long fat_size;
	remap_boot();
	if (!has_partition_active()) return 0;
	if (!isfattype()) return 0;
	start = main_partition->lba_start + fat->bpb.bpb1.reserved_sectors_count;
	if (isfat16type())
	{
		fat_size = fat->bpb.bpb1.fat_size_16;
	}
	else if (isfat32type())
	{
		fat_size = fat->bpb.bpb2.fat32.fat_size_32;
	}
	sectors = fat_size * fat->bpb.bpb1.number_fats;
	root_dir = start + sectors;
	return root_dir;
}

unsigned long getrootdirsectorscount(void)
{
	unsigned long sectors;
	unsigned long entries;
	unsigned long bytes;
	if (!has_partition_active()) return 0;
	if (!isfattype()) return 0;
	bytes = fat->bpb.bpb1.bytes_per_sector;
	entries = fat->bpb.bpb1.root_entries_count;
	sectors = (FAT_ENTRY_SIZE * entries + bytes - 1) / bytes;
	return sectors;
}

unsigned long getcurrentdirsector(void)
{
	remap_boot();
	if (!has_partition_active()) return 0;
	if (!isfattype()) return 0;
	return current_sector_pwd;
}

unsigned long setcurrentdirsector(unsigned long sector)
{
	remap_boot();
	if (!has_partition_active()) return 0;
	if (!isfattype()) return 0;	
	current_sector_pwd = sector;
	strcpy(pwd, getcwd_sector(current_sector_pwd));
	return current_sector_pwd;
}

unsigned long getdatasector(void)
{
	unsigned long start;
	unsigned long root_start;
	unsigned long root_count;
	if (!has_partition_active()) return 0;
	if (!isfattype()) return 0;
	root_start = getrootdirsector();
	root_count = getrootdirsectorscount();
	start = (root_start + root_count);
	return start;
}


unsigned long getdatasectorcount(void)
{
	unsigned long sectors;
	unsigned long start;
	unsigned long total;
	if (!has_partition_active()) return 0;
	if (!isfattype()) return 0;
	start = getdatasector();
	if (isfat16type())
	{
		total = fat->bpb.bpb1.total_sectors_16;
	}
	else if (isfat32type())
	{
		total = fat->bpb.bpb1.total_sectors_32;
	}
	sectors = (total - start);
	return sectors;
}



unsigned long getclustercount(void)
{
	unsigned long clusters;
	unsigned long sectors;
	unsigned long sector_per_cluster;
	if (!has_partition_active()) return 0;
	if (!isfattype()) return 0;
	sectors = getdatasectorcount();
	sector_per_cluster = fat->bpb.bpb1.sector_per_cluster;
	clusters = (sectors / sector_per_cluster);
	return clusters;
}

unsigned long getsectornumber(unsigned long sector)
{
	unsigned long sector_number;
	unsigned long reserved_sectors;
	unsigned long bytes_per_sector;
	if (!has_partition_active()) return 0;
	if (!isfattype()) return 0;
	reserved_sectors = fat->bpb.bpb1.reserved_sectors_count;
	bytes_per_sector = fat->bpb.bpb1.bytes_per_sector;
	if (isfat16type())
	{
		sector_number = (reserved_sectors + (sector * 2 / bytes_per_sector));
	}
	else if (isfat32type())
	{
		sector_number = (reserved_sectors + (sector * 4 / bytes_per_sector));
	}
	return sector_number;
}

unsigned long getentryoffset(unsigned long sector)
{
	unsigned long offset;
	unsigned long bytes_per_sector;
	if (!has_partition_active()) return 0;
	if (!isfattype()) return 0;
	bytes_per_sector = fat->bpb.bpb1.bytes_per_sector;
	if (isfat16type())
	{
		offset = ((sector * 2) % bytes_per_sector);
	}
	else if (isfat32type())
	{
		offset = ((sector * 4) % bytes_per_sector);
	}
	return offset;
}


unsigned long getfirstsectorofcluster(unsigned long cluster)
{
	unsigned long first_sector;
	unsigned long data_sector;
	unsigned long sector_per_cluster;
	if (!has_partition_active()) return 0;
	if (!isfattype()) return 0;
	remap_boot();
	sector_per_cluster = fat->bpb.bpb1.sector_per_cluster;
	data_sector = getdatasector();
	first_sector = (data_sector + (cluster-2) * sector_per_cluster);
	return first_sector;
}

unsigned long getrootdircluster(void)
{
	unsigned long root_cluster;
	if (!has_partition_active()) return 0;
	if (!isfat32type()) return 0;
	root_cluster = fat->bpb.bpb2.fat32.root_cluster;
	return root_cluster;
}

unsigned long getrootdirsectorstart(void)
{
	unsigned long root_cluster;
	unsigned long root_sector_start;
	if (!has_partition_active()) return 0;
	if (!isfattype()) return 0;
	root_cluster = getrootdircluster();
	root_sector_start = getfirstsectorofcluster(root_cluster);
	return root_sector_start;
}

/*
file_entry_t* getfileentryofcluster(unsigned long cluster)
{
	unsigned long first_sector;
	unsigned long entryoffset;
	unsigned long filecount;
	unsigned char sector[SECTORSIZE];
	unsigned char dir_entry_data[FAT_ENTRY_SIZE];
	file_entry_t file[16];
	file_entry_t *file_p;
	file_entry_t* entry;
	if (!has_partition_active()) return NULL;
	if (!isfat32type()) return NULL;
	remap_fat();
	first_sector = getfirstsectorofcluster(cluster);
	if (!readsector(first_sector, sector)) return NULL;
	filecount = 0;
	while (filecount < 16)
	{
		entryoffset = (filecount*FAT_ENTRY_SIZE);
		memcpy(&dir_entry_data[0], &sector[entryoffset], FAT_ENTRY_SIZE);
		entry = (file_entry_t*)dir_entry_data;
		file[filecount] = *entry;
		filecount++;
	}
	file_p = &file[0];
	return file_p;
}


*/

file_entry_t* getfileentryofsector(unsigned long sector)
{
	unsigned long entryoffset;
	unsigned long filecount;
	unsigned char *first_sector_entry = (unsigned char*)root_sector;
	unsigned char dir_entry_data[FAT_ENTRY_SIZE];
	file_entry_t *file_p;
	file_entry_t* entry;
	memset(first_sector_entry, 0, SECTORSIZE);
	memset(file_dir_sector, 0, SECTORSIZE);
	if (!has_partition_active()) return NULL;
	if (!isfattype()) return NULL;
	if (!readsector(sector, first_sector_entry)) return NULL;
	filecount = 0;
	while (filecount < 16)
	{
		entryoffset = (filecount*FAT_ENTRY_SIZE);
		memcpy(dir_entry_data, &first_sector_entry[entryoffset], FAT_ENTRY_SIZE);
		entry = (file_entry_t*)dir_entry_data;
		file_dir_sector[filecount] = *entry;
		filecount++;
	}
	file_p = (file_entry_t*)&file_dir_sector[0];
	return file_p;
}


path_sub_t getpath(const char *p)
{
    path_sub_t ps;
    int i;
    int l;
    i = 0;
    l = 0;
    ps.pathcount = 0;
    while(*p)
    {
        if (ps.pathcount >= 30)
        {
            break;
        }
        if (*p != '/')
        {
            ps.path[ps.pathcount].path[l++] = *p;
        }
        if (i > 0)
        {
            if (*p == '/')
            {
                ps.pathcount++;
                l = 0;
            }
        }
        *p++;
        i++;
    }
    ps.path[ps.pathcount].path[l++] = '\0';
    ps.pathcount++;
    ps.path[ps.pathcount].path[0] = '\0';
    return ps;
}


unsigned long getfilefirstcluster(file_entry_t entry)
{
	unsigned short cluster_hi;
	unsigned short cluster_lo;
	unsigned long cluster;
	if (!has_partition_active()) return false;
	if (!isfattype()) return false;
	remap_boot();
	cluster_hi = entry.first_cluster_hi;
	cluster_lo = entry.first_cluster_lo;
	cluster = USHORT16(cluster_hi, cluster_lo);
	return cluster;
}

unsigned long filesizeondisk(unsigned long size)
{
	unsigned long i,szd=0;
	for(i=0;i<=size;i+=512)
	{
		szd += 512;
	}
	return szd;
}

void strfilenamedot8e3s11(char *source_filename, char *destination_filename)
{
        int n = 1;
        int i = 0;
        int j = 0;
        int k = 0;
        int a = 0;
        int b = 0;
        int has_dot = 0;
        char tmpfn[12];
        char fn[12];
        memset(fn,0,12);
        memset(tmpfn,0,12);
        while (n != 0)
        {
          if (i > 10) n = 0;
          else
          {
                if (source_filename[i] != ' ')
                {
                        tmpfn[j] = source_filename[i];
                        j++;
                }
                else
                {
                        if (i < 9)
                        {
                           if (source_filename[i+1] != ' ')
                           {
                              tmpfn[j] = '.';
                              j++;
                           }
                        }
                }
                i++;
          }
        }
        tmpfn[j] = '\0';
        tmpfn[11] = '\0';
        while (k < 11)
        {
                if (tmpfn[k] == '.') has_dot = 1;
                k++;
        }
        while (a < 11)
        {
                if (has_dot == 0)
                {
                  if (a == 8)
                  {
					    if ((tmpfn[a] != ' ') && (tmpfn[a] != 0))
						{
							fn[b] = '.';
							b++;
						}
                  }
                  fn[b] = tmpfn[a];
                  b++;
                }
                else
                {
                  fn[b] = tmpfn[a];
                  b++;
                }
                a++;
        }
        fn[b] = '\0';
        fn[12] = '\0';
        strcpy(destination_filename,fn);
}

void getshortfilename(char *filename11, char *sfilename)
{
	char filename[12];
	char filename_s[12];
	memcpy(filename_s, filename11, 12);
	filename_s[12] = '\0';
	strfilenamedot8e3s11(filename_s, filename);
	filename[12] = '\0';	
	strcpy(sfilename, filename);
}

unsigned char getlongfilename(char *filename, unsigned long sector)
{
	int d;
	int q;
	unsigned long totalfiles;
	unsigned long entryoffset;
	unsigned long entrycount;
	unsigned long filecount;
	int has_lfn = 0;
	char shortfilename[12];
	char longfilename[256];
	file_entry_t *readdir_entries;
	file_entry_t* file;
	path_sub_t path_sub;
	if (!has_partition_active()) return NULL;
	if (!isfattype()) return NULL;
	entrycount = 0;
	filecount = 0;
	totalfiles = 0;
	entryoffset = 0;
	d = 0;
	q = 0;
	path_sub = getpath(strupr(filename));
	strcpy(longfilename, "");	
	while(q == 0)
	{
		filecount = 0;
		file = getfileentryofsector(sector+entrycount);
		if (file == NULL)
		{
			q = 1;
			break;
		}
		while (filecount < 16)
		{
			if (file[filecount].name[0] == 0)
			{
				q = 1;
				break;
			}
			if (((unsigned char)file[filecount].name[0] != FILE_NAME_DELETED) && 
			    (file[filecount].attribute != F_ATTR_VOLMID))
			{
				if (file[filecount].attribute != F_ATTR_LNGFNM)
				{
					if ((file[filecount].name[0] == '.') && (file[filecount].name[1] == ' ') && (file[filecount].attribute & F_ATTR_DIRECT))
					{
						d++;
					}
					if (d > 1)
					{
						q = 1;
						break;
					}
					getshortfilename(file[filecount].name, shortfilename);
					if (strcmp(strlwr(shortfilename), strlwr(filename)) == 0)
					{
						if ((has_lfn) && (strlen(longfilename) > 0))
						{
							strcpy(filename, longfilename);
							strcpy(longfilename, "");
							has_lfn = 0;						
							return 1;
						}
						else
						{
							strcpy(longfilename, "");
							has_lfn = 0;			
							return 0;
						}
					}
					else
					{
						strcpy(longfilename, "");
					}
				}
				else
				{
					char filename_s1[11];
					char filename_s2[20];
					char lfn_s[256];
					file_lfn_entry_t *file_entry_lfn = (file_lfn_entry_t*)&file[filecount];
					memcpy(filename_s1, file_entry_lfn->name1, 11);
					filename_s1[11] = '\0';
					int a = 1;
					int b = 0;
					while(a < 11)
					{
						if ((filename_s1[a] != 0x00) && ((char)filename_s1[a] != (char)0xFF))
						{
							lfn_s[b] = filename_s1[a];
							b++;
						}
						a += 2;
					}
					memcpy(filename_s2, file_entry_lfn->name2, 20);
					filename_s2[19] = '\0';
					a = 0;
					while (a < 20)
					{
						if (filename_s2[a] != 0x00)
						{
							if ((char)filename_s2[a] != (char)0xFF)
							{
								lfn_s[b] = filename_s2[a];
								b++;
							}
							else
							{
								lfn_s[b] = '\0';
								break;
							}
						}
						a += 2;
					}
					lfn_s[b] = '\0';
					strcatb(longfilename, lfn_s);
					has_lfn = 1;
				}
			}
			filecount++;
		}
		entrycount++;
	}
	return 0;	
}

unsigned char getpartitiontype(void)
{
        unsigned char partition_type;
		if (!main_partition) return 0;
		partition_type = main_partition->type;
		return partition_type;
}

unsigned long getpartitionsize(void)
{
        unsigned long partition_type;
		if (!main_partition) return 0;
		partition_type = (main_partition->lba_end * SECTORSIZE);
		return partition_type;
}

unsigned long getpartitionstart(void)
{
        unsigned long partition_start;
		if (!main_partition) return 0;
		partition_start = (main_partition->lba_start * SECTORSIZE);
		return partition_start;
}

unsigned long getpartitionlbastart(void)
{
        unsigned long lba_start;
		if (!main_partition) return 0;
		lba_start = main_partition->lba_start;
		return lba_start;
}

unsigned long getrootlbaaddress(void)
{
	return getrootdirsector();
}

void getoemname_fat(char *oem_name)
{
		char oem_name_s[16];
		if (!main_partition) return;
		if (!fat) return;
		memset(oem_name_s, 0, 8);
		memcpy(oem_name_s, fat->oem_name, 8);
		strtrm(oem_name, oem_name_s);		
		oem_name[8] = '\0';
}

void getvolumelabel_fat16(char *volume_label)
{
		char volume_label_s[16];
		if (!main_partition) return;
		if (!fat) return;
		memset(volume_label_s, 0, 11);
		memcpy(volume_label_s, fat->bpb.bpb2.fat16.volume_label, 11);
		strtrm(volume_label, volume_label_s);
		volume_label[11] = '\0';
}

void getvolumelabel_fat32(char *volume_label)
{
		char volume_label_s[16];
		if (!main_partition) return;
		if (!fat) return;
		memset(volume_label_s, 0, 11);
		memcpy(volume_label_s, fat->bpb.bpb2.fat32.volume_label, 11);
		strtrm(volume_label, volume_label_s);
		volume_label[11] = '\0';
}

void getfattypelabel_fat16(char *fat_type_label)
{
		char fat_type_label_s[16];
		if (!main_partition) return;
		if (!fat) return;
		memset(fat_type_label_s, 0, 8);
		memcpy(fat_type_label_s, fat->bpb.bpb2.fat16.type, 8);
		strtrm(fat_type_label, fat_type_label_s);
		fat_type_label[8] = '\0';
}

void getfattypelabel_fat32(char *fat_type_label)
{
		char fat_type_label_s[16];
		if (!main_partition) return;
		if (!fat) return;
		memset(fat_type_label_s, 0, 8);
		memcpy(fat_type_label_s, fat->bpb.bpb2.fat32.type, 8);
		strtrm(fat_type_label, fat_type_label_s);
		fat_type_label[8] = '\0';
}

unsigned long getclusterstart_fat16(void)
{
        unsigned long lba_cluster;
		if (!main_partition) return 0;
		if (!fat) return 0;
		lba_cluster = (main_partition->lba_start + fat->bpb.bpb1.reserved_sectors_count) + 
					  (fat->bpb.bpb1.number_fats * fat->bpb.bpb1.fat_size_16);
		return lba_cluster;
}

unsigned long getclusterstart_fat32(void)
{
        unsigned long lba_cluster;
		if (!main_partition) return 0;
		if (!fat) return 0;
		lba_cluster = (main_partition->lba_start + fat->bpb.bpb1.reserved_sectors_count) + 
					  (fat->bpb.bpb1.number_fats * fat->bpb.bpb2.fat32.fat_size_32);
		return lba_cluster;
}

void getoemname(char *oem_name)
{
	if (isfat16type())
	{
		getoemname_fat(oem_name);
	}
	else
	{
		if (isfat32type())
		{
			getoemname_fat(oem_name);
		}
		else
		{
			strcpy(oem_name, "");
		}
	}
}

void getvolumelabel(char *volume_label)
{
	if (isfat16type())
	{
		getvolumelabel_fat16(volume_label);
	}
	else
	{
		if (isfat32type())
		{
			getvolumelabel_fat32(volume_label);
		}
		else
		{
			strcpy(volume_label, "");
		}
	}	
}

void getfattypelabel(char *fat_type_label)
{
	if (isfat16type())
	{
		getfattypelabel_fat16(fat_type_label);
	}
	else
	{
		if (isfat32type())
		{
			getfattypelabel_fat32(fat_type_label);
		}
		else
		{
			strcpy(fat_type_label, "");
		}
	}	
}

unsigned long getclusterstart(void)
{
	if (isfat16type())
	{
		return getclusterstart_fat16();
	}
	else
	{
		if (isfat32type())
		{
			return getclusterstart_fat32();
		}		
		else
		{
			return 0;
		}
	}	
}

void strtrm(char *s1, char *s2)
{
    int i,j;
    int l;
    int n = 0;
    int e = 0;
    char c;
    j = 0;
    l = strlen(s2);
    for(i=0;i<l;i++)
    {
        if (s2[i] != ' ')
        {
            e = i;
        }
    }
    while(*s2)
    {
        c = *s2++;
        if (c != ' ')
        {
            n = 1;
        }
        if (n == 1)
        {
            if (j <= e)
            {
                *s1++ = c;
            }
        }
        j++;
    }
    *s1++ = '\0';
}

unsigned long getrootaddress(void)
{
		unsigned long root_lba_address;
		unsigned long root_address;
		if (!main_partition) return 0;
		if (!fat) return 0;
		root_lba_address = getrootlbaaddress();
		root_address = sectortobytes(root_lba_address);
		return root_address;
}

unsigned char loadrootentries(void)
{
        unsigned char sector[SECTORSIZE];
        int next_dir_sector = 1;
        unsigned long addr = 0;
		unsigned char can_read;
		int next_dir_entry;
		int dir_entry_address;
		unsigned char dir_entry_data[32];
		char volume_str[11];
		char filename[256];
		char filename_s[11];
		file_dir_t *file;
		
		if (!main_partition) return 0;
		if (!fat) return 0;

		addr = getrootlbaaddress();
		
		if (addr == 0) return 0;		
		
		file = (file_dir_t*)malloc(sizeof(file_dir_t));	
        while (next_dir_sector != 0)
        {
                can_read = readsector(addr, sector);
				if (!can_read) return 0;
                if (sector[0] == 0x00)
                {
                 next_dir_sector = 0;
                 break;
                }
                next_dir_entry = 1;
                dir_entry_address = 0;
                while (next_dir_entry != 0)
                {
                        if (dir_entry_address >= SECTORSIZE)
                        {
                         next_dir_entry = 0;
                         break;
                        }
                        memcpy(dir_entry_data,&sector[dir_entry_address],32);
                        file_entry_t *file_entry;
                        file_entry = (file_entry_t*)dir_entry_data;
                        if (file_entry->name[0] != 0)
                        {
                                if (file_entry->attribute == F_ATTR_VOLMID) // Volume ID
                                {
                                        memcpy(volume_str, file_entry->name, 11);
                                        volume_str[11] = '\0';
                                        strtrm(volume_id, volume_str);
                                }
                                else if (file_entry->attribute != F_ATTR_LNGFNM) // if not lfn
                                {
                                       memcpy(filename_s,file_entry->name,11);
                                       filename_s[11] = '\0';
									   file_entry_to_file_dir_sector(file_entry, file, addr);
                                }
                        }
                        else
                        {
                           next_dir_entry = 0;
                           next_dir_sector = 0;
                           break;
                        }
                        dir_entry_address += 32;
                }
                addr++;
        }
		free(file);
		return 1;
}


unsigned char *opendir_sector_dir(unsigned long sector)
{
	int d, q;
	unsigned long totalfiles;
	unsigned long entryoffset;
	unsigned long entrycount;
	unsigned long filecount;
	unsigned long root_dir;
	unsigned char *dir_sector;
	char shortfilename[12];
	char longfilename[256];
	char filename[256];
	file_dir_t *readdir_entries;
	file_entry_t* entry;
	file_dir_t* file;
	if (!has_partition_active()) return NULL;
	if (!isfattype()) return NULL;
	entrycount = 0;
	filecount = 0;
	totalfiles = 0;
	entryoffset = 0;
	d = 0;
	q = 0;
	root_dir = getrootdirsector();
	if (sector < root_dir) return NULL;
	dir_sector = (unsigned char*)malloc((4096*sizeof(file_dir_t))+8+sizeof(file_dir_t)+16);
	if (dir_sector == NULL) return NULL;
	readdir_entries = (file_dir_t*)dir_sector;
	memset(readdir_entries, 0, (4096*sizeof(file_dir_t))+8+sizeof(file_dir_t)+16);
	memcpy(dir_sector+((4096*sizeof(file_dir_t))+8), &sector, 4);
	strcpy(longfilename, "");	
	while(q == 0)
	{
		filecount = 0;
		entry = getfileentryofsector(sector+entrycount);
		if (entry == NULL)
		{
			q = 1;
			break;
		}
		while (filecount < 16)
		{
			if (entry[filecount].name[0] == 0)
			{
				q = 1;
				break;
			}
			if (((unsigned char)entry[filecount].name[0] != FILE_NAME_DELETED) &&
				(entry[filecount].attribute != F_ATTR_VOLMID))
			{
				if (entry[filecount].attribute != F_ATTR_LNGFNM)
				{
					if (totalfiles < 4096)
					{
						if ((entry[filecount].name[0] == '.') && (entry[filecount].name[1] == ' ') && (entry[filecount].attribute & F_ATTR_DIRECT))
						{
							d++;
						}
						if (d > 1)
						{
							q = 1;
							break;
						}
						file = (file_dir_t*)malloc(sizeof(file_dir_t));
						entryoffset = (totalfiles*sizeof(file_dir_t))+4;
						file_entry_to_file_dir(&entry[filecount], file);
						strcpy(shortfilename, file->name);
						if (strlen(longfilename) > 0)
						{
							strcpy(filename, longfilename);
						}
						else
						{
							strcpy(filename, strlwr(shortfilename));
						}
						if ((strcmp(file->name, ".") == 0) || (strcmp(file->name, "..") == 0))
						{
							//skip dir
						}
						else
						{
							strcpy(file->name, filename);
						}
						strcpy(longfilename, "");
						memcpy(&dir_sector[entryoffset], file, sizeof(file_dir_t));
						totalfiles++;
						/*
						if (strlen(file->name) > 0)
						{
							printk("File Entry 0x%X is '%s'\n",entryoffset,file->name);							
						}
						*/
						free(file);
					}
				}
				else
				{
					if (totalfiles < 4096)
					{
						char filename_s1[11];
						char filename_s2[20];
						char lfn_s[256];
						file_lfn_entry_t *file_entry_lfn = (file_lfn_entry_t*)&entry[filecount];
						memcpy(filename_s1, file_entry_lfn->name1, 11);
						filename_s1[11] = '\0';
						int a = 1;
						int b = 0;
						while(a < 11)
						{
							if ((filename_s1[a] != 0x00) && ((char)filename_s1[a] != (char)0xFF))
							{
								lfn_s[b] = filename_s1[a];
								b++;
							}
							a += 2;
						}
						memcpy(filename_s2, file_entry_lfn->name2, 20);
						filename_s2[19] = '\0';
						a = 0;
						while (a < 20)
						{
							if (filename_s2[a] != 0x00)
							{
								if ((char)filename_s2[a] != (char)0xFF)
								{
									lfn_s[b] = filename_s2[a];
									b++;
								}
								else
								{
									lfn_s[b] = '\0';
									break;
								}
							}
							a += 2;
						}
						lfn_s[b] = '\0';
						strcatb(longfilename, lfn_s);
						/*
						if (strlen(longfilename) > 0)
						{
							printk("File Entry 0x%X is '%s'\n",entryoffset,longfilename);
						}
						*/
					}
				}
			}
			filecount++;
		}
		entrycount++;
	}
	memcpy(&dir_sector[0], &totalfiles, 2);
	memset(&dir_sector[2], 0, 2);
	return dir_sector;
}

unsigned char closedir_sector_dir(unsigned char *dir_ptr)
{
	if (dir_ptr == NULL) return 0;	
	memset(dir_ptr, 0, ((4096*sizeof(file_dir_t))+8+sizeof(file_dir_t)+16));
	free(dir_ptr);
	return 1;
}

file_dir_t *readdir_sector_dir(unsigned char *dir_ptr)
{
	unsigned short totalfiles = 0;
	unsigned short filecount = 0;
	file_dir_t *entry;
	file_dir_t *readdir_entries;
	totalfiles = UINT16(dir_ptr[0], dir_ptr[1]);
	filecount = UINT16(dir_ptr[2], dir_ptr[3]);	
	if (!has_partition_active()) return NULL;
	if (!isfattype()) return  NULL;
	if (filecount < 0) return NULL;
	if (filecount >= totalfiles) return NULL;
	readdir_entries = (file_dir_t*)&dir_ptr[4];
	entry = (file_dir_t*)&readdir_entries[filecount];	
	if (entry->name == NULL) return NULL;
	if (entry->name[0] == 0) return NULL;
	filecount++;
	dir_ptr[2] = UCHAR8A(filecount);
	dir_ptr[3] = UCHAR8B(filecount);
	return entry;
}

void file_dir_to_file_entry(file_dir_t* dir, file_entry_t* entry)
{
    strncpy(entry->name, dir->name, 11);
    entry->attribute = dir->attribute;
    entry->reserved = 0;
    entry->creation_time_tenth = dir->creation_time_tenth;
    entry->creation_time = dir->creation_time;
    entry->creation_date = dir->creation_date;
    entry->last_date = dir->last_date;
    entry->first_cluster_hi = dir->first_cluster >> 16;
    entry->write_time = dir->write_time;
    entry->write_date = dir->write_date;
    entry->first_cluster_lo = dir->first_cluster & 0xFFFF;
    entry->size = dir->size;
}

void file_entry_to_file_dir(file_entry_t* entry, file_dir_t* dir) 
{
	char fn[256];
	char filename[256];
	memset(dir->name, 0, 256);
    strncpy(dir->name, entry->name, 11);
    dir->attribute = (unsigned short)entry->attribute;
    dir->creation_time_tenth = entry->creation_time_tenth;
    dir->creation_time = entry->creation_time;
    dir->creation_date = entry->creation_date;
    dir->last_date = entry->last_date;
    dir->write_time = entry->write_time;
    dir->write_date = entry->write_date;
    dir->first_cluster = ((unsigned long)entry->first_cluster_hi << 16) | entry->first_cluster_lo;
    dir->size = entry->size;
	strcpy(fn, dir->name);
	strcpy(filename, fn);
	getshortfilename(fn, strlwr(filename));
	strcpy(dir->name, filename);
}

void file_entry_to_file_dir_sector(file_entry_t* entry, file_dir_t* dir, unsigned long sector) 
{
	char sfn[13];
	char fn[12];
	char filename[256];
	memset(dir->name, 0, 256);
    strncpy(dir->name, entry->name, 11);
    dir->attribute = (unsigned short)entry->attribute;
    dir->creation_time_tenth = entry->creation_time_tenth;
    dir->creation_time = entry->creation_time;
    dir->creation_date = entry->creation_date;
    dir->last_date = entry->last_date;
    dir->write_time = entry->write_time;
    dir->write_date = entry->write_date;
    dir->first_cluster = ((unsigned long)entry->first_cluster_hi << 16) | entry->first_cluster_lo;
    dir->size = entry->size;
	strcpy(fn, dir->name);
	getshortfilename(fn, filename);
	strcpy(sfn, strlwr(filename));
	if (!getlongfilename(filename, sector))
	{
		strcpy(filename, sfn);
	}
	strcpy(dir->name, filename);
}

void *opendir_sector(unsigned long sector)
{
	remap_boot();
	if (!has_partition_active()) return NULL;
	if (!isfattype()) return NULL;	
	return opendir_sector_dir(sector);
}

file_dir_t *readdir_sector(void *dir_ptr)
{
	remap_boot();
	if (!has_partition_active()) return NULL;
	if (!isfattype()) return NULL;	
	file_dir_t* file_ptr = readdir_sector_dir(dir_ptr);
	if (file_ptr == NULL) return NULL;
	return file_ptr;
}

void closedir_sector(void *dir_ptr)
{
	remap_boot();
	closedir_sector_dir(dir_ptr);
}


unsigned char findfile_sector(const char *filename, unsigned long sector)
{
	unsigned char result = 0;
	unsigned long lsector = 0;
	int i = 0;
	void *dir;
	file_entry_t* entry = (file_entry_t*)malloc(sizeof(file_entry_t));	
	file_dir_t *file;
	remap_boot();
	lsector = sector;
	if ((dir = opendir_sector(sector)) != NULL)
	{
		while((file = readdir_sector(dir)) != NULL)
		{		
			file_dir_to_file_entry(file, entry);
			if (strcmp(strupr(file->name), strupr(filename)) == 0)
			{
				if (has_enum(SYSTEM_VERBOSE))
				{
					printk("%s at sector %d has size %d bytes (%d sectors)\n", 
					(filename), 
					getfirstsectorofcluster(getfilefirstcluster(*entry)), 
					file->size, 
					bytestosector(filesizeondisk(file->size)));
				}
				result = 1;
			}	
			if (i == 16)
			{
				lsector++;
				i = 0;
			}		
		}
		closedir_sector(dir);
	}
	free(entry);
	return result;
}


unsigned char fileexists_sector(const char *filename, unsigned long sector)
{
	unsigned char result = 0;
	void *dir;
	unsigned long lsector = 0;
	int i = 0;
	file_dir_t *file;
	remap_boot();
	lsector = sector;
	if ((dir = opendir_sector(sector)) != NULL)
	{
		while((file = readdir_sector(dir)) != NULL)
		{		
			if (strcmp(strupr(file->name), strupr(filename)) == 0)
			{
				result = 1;
			}			
			if (i == 16)
			{
				lsector++;
				i = 0;
			}
		}
		closedir_sector(dir);
	}
	return result;
}

unsigned char findfileinsector(unsigned long sector, char *filename, file_dir_t *file)
{
	unsigned char result = 0;
	void *dir;
	unsigned long lsector = 0;
	int i = 0;
	file_dir_t *file_tmp;
	remap_boot();
	lsector = sector;
	if ((dir = opendir_sector(sector)) != NULL)
	{
		while((file_tmp = readdir_sector(dir)) != NULL)
		{
			if (strcmp(strupr(file_tmp->name), strupr(filename)) == 0)
			{
				memcpy(file, file_tmp, sizeof(file_dir_t));
				result = 1;
			}			
			if (i == 16)
			{
				lsector++;
				i = 0;
			}		
		}
		closedir_sector(dir);
	}
	return result;
}

unsigned char getfileinsector(unsigned long sector, file_dir_t *file, unsigned long *parent_sector)
{
	unsigned char result = 0;
	unsigned long fcluster;
	unsigned long fsector;
	unsigned long fcluster_p;
	unsigned long fsector_p;
	unsigned long root_sector = getrootdirsector();
	void *dir;
	file_dir_t* file_tmp;
	file_entry_t* entry_tmp = (file_entry_t*)malloc(sizeof(file_entry_t));	
	remap_boot();
	if ((dir = opendir_sector(sector)) != NULL)
	{
		while((file_tmp = readdir_sector(dir)) != NULL)
		{
			file_dir_to_file_entry(file_tmp, entry_tmp);
			fcluster = getfilefirstcluster(*entry_tmp);
			fsector = getfirstsectorofcluster(fcluster);
			if ((file_tmp->name[0] == '.') && (file_tmp->name[1] == '.'))
			{
				fcluster_p = getfilefirstcluster(*entry_tmp);
				fsector_p = getfirstsectorofcluster(fcluster_p);
				*parent_sector = fsector_p;
			}
			if (sector == fsector)
			{
				memcpy(file, file_tmp, sizeof(file_dir_t));
				result = 1;
			}			
		}
		closedir_sector(dir);
	}
	free(entry_tmp);
	return result;
}

unsigned char getfiledata_sector(const char *filename, unsigned long sector, unsigned char *data)
{
	unsigned char result = 0;
	unsigned long fsecs = 0;
	unsigned long fsec_cnt = 0;
	unsigned long fsiz_pos = 0;
	unsigned long fsecstotal = 0;
	unsigned long lsector = 0;
	int i = 0;
	void *dir;
	file_dir_t* file;
	file_entry_t *entry = (file_entry_t*)malloc(sizeof(file_entry_t));	
	remap_boot();
	lsector = sector;
	if ((dir = opendir_sector(sector)) != NULL)
	{
		while((file = readdir_sector(dir)) != NULL)
		{
			file_dir_to_file_entry(file, entry);
			if (strcmp(file->name, strupr(filename)) == 0)
			{
				if ((!(file->attribute & F_ATTR_DIRECT)) && (file->name[0] != FILE_NAME_DIRECTORY))
				{
					fsecs = getfirstsectorofcluster(getfilefirstcluster(*entry));
					fsec_cnt = fsecs;
					fsecstotal = fsec_cnt+bytestosector(filesizeondisk(file->size));							
					while(fsec_cnt < fsecstotal)
					{
						if (readsector(fsec_cnt, data+fsiz_pos) != 0)
						{
							result = 1;
						}
						fsiz_pos += SECTORSIZE;
						fsec_cnt++;
					}
				}
			}		
			if (i == 16)
			{
				lsector++;
				i = 0;
			}
		}
		closedir_sector(dir);		
	}
	free(entry);
	return result;
}

unsigned long getfilesize_sector(const char *filename, unsigned long sector)
{
	unsigned long result = 0;
	unsigned long lsector = 0;
	int i = 0;
	void *dir;
	file_dir_t* file;
	remap_boot();
	lsector = sector;
	if ((dir = opendir_sector(sector)) != NULL)
	{
		while((file = readdir_sector(dir)) != NULL)
		{
			if (strcmp(file->name, strupr(filename)) == 0)
			{
				if ((!(file->attribute & F_ATTR_DIRECT)) && (file->name[0] != FILE_NAME_DIRECTORY))
				{
					result = file->size;
				}
			}
			if (i == 16)
			{
				lsector++;
				i = 0;
			}
		}
		closedir_sector(dir);		
	}
	return result;
}

void prints_pad(int color, const char *string, int padding)
{
	if (!padding) return;
	int slen = strlen(string);
	char *str = (char*)malloc(padding);
	memset(str, ' ', padding);
	memcpy(str, string, slen);
	cprintf(color, "%s", str);
	free(str);
}

unsigned long listdir_sector(unsigned long sector, char *file_name)
{
	unsigned long result = 0;
	unsigned long filecount;
	unsigned long fsize;
	unsigned long fcluster;
	unsigned long fsector;
	unsigned long fwhere;
	unsigned long lsector = 0;
	int length_cols = 0;
	int i = 0;
	int cols;
	char filename[256];
	char ftype[8];
	void *dir;
	file_entry_t* entry = (file_entry_t*)malloc(sizeof(file_entry_t));	
	file_dir_t *file;
	filecount = 0;
	cols = 0;
	remap_boot();
	lsector = sector;
	if ((dir = opendir_sector(sector)) != NULL)
	{
		while((file = readdir_sector(dir)) != NULL)
		{
			file_dir_to_file_entry(file, entry);
			strcpy(filename, file->name);
			fsize = file->size;
			fcluster = getfilefirstcluster(*entry);
			fsector = getfirstsectorofcluster(fcluster);
			fwhere = sectortobytes(fsector);			
			if (((strcmp(filename, ".") == 0) && (file->attribute & F_ATTR_DIRECT)) || ((strcmp(filename, "..") == 0) && (file->attribute & F_ATTR_DIRECT)))
			{
				// skip
			}
			else
			{
				if ((file_name != NULL) && (file_name[0] != 0) && (strlen(file_name) > 0))
				{
					if (strcmp(strlwr(filename), strlwr(file_name)) == 0)
					{
						if (cols == 4)
						{
							cprintf(TEXTCOLOR_DEFAULT, "\n");
							length_cols = 0;
							cols = 0;
						}
						if (cols >= 2)
						{
							if ((length_cols > 40) && (strlen(filename) > 12))
							{
								cprintf(TEXTCOLOR_DEFAULT, "\n");
								cols = 0;
								length_cols = 0;
							}
						}
						if (file->attribute & F_ATTR_DIRECT)
						{
							strcpy(ftype, "<DIR>");
							cprintf(LIGHTBLUE,"%-12s", (filename));
							length_cols += strlen(filename);
							//cprintf(LIGHTBLUE,"%-12s", strlwr(filename));
						}
						else
						{
							strcpy(ftype, "");
							cprintf(TEXTCOLOR_DEFAULT, "%-12s", (filename));
							length_cols += strlen(filename);
							//cprintf(TEXTCOLOR_DEFAULT, "%-12s", strlwr(filename));
						}
						if (cols < 3)
						{
							if ((cols == 2) && (length_cols >= 70))
							{
								cprintf(TEXTCOLOR_DEFAULT, " ");
							}
							else
							{
								if (length_cols >= 70) cprintf(TEXTCOLOR_DEFAULT, " ");
								else 
								{
									if ((strlen(filename) > 12) && (length_cols >= 50))
									{
										cprintf(TEXTCOLOR_DEFAULT, "   ");
									} else cprintf(TEXTCOLOR_DEFAULT, "\t");								
								}
								length_cols += 8;
							}
						}
					}
				}
				else
				{
					if (cols == 4)
					{
						cprintf(TEXTCOLOR_DEFAULT, "\n");
						cols = 0;
						length_cols = 0;
					}
					if (cols >= 2)
					{
						if ((length_cols > 40) && (strlen(filename) > 12))
						{
							cprintf(TEXTCOLOR_DEFAULT, "\n");
							cols = 0;
							length_cols = 0;
						}
					}
					if (file->attribute & F_ATTR_DIRECT)
					{
						strcpy(ftype, "<DIR>");
						cprintf(LIGHTBLUE,"%-12s", (filename));
						length_cols += strlen(filename);
						//cprintf(LIGHTBLUE,"%-12s", strlwr(filename));
					}
					else
					{
						strcpy(ftype, "");
						cprintf(TEXTCOLOR_DEFAULT, "%-12s", (filename));
						length_cols += strlen(filename);
						//cprintf(TEXTCOLOR_DEFAULT, "%-12s", strlwr(filename));
					}
					if (cols < 3)
					{
						if ((cols == 2) && (length_cols >= 70))
						{
							cprintf(TEXTCOLOR_DEFAULT, " ");
						}
						else
						{
							if (length_cols >= 70) cprintf(TEXTCOLOR_DEFAULT, " ");
							else 
							{
								if ((strlen(filename) > 12) && (length_cols >= 50))
								{
									cprintf(TEXTCOLOR_DEFAULT, "   ");
								} else cprintf(TEXTCOLOR_DEFAULT, "\t");								
							}
						}
						length_cols += 8;
					}	
				}
				if (i == 16)
				{
					lsector++;
					i = 0;
				}
				/*
				if (file->attribute & F_ATTR_DIRECT)
				{
					strcpy(ftype, "<DIR>");
					printk("%s\t%s\t\t\t%u\t%u\t0x%X\n", filename, ftype, fcluster, fsector, fwhere);
					//printk("%-12s\t%-12s\t\t\t%-10u\t%-10u\t0x%08X\n", filename, ftype, fcluster, fsector, fwhere);
				}
				else
				{
					strcpy(ftype, "");
					printk("%s\t%s\t\t\t%u\t%u\t0x%X\n", filename, ftype, fcluster, fsector, fwhere);
					//printk("%-12s\t%-12s\t%-10u\t%-10u\t%-10u\t0x%08X\n", filename, ftype, fsize, fcluster, fsector, fwhere);
				}
				*/
				filecount++;
				cols++;		
			}
		}
		closedir_sector(dir);		
		if (filecount > 0)
		{
			cprintf(TEXTCOLOR_DEFAULT, "\n");
			length_cols = 0;
		}
	}
	free(entry);
	return filecount;
}

unsigned char findfile(const char *filename, file_dir_t *file, unsigned long *sector, int only_dir, char *find_filename)
{
	unsigned char result = 0;
	unsigned char has_file = 0;
	unsigned long file_sector = getcurrentdirsector();
	char file_name[256];
	memset(file_name, 0, 256);
	if (strlen(filename) > 0)
	{
		if (strcmp(filename, "/") == 0)
		{
			file_sector = getrootdirsector();
			has_file = 1;
			file_dir_t *find_file = (file_dir_t *)malloc(sizeof(file_dir_t));
			memset(find_file, 0, sizeof(file_dir_t));
			find_file->attribute = F_ATTR_DIRECT;
			memcpy(file, find_file, sizeof(file_dir_t));
			free(find_file);
		}
		else
		{
			file_dir_t *find_file = (file_dir_t *)malloc(sizeof(file_dir_t));
			file_entry_t *find_file_entry = (file_entry_t *)malloc(sizeof(file_entry_t));
			path_sub_t path_sub = getpath(strupr(filename));						
			if (path_sub.pathcount != 0)
			{
				if (filename[0] == '/')
				{
					file_sector = getrootdirsector();
				}
				if ((path_sub.pathcount == 1) && (strcmp(path_sub.path[0].path, ".") == 0))
				{
					has_file = 1;
					memset(find_file, 0, sizeof(file_dir_t));
					find_file->attribute = F_ATTR_DIRECT;
					memcpy(file, find_file, sizeof(file_dir_t));
					file_dir_to_file_entry(find_file, find_file_entry);
				}
				else
				{
					for(int j=0;j<path_sub.pathcount;j++)
					{
						if (path_sub.path[j].path[0] != 0)
						{
							has_file = findfileinsector(file_sector, path_sub.path[j].path, find_file);
							if (has_file)
							{
								file_dir_to_file_entry(find_file, find_file_entry);
								if (only_dir)
								{
									if (find_file->attribute & F_ATTR_DIRECT)
									{
										file_sector = getfirstsectorofcluster(getfilefirstcluster(*find_file_entry));
										memcpy(file, find_file, sizeof(file_dir_t));										
									}
									else
									{
										if (find_filename != NULL)
										{
											strcpy(find_filename, path_sub.path[j].path);
										}
									}
								}
								else
								{
									file_sector = getfirstsectorofcluster(getfilefirstcluster(*find_file_entry));
									memcpy(file, find_file, sizeof(file_dir_t));									
									if (find_filename != NULL)
									{
										strcpy(find_filename, path_sub.path[j].path);
									}
								}
							}
							else
							{
								break;
							}
						}
					}
				}
			}
			free(find_file);	
			free(find_file_entry);			
		}
	}	
	if (has_file)
	{
		*sector = file_sector;
		result = 1;
	}
	else
	{
		result = 0;
	}
	return result;
}

unsigned char getfilenamefromsector_parent(unsigned long sector, char *filename, unsigned long *parent_sector)
{
	unsigned char result = 0;
	unsigned char has_file = 0;
	unsigned long fsector = 0;
	unsigned long fcluster = 0;
	//unsigned long current_sector = getcurrentdirsector();
	unsigned long root_sector = getrootdirsector();
	
	if (sector == root_sector)
	{
		strcpy(filename, "/");
		has_file = 1;
		*parent_sector = 0;
	}
	else 
	{
		file_dir_t *find_file = (file_dir_t *)malloc(sizeof(file_dir_t));
		has_file = getfileinsector(sector, find_file, parent_sector);
		if (has_file)
		{
			file_entry_t *entry = (file_entry_t*)malloc(sizeof(file_entry_t));	
			strcpy(filename, strlwr(find_file->name));
			if (parent_sector != NULL)
			{
				void *dir;
				file_dir_t *file;
				dir = opendir_sector(*parent_sector);					
				if( dir != NULL ) 
				{
					for(;;) 
					{
						file = readdir_sector(dir);
						if( file == NULL ) break;
						file_dir_to_file_entry(file, entry);
						fcluster = getfilefirstcluster(*entry);
						fsector = getfirstsectorofcluster(fcluster);
						if (fsector == sector)
						{
							if (strlen(file->name) < 12)
							{
								strcpy(filename, strlwr(file->name));	
							}
							else
							{
								strcpy(filename, file->name);		
							}
						}
					}
					closedir_sector( dir );
				}
			}
			else *parent_sector = 0;
			free(entry);
		} else *parent_sector = 0;
		free(find_file);	
		/*
		if (sector == current_sector)
		{
			strcpy(filename, ".");
			has_file = 1;
		}
		else
		{
			file_dir_t *find_file = (file_dir_t *)malloc(sizeof(file_dir_t));
			has_file = getfileinsector(sector, find_file);
			if (has_file)
			{
				file_dir_t *file_tmp = (file_dir_t*)malloc(sizeof(file_dir_t));	
				file_entry_to_file_dir_sector(find_file, file_tmp, sector);
				strcpy(filename, strlwr(file_tmp->name));
				free(file_tmp);
			}
			free(find_file);			
		}
		*/
	}
	
	if (has_file)
	{
		result = 1;
	}
	else
	{
		result = 0;
	}
	return result;
}

unsigned char getfilenamefromsector(unsigned long sector, char *filename)
{
	unsigned char result = 0;
	unsigned long parent_sector = getrootdirsector();//0
	result = getfilenamefromsector_parent(sector, filename, &parent_sector);
	return result;
}

unsigned char getfilepathfromsector2(unsigned long sector, char *filename)
{
	return getfilenamefromsector(sector, filename);
}

unsigned char getfilepathfromsector(unsigned long sector, char *filename)
{
	char fn[1024];
	char tmp_fn[1024];
	char full_fn[1024];
	unsigned char result = 1;
	unsigned long root_sector = getrootdirsector();
	unsigned long parent_sector = root_sector;
	unsigned long current_sector = sector;
	int q = 0;
	memset(fn, 0, 1024);
	memset(full_fn, 0, 1024);
	memset(tmp_fn, 0, 1024);
	strcpy(tmp_fn, "/");
	while ((parent_sector != 0) && (parent_sector != NULL) && (result == 1) && (q == 0))
	{
		result = getfilenamefromsector_parent(current_sector, fn, &parent_sector);
		if (parent_sector == root_sector)
		{
			current_sector = parent_sector;
			strcat(tmp_fn, fn);
			parent_sector = NULL;
			q = 1;
			break;
		}
		else if (parent_sector == 0)
		{
			parent_sector = NULL;
			q = 1;
			break;
		}
		else if (parent_sector == NULL)
		{
			q = 1;
			break;
		}
		else
		{
			current_sector = parent_sector;
			strcat(tmp_fn, fn);
			strcat(tmp_fn, "/");
		}
	}
	if (strlen(tmp_fn) > 0)
	{
		path_sub_t path_sub = getpath(tmp_fn);		
		strcpy(full_fn, "/");
		full_fn[strlen(full_fn)] = '\0';
		if (path_sub.pathcount != 0)
		{
			int j=0;
			for(int i=path_sub.pathcount-1;i>=0;i--)
			{
				if (path_sub.path[i].path[0] != 0)
				{
					if (j > 0)
					{
						if (j == 3)
						{
							full_fn[strlen(full_fn)-1] = '\0';
							full_fn[strlen(full_fn)] = '\0';
						}
						strcat(full_fn, "/");
					}
					full_fn[strlen(full_fn)] = '\0';
					if (j == 4) path_sub.path[i].path[strlen(path_sub.path[i].path)-1] = '\0';
					path_sub.path[i].path[strlen(path_sub.path[i].path)] = '\0';
					strcat(full_fn, path_sub.path[i].path);
					j++;
				}
			}
		}
		result = 1;
		strcpy(filename, full_fn);
	}
	else 
	{
		result = 0;
	}
	return result;
}

unsigned char is_file(const char *filename)
{
	unsigned char result = 0;
	unsigned long sector = getrootdirsector();
	file_dir_t *find_file = (file_dir_t *)malloc(sizeof(file_dir_t));
	if (findfile(filename, find_file, &sector, 0, NULL))
	{
		if ((!(find_file->attribute & F_ATTR_DIRECT)) && (find_file->name[0] != FILE_NAME_DIRECTORY)) result = 1;
	}
	else 
	{
		result = 0;
	}
	free(find_file);
	return result;
}

unsigned char is_dir(const char *filename)
{
	unsigned char result = 0;
	unsigned long sector = getrootdirsector();
	file_dir_t *find_file = (file_dir_t *)malloc(sizeof(file_dir_t));
	if (findfile(filename, find_file, &sector, 0, NULL))
	{
		if ((find_file->attribute & F_ATTR_DIRECT) || (find_file->name[0] == FILE_NAME_DIRECTORY)) result = 1;
	}
	else 
	{
		result = 0;
	}
	free(find_file);
	return result;
}

unsigned char fileexists(const char *filename)
{
	unsigned char result = 0;
	unsigned long sector = getrootdirsector();
	file_dir_t *find_file = (file_dir_t *)malloc(sizeof(file_dir_t));
	if (findfile(filename, find_file, &sector, 0, NULL))
	{
		result = 1;
	}
	else 
	{
		result = 0;
	}
	free(find_file);
	return result;
}

unsigned long getfilesize(const char *filename)
{
	unsigned long result = 0;
	unsigned long sector = getrootdirsector();
	file_dir_t *file = (file_dir_t *)malloc(sizeof(file_dir_t));
	if (findfile(filename, file, &sector, 0, NULL))
	{
		if ((!(file->attribute & F_ATTR_DIRECT)) && (file->name[0] != FILE_NAME_DIRECTORY))
		{
			result = file->size;
		}
	}
	free(file);
	return result;
}

unsigned char getfiledata(const char *filename, unsigned char *data)
{
	unsigned char result = 0;
	unsigned long fsecs = 0;
	unsigned long fsec_cnt = 0;
	unsigned long fsiz_pos = 0;
	unsigned long fsecstotal = 0;	
	unsigned long sector = getrootdirsector();
	file_dir_t *file = (file_dir_t *)malloc(sizeof(file_dir_t));
	file_entry_t *entry = (file_entry_t *)malloc(sizeof(file_entry_t));
	if (findfile(filename, file, &sector, 0, NULL))
	{
		if ((!(file->attribute & F_ATTR_DIRECT)) && (file->name[0] != FILE_NAME_DIRECTORY))
		{
			file_dir_to_file_entry(file, entry);
			fsecs = getfirstsectorofcluster(getfilefirstcluster(*entry));
			fsec_cnt = fsecs;
			fsecstotal = fsec_cnt+bytestosector(filesizeondisk(file->size));							
			while(fsec_cnt < fsecstotal)
			{
				if (readsector(fsec_cnt, data+fsiz_pos) != 0)
				{
					result = 1;
				}
				fsiz_pos += SECTORSIZE;
				fsec_cnt++;
			}
		}
	}
	free(file);
	free(entry);
	return result;
}

void *opendir(char *filename)
{
	char filename_s[1024];
	unsigned char result = 0;
	unsigned long sector = getrootdirsector();
	void *dir_ptr;
	if (filename == NULL) return NULL;
	file_dir_t *file = (file_dir_t *)malloc(sizeof(file_dir_t));
	strcpy(filename_s, filename);
	if (strlen(filename_s) == 0)
	{
		strcat(filename_s, ".");
	}
	if (findfile(filename_s, file, &sector, 1, NULL))
	{
		result = 1;
	}
	free(file);
	if (result)
	{
		dir_ptr = opendir_sector(sector);
	}
	else
	{
		dir_ptr = NULL;
	}
	return dir_ptr;
}

file_dir_t *readdir(void *dir_ptr)
{
	char fn[256];
	memset(fn, 0, 256);
	unsigned long parent_sector;
	unsigned long dir_ptr_start = (unsigned long)dir_ptr;
	unsigned long dir_ptr_len = (4096*sizeof(file_dir_t))+4;
	int has_file = 0;
	unsigned char *sect = (unsigned char*)dir_ptr_start+dir_ptr_len+4;
	unsigned long sector_start = (unsigned long)UINT32(sect[0], sect[1], sect[2], sect[3]);
	file_dir_t *file = (file_dir_t*)(dir_ptr_start+dir_ptr_len+8+sizeof(file_dir_t));
	memset(file, 0, sizeof(file_dir_t));
	file_dir_t *entry = readdir_sector(dir_ptr);
	if (entry == NULL) return NULL;
	unsigned long sector = sector_start;
	if (sector == 0) return NULL;
	memcpy(file, entry, sizeof(file_dir_t));
	strcpy(file->name, file->name);
	return file;
}

void closedir(void *dir_ptr)
{
	closedir_sector(dir_ptr);
}

unsigned char listdir(const char *filename)
{
	unsigned char result = 0;
	unsigned char has_file = 1;
	unsigned long sector = getcurrentdirsector();
	char file_name[256];
	memset(file_name, 0, 256);
	if (strlen(filename) > 0)
	{
		file_dir_t *find_file = (file_dir_t *)malloc(sizeof(file_dir_t));
		if (findfile(filename, find_file, &sector, 1, file_name))
		{
			has_file = 1;
		}
		else
		{
			has_file = 0;
		}
		free(find_file);
	}	
	if (has_file)
	{
		result = 1;
		listdir_sector(sector, file_name);
	}
	else
	{
		result = 0;
	}
	return result;
}


unsigned char checksum_str (char *str)
{
  unsigned char sum = 0;                                                                                                                                                                                           
  for (int i=128;i!=0;i--)
  {               
      sum = ((sum & 1) ? 0x80 : 0) + (sum >> 1) + *str++;
  }   
  return sum;
}

unsigned long get_sys_vars_checksum(void)
{
    unsigned long checksum=0;
	unsigned long vars_count=sys_vars_info->count;
    for(int i=0;i<vars_count;i++)
    {
        checksum += checksum_str(sys_vars[i].name);
        checksum += checksum_str(sys_vars[i].value);
        checksum += vars_count;
        checksum += vars_count >> i;
    }
    checksum <<= 4;
    checksum &= 0xffffffff;
	return checksum;
}

unsigned long get_sys_enum_checksum(void)
{
    unsigned long checksum=0;
	unsigned long enum_count=sys_enum_info->count;
    for(int i=0;i<enum_count;i++)
    {
        checksum += checksum_str(sys_enum->name[i]);
        checksum += (sys_enum->value[i]) << 4;
        checksum += enum_count;
        checksum += enum_count >> i;
    }
    checksum <<= 4;
    checksum &= 0xffffffff;
	return checksum;
}

int has_sys_var(const char* _sys_var_)
{
	for(int i=0;i<sys_vars_info->count;i++)
	{
		if (strcmp(sys_vars[i].name, _sys_var_) == 0)
		{
			return 1;
		}
	}
	return 0;
}

char* get_sys_var(const char* _sys_var_)
{
	char *_sys_var_value_;
	for(int i=0;i<sys_vars_info->count;i++)
	{
		if (strcmp(sys_vars[i].name, _sys_var_) == 0)
		{
			_sys_var_value_ = sys_vars[i].value;
			return _sys_var_value_;
		}
	}
	return NULL;
}

void add_sys_var(const char* _sys_var_name_, const char* _sys_var_value_)
{
	int i = sys_vars_info->count;
	strcpy(sys_vars[i].name, _sys_var_name_);
	strcpy(sys_vars[i].value, _sys_var_value_);
	sys_vars_info->count++;
}

unsigned char has_loader_kernel(void)
{
	if (loader_kernel[0] == 0) return 0;
	else return 1;
}

unsigned char has_external_kernel(void)
{
	if (external_kernel[0] == 0) return 0;
	else return 1;
}

unsigned char is_external_kernel(void)
{
	if ((!has_loader_kernel()) && (has_external_kernel())) return 1;
	else return 0;
}

void get_external_kernel_cmdline(char *s)
{
	const char *s1 = (const char *)external_kernel;
	while (*s1)
	{
		*s++ = *s1++;
	}
	*s = '\0';
}

void get_external_kernel_hypervisor(char *s)
{
	const char *s1 = (const char *)external_kernel;
	while (*s1)
	{
		s1++;
	}
	s1++;
	while (*s1)
	{
		*s++ = *s1++;
	}
	*s = '\0';
}

void loadvars(void)
{
	char cmdline[65536];
	char hypervisor[65536];
	char k_argv[256][256];
	sys_vars = (sys_vars_t *)system_variables;
	sys_vars_info = (sys_vars_info_t *)system_variables_info;
	sys_enum = (sys_enum_t *)system_variables_enum;
	sys_enum_info = (sys_enum_info_t *)system_variables_enum_info;
	if (is_external_kernel()) 
	{
		if ((sys_vars_info->signature[0] != 0) && (sys_enum_info->signature[0] != 0))
		{
			if (strncmp(sys_vars_info->signature, "VARS", 4) != 0)
			{
				panic((unsigned long)system_variables_info);
			}
			if (sys_vars_info->version != 1)
			{
				panic((unsigned long)system_variables_info);
			}
			if (sys_vars_info->id[0] != 0)
			{
				if (uuidv4_validate(sys_vars_info->id))
				{
					/*
					if (has_enum(SYSTEM_VERBOSE))
					{
						printk("System Variables UUID: %s\n", sys_vars_info->id);
					}
					*/
					if (sys_vars_info->checksum != get_sys_vars_checksum())
					{
						printk("Checksum Error\n");
						printk("Variables are not configured correctly.\n");
						panic((unsigned long)sys_vars_info->checksum);
					}
				}
				else
				{
					panic((unsigned long)sys_vars_info->id);
				}
			}
			if (strncmp(sys_enum_info->signature, "ENUM", 4) != 0)
			{
				panic((unsigned long)system_variables_enum_info);
			}
			if (sys_enum_info->version != 1)
			{
				panic((unsigned long)system_variables_enum_info);
			}
			if (sys_enum_info->id[0] != 0)
			{
				if (uuidv4_validate(sys_enum_info->id))
				{
					/*
					if (has_enum(SYSTEM_VERBOSE))
					{
						printk("System Enumeration UUID: %s\n", sys_enum_info->id);
					}
					*/
					if (sys_enum_info->checksum != get_sys_enum_checksum())
					{
						printk("Checksum Error\n");
						printk("Enumerations are not configured correctly.\n");
						panic((unsigned long)sys_enum_info->checksum);
					}
				}
				else
				{
					panic((unsigned long)sys_enum_info->id);
				}
			}
		}
		else
		{
			if (sys_vars_info->signature[0] == 0)
			{
				strcpy(sys_vars_info->signature, "VARS");
				sys_vars_info->version = 1;
			}
			if (sys_enum_info->signature[0] == 0)
			{
				strcpy(sys_enum_info->signature, "ENUM");
				sys_enum_info->version = 1;
			}		
			memset(cmdline, 0, 65536);
			memset(hypervisor, 0, 65536);
			get_external_kernel_cmdline(cmdline);
			get_external_kernel_hypervisor(hypervisor);
			int cmdlinelen = strlen(cmdline)+1;
			int hypervisorlen = strlen(hypervisor)+1;
			extern_kernel.cmdline = (char*)malloc(cmdlinelen);
			extern_kernel.hypervisor = (char*)malloc(hypervisorlen);
			memset(extern_kernel.cmdline, 0, 65536);
			memset(extern_kernel.hypervisor, 0, 65536);
			strcpy(extern_kernel.cmdline, cmdline);
			strcpy(extern_kernel.hypervisor, hypervisor);
			int k_argc = getargc(extern_kernel.cmdline);			
			if (k_argc > 0)
			{
				for(int argcount=0;argcount<k_argc;argcount++)
				{
					char *k_tmp_argv = (char*)malloc(256);
					memset(k_tmp_argv, 0, 256);
					getargv(k_tmp_argv, extern_kernel.cmdline, argcount);
					strcpy(k_argv[argcount], k_tmp_argv);
					free(k_tmp_argv);
				}
				strcpy(kernelfilename, k_argv[0]);
				push_module(kernelfilename);
				int filenamelen = strlen(k_argv[0])+1;		
				extern_kernel.filename = (char*)malloc(filenamelen);	
				strcpy(extern_kernel.filename, k_argv[0]);	
				if (k_argc > 1)
				{
					int j=0;
					extern_kernel.paramcount = k_argc-1;
					extern_kernel.params = (char**)malloc(extern_kernel.paramcount*256*255);
					for(int i=1;i<k_argc;i++)
					{
						int paramlen = strlen(k_argv[i])+1;
						extern_kernel.params[j] = (char*)malloc(paramlen);
						strcpy(extern_kernel.params[j], k_argv[i]);
						char p[256];
						char *v = (char*)malloc(256);
						char p_k[256];
						char p_v[256];
						strcpy(p, k_argv[i]);
						const char*t_p = (const char*)p;
						int l_p = 0;
						while(*t_p)
						{
							if (*t_p == '=') 
							{
								p[l_p] = '\0';
								break;
							}
							l_p++;
							t_p++;
						}
						strcpy(p_k, p);
						strcpy(p_v, v);
						int p_v_l = strlen(p_v)-1;
						p_k[p_v_l] = '\0';
						strcpy(p_v, &k_argv[i][l_p+1]);
						// printk("sys_vars[\"%s\"] = %s\n", p_k, p_v);
						add_sys_var(p_k, p_v);
						j++;
						free(v);
					}					
				}
				sys_vars_info->checksum = get_sys_vars_checksum();
				uuidv4(sys_vars_info->id);
				if (!uuidv4_validate(sys_vars_info->id))
				{
					panic((unsigned long)sys_vars_info->id);
				}
			}
		}
		sys_vars_loaded = 1;
		sys_enum_loaded = 1;
	}
	else
	{
		if (strncmp(sys_vars_info->signature, "VARS", 4) != 0)
		{
			panic((unsigned long)system_variables_info);
		}
		if (sys_vars_info->version != 1)
		{
			panic((unsigned long)system_variables_info);
		}
		if (sys_vars_info->id[0] != 0)
		{
			if (uuidv4_validate(sys_vars_info->id))
			{
				/*
				if (has_enum(SYSTEM_VERBOSE))
				{
					printk("System Variables UUID: %s\n", sys_vars_info->id);
				}
				*/
				if (sys_vars_info->checksum != get_sys_vars_checksum())
				{
					printk("Checksum Error\n");
					printk("Variables are not configured correctly.\n");
					panic((unsigned long)sys_vars_info->checksum);
				}
			}
			else
			{
				panic((unsigned long)sys_vars_info->id);
			}
		}
		if (strncmp(sys_enum_info->signature, "ENUM", 4) != 0)
		{
			panic((unsigned long)system_variables_enum_info);
		}
		if (sys_enum_info->version != 1)
		{
			panic((unsigned long)system_variables_enum_info);
		}
		if (sys_enum_info->id[0] != 0)
		{
			if (uuidv4_validate(sys_enum_info->id))
			{
				/*
				if (has_enum(SYSTEM_VERBOSE))
				{
					printk("System Enumeration UUID: %s\n", sys_enum_info->id);
				}
				*/
				if (sys_enum_info->checksum != get_sys_enum_checksum())
				{
					printk("Checksum Error\n");
					printk("Enumerations are not configured correctly.\n");
					panic((unsigned long)sys_enum_info->checksum);
				}
			}
			else
			{
				panic((unsigned long)sys_enum_info->id);
			}
		}
		sys_vars_loaded = 1;
		sys_enum_loaded = 1;		
	}
}

void list_sys_vars(void)
{
	int i;
	char var_name[128];
	char var_value[128];
	sys_vars = (sys_vars_t *)system_variables;
	sys_vars_info = (sys_vars_info_t *)system_variables_info;
	for(i=0;i<sys_vars_info->count;i++)
	{
		strcpy(var_name, sys_vars[i].name);
		if (strlen(var_value) == 0)
		{
			strcpy(var_value, "0");
		}
		else
		{
			strcpy(var_value, sys_vars[i].value);
		}
		if (strlen(var_name) != 0)
		{
			printk("sys_vars[\"%s\"] = %s\n", var_name, var_value);
		}
	}
}

void list_sys_enum(void)
{
	int i;
	char enum_name[65];
	int enum_value;
	sys_enum = (sys_enum_t *)system_variables_enum;
	sys_enum_info = (sys_enum_info_t *)system_variables_enum_info;
	for(i=0;i<sys_enum_info->count;i++)
	{
		strcpy(enum_name, sys_enum->name[i]);
		enum_value = sys_enum->value[i];
		if (strlen(enum_name) != 0)
		{
			printk("sys_enum[\"%s\"] = %d\n", enum_name, enum_value);
		}
	}
}

void init_variables(void)
{
	int i=0;
	char var_name[128];
	char var_value[128];
	char enum_name[65];
	int enum_value;
	sys_vars = (sys_vars_t *)system_variables;
	sys_vars_info = (sys_vars_info_t *)system_variables_info;
	sys_enum = (sys_enum_t *)system_variables_enum;
	sys_enum_info = (sys_enum_info_t *)system_variables_enum_info;
	/*
	if (has_enum(SYSTEM_VERBOSE))
	{
		list_sys_vars();
		list_sys_enum();
	}
	*/
}

void reloadvars(void)
{
	loadvars();
}

void reloadsysteminfo(void)
{
	unsigned long physical_memory_l;
	unsigned long physical_memory;
	unsigned long used_memory;
	unsigned long used_memory_percent;
	char *cpu_id_name;
	char *cpu_id_str;
	char *cpu_brand_str;
	int cpu_id_type, cpu_id_family, cpu_id_model, cpu_id_stepping, cpu_id_longmode;
	if (!is_external_kernel()) return;
	
	cpu_speed = get_cpu_speed();
	get_cpu_info(&cpu_id_type, &cpu_id_family, &cpu_id_model, &cpu_id_stepping);
	cpu_id_longmode = has_cpu_support_longmode();

	physical_memory_l = probememory();
	physical_memory = physical_memory_l / 1024 / 1024;
	used_memory = _heap_size / 1024 / 1024;
	used_memory_percent = (100/(physical_memory/used_memory));

	cpu_id_name = (char*)malloc(32);
	cpu_id_str = (char*)malloc(64);
	cpu_brand_str = (char*)malloc(256);

	memset(cpu_id_name, 0, 32);
	memset(cpu_id_str, 0, 64);
	memset(cpu_brand_str, 0, 256);

	get_cpu_vendor_id_string(cpu_id_name);
	get_cpu_vendor_string(cpu_id_str);
	get_cpu_brand_str(cpu_brand_str);
	
	info = (system_info_t*)system_info;
	strcpy(info->signature, "INFO");
	info->version = 1;
	info->flags = 0;
	info->unused = 0;
	info->heap_1_size = _heap_size;
	info->heap_2_size = 0;
	info->heap_3_size = 0;
	info->heap_4_size = 0;
	info->physical_memory = physical_memory_l;
	info->cpu_speed = cpu_speed;
	info->cpu_id_type = cpu_id_type;
	info->cpu_id_family = cpu_id_family;
	info->cpu_id_model = cpu_id_model;
	info->cpu_id_stepping = cpu_id_stepping;
	info->cpu_id_longmode = cpu_id_longmode;
	memset(info->cpu_id_name, 0, 32);
	memset(info->cpu_id_str, 0, 64);
	memset(info->cpu_brand_str, 0, 256);
	memset(info->reserved, 0, 104);
	strcpy(info->cpu_id_name, cpu_id_name);
	strcpy(info->cpu_id_str, cpu_id_str);
	strcpy(info->cpu_brand_str, cpu_brand_str);
	strcpy(info->reserved, "");
}

void loadsysteminfo(void)
{
	if (is_external_kernel()) return;
	info = (system_info_t*)system_info;
	info->multi_boot = multiboot_address;
	if (info->multi_boot != 0)
	{
		printk("Multiboot enabled.\n");
		printk("Multiboot info address: 0x%X\n", info->multi_boot);
		multiboot_info = (multiboot_info_t*)info->multi_boot;
		if (multiboot_info->module_count == 1)
		{
			printk("Module loaded.\n");
			multiboot_module_t* modules = (multiboot_module_t*)multiboot_info->module_address;
			unsigned int module_start = modules->module_start;
	    	typedef void (*multiboot_module)(void);
	    	multiboot_module start_module = (multiboot_module)module_start;
	    	start_module();
		}
	}
	if (strncmp(info->signature, "INFO", 4) != 0)
	{
		panic((unsigned long)system_info);
	}
	if (info->version != 1)
	{
		panic((unsigned long)system_info);
	}
	cpu_speed = info->cpu_speed;
}

void kernelmode_preinit(void)
{
	enter_kernelmode = 1;
	push_module(strlwr(kernelfilename));
}

void kernelmode_init(void)
{
	ata_delay = 10;
	usermode = 0;
	kernelmode = 1;
	enter_kernelmode = 0;
	printk("Kernel Mode are initialized.\n");
}

void usermode_init(void)
{
	ata_delay = 10;
	usermode = 1;
	kernelmode = 0;
	enter_kernelmode = 0;
	printk("User Mode are initialized.\n");
}

char *getcwd_sector(unsigned long sector)
{
	unsigned long root_sector_start = getrootdirsector();
	
	strcpy(mount_p, "/");
	
	if (mbr_active)
		strcpy(root_dev, "/dev/hd1");
	else
		strcpy(root_dev, "/dev/hd");
	
	if (sector == root_sector_start)
	{
		strcpy(dir_path, mount_p);
		if (has_var("ROOT")) 
		{
			if (strcmp(get_var("ROOT"), root_dev) == 0)
			{
				if (has_var("MOUNT_POINT"))
				{
					if (strcmp(get_var("MOUNT_POINT"), mount_p) == 0)
					{
						return get_var("MOUNT_POINT");
					}
					else
					{
						printk("Cannot mount point \"%s\"\n", get_var("MOUNT_POINT"));
						printk("Please set a correct system variable \"MOUNT_POINT=\"\n");
						panic((unsigned long)get_var("MOUNT_POINT"));
					}
				}
				else
					return (char*)dir_path;
			}
			else
			{
				printk("Cannot open root device \"%s\"\n", get_var("ROOT"));
				printk("Please set a correct system variable \"ROOT=\"\n");
				panic(root_sector_start);
			}
		}
		return (char*)dir_path;
	}
	else
	{
		getfilepathfromsector(sector, dir_path);
		return (char*)dir_path;
		/*
		if (is_dir(dir_path))
		{
			return (char*)dir_path;
		}
		else
		{
			strcpy(dir_path, mount_p);
			return (char*)dir_path;
		}
		*/
	}
	
	return "/no-where";
}

void srand(unsigned int seed)
{
    rand_next = seed;
}

int rand(void)
{
    int r;
    rand_next = ((rand_next * 1103515245) + 12345);
    r = ((unsigned int)(rand_next / 65536) % 32768);
    return r;
}

void HexToStr(unsigned long val, char *str)
{
    char table[17] = {"0123456789abcdef"};
    char hex[8];
    int i=val;
    char *s = str;
    char *rev = s;
    if (val == 0)
    {
        *s++ = '0';
    }    
    while(i > 0)
    {
        *s++ = table[(i % 16)];
        i /= 16;
    }
    *s-- = '\0';
    while(rev < s)
    {
        char ch = *rev;
        *rev++ = *s;
        *s-- = ch;
    }
}

unsigned long StrToHex(char* str)
{
    unsigned long hex,val;
    int n;
    hex=0;
    for(n=0;n<strlen(str);n++) 
    {
		if(str[n] >= '0' && str[n] <= '9')
		{
			val = (unsigned long)(str[n]-'0');
		}
		else if(str[n] >= 'a' && str[n] <= 'f')
		{
			val = ((unsigned long)(str[n]-'a')) + 0xA;
		}
		else if(str[n] >= 'A' && str[n] <= 'F')
		{
			val = ((unsigned long)(str[n]-'A')) + 0xA;
		}
		else
		{
			val = 0;
		}
		hex <<= 4;
		hex |= val;
    }
    return hex;
}

void uuidv4(char *str)
{
    char uuid[37];
    char base[37];
    strcpy(base, "10000000-1000-4000-8000-100000000000");
    int len = strlen(base);
    int slen=0;
    uuid[0] = '\0'; 
    for(int i=0;i<len;i++)
    {
        if ((base[i] >= '0') && (base[i] <= '9'))
        {
            char ch[2];
            ch[0] = base[i];
            ch[1] = '\0';
            unsigned char n = StrToHex(ch);
            unsigned char c = (n ^ rand() & (15 >> (n / 4)));
            if (slen == 12) c = ((base[slen+2] - '0') % 10);
            char hex[2];
            HexToStr(c, hex);
            strcat(uuid, hex);
            if (slen == 7) strcat(uuid, "-");
            if (slen == 11) strcat(uuid, "-");
            if (slen == 15) strcat(uuid, "-");
            if (slen == 19) strcat(uuid, "-");
            slen++;
        }
    }
    slen += 4;
    uuid[slen] = '\0';     
    strcpy(str, uuid);
}

int uuidv4_validate(const char *str)
{
    int r=0, v=0, i=0;
    if (!str) return 0;
    if (strlen(str) != 36) return 0;
    while(*str)
    {
        if ((i == 8) && (*str != '-')) v--;
        if ((i == 13) && (*str != '-')) v--;
        if ((i == 18) && (*str != '-')) v--;
        if ((i == 23) && (*str != '-')) v--;
        if (((*str >= '0') && (*str <= '9')) || 
        ((*str >= 'a') && (*str <= 'f')) || 
        ((*str >= 'A') && (*str <= 'F')))
        {
            if (((i >= 0) && (i <= 7)) || 
                ((i >= 9) && (i <= 12)) || 
                ((i >= 14) && (i <= 17)) || 
                ((i >= 19) && (i <= 22)) || 
                ((i >= 24) && (i <= 36)))
            {
                if ((i == 14) && (*str != '4')) v--;
                if ((i == 19) && 
                ((*str != '8' && (*str != '9' && 
                (*str != 'a' && (*str != 'b')) && 
                (*str != 'A' && (*str != 'B')))))) v--;
                v++;
            }
        }
        i++;
        str++;
    }
    if (v == 32) r = 1;
    return r;
}

void getargv(char *s1, const char *s2, int n)
{
	char *tmp_argvx = s1;
	int tmp_argc=0;
	while(*s2)
	{
		if (*s2 == ' ') 
		{
			*tmp_argvx = '\0';
			tmp_argc++;
		}
		else 
		{
			if (tmp_argc == n)
			{
				*tmp_argvx++ = *s2;
			}
		}
		s2++;
	}
	*tmp_argvx = '\0';
}

int getargc(const char *s)
{
    int r=0;
    if (*s) r = 1;
	while(*s)
	{
		if (*s == ' ') 
		{
			r++;
		}
		s++;
	}
	return r;
}

void test_realmode(void)
{
	registers16_t r;
	memset(&r, 0, sizeof(registers16_t));
	r.ax = UINT16(0,0x0f);
	int86(0x10, &r, &r);
	printk("int86:\n");
	printk("Video Mode: 0x%X\n", r.ax & 0xff);
	printk("AX: 0x%X, BX: 0x%X, CX: 0x%X, DX: 0x%X\n", r.ax, r.bx, r.cx, r.dx);
	printk("SP: 0x%X, BP: 0x%X, SI: 0x%X, DI: 0x%X\n", r.sp, r.bp, r.si, r.di);
	printk("DS: 0x%X, ES: 0x%X, FS: 0x%X, GS: 0x%X\n", r.ds, r.es, r.fs, r.gs);
	printk("FLAGS: 0x%X\n", r.flags);
	printk("\n");
}

void reload_devices(void)
{
	disable_interrupt();
	remap_irq();
	enable_interrupt();
	init_timer(timer_frequency);
	keyboard_restart();
	keyboard_flush();
}

unsigned long pci_config_address(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset)
{
	unsigned long l_bus = (unsigned long)bus;
	unsigned long l_slot = (unsigned long)slot;
	unsigned long l_func = (unsigned long)func;
	unsigned long l_addr = (unsigned long)((l_bus << 16) | (l_slot << 11) | (l_func << 8) | (offset & 0xfc) | ((unsigned long)0x80000000));
	return l_addr;
}

unsigned char pci_read_byte(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset)
{
	unsigned long addr = pci_config_address(bus, slot, func, offset);
	unsigned char r;
	outl(0x0CF8, (unsigned long)addr);
	r = inb(0x0CFC + (offset&3));
	return r;
}

unsigned short pci_read_word(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset)
{
	unsigned long addr = pci_config_address(bus, slot, func, offset);
	unsigned short r;
	outl(0x0CF8, (unsigned long)addr);
	r = inw(0x0CFC + (offset&2));
	return r;
}

unsigned long pci_read_long(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset)
{
	unsigned long addr = pci_config_address(bus, slot, func, offset);
	unsigned long r;
	outl(0x0CF8, (unsigned long)addr);
	r = inl(0x0CFC);
	return r;
}

void pci_write_byte(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset, unsigned char value)
{
	unsigned long addr = pci_config_address(bus, slot, func, offset);
	outl(0x0CF8, (unsigned long)addr);
	outb(0x0CFC + (offset&3), value);
}

void pci_write_word(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset, unsigned short value)
{
	unsigned long addr = pci_config_address(bus, slot, func, offset);
	outl(0x0CF8, (unsigned long)addr);
	outw(0x0CFC + (offset&2), value);
}

void pci_write_long(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset, unsigned long value)
{
	unsigned long addr = pci_config_address(bus, slot, func, offset);
	outl(0x0CF8, (unsigned long)addr);
	outl(0x0CFC, value);
}

pci_class_name_t pci_class_name[] = 
{
	{0x00, 0x00, "Non-VGA-Compatible Unclassified Device"},
	{0x00, 0x01, "VGA-Compatible Unclassified Device"},
	{0x01, 0x00, "SCSI Bus Controller"},
	{0x01, 0x01, "IDE Controller"},
	{0x01, 0x02, "Floppy Disk Controller"},
	{0x01, 0x03, "IPI Bus Controller"},
	{0x01, 0x04, "RAID Controller"},
	{0x01, 0x05, "ATA Controller"},
	{0x01, 0x06, "Serial ATA Controller"},
	{0x01, 0x07, "Serial Attached SCSI Controller"},
	{0x01, 0x08, "Non-Volatile Memory Controller"},
	{0x01, 0x80, "Other Mass Storage Controller"},
	{0x02, 0x00, "Ethernet Controller"},
	{0x02, 0x01, "Token Ring Controller"},
	{0x02, 0x02, "FDDI Controller"},
	{0x02, 0x03, "ATM Controller"},
	{0x02, 0x04, "ISDN Controller"},
	{0x02, 0x05, "WorldFip Controller"},
	{0x02, 0x06, "PICMG 2.14 Multi Computing Controller"},
	{0x02, 0x07, "Infiniband Controller"},
	{0x02, 0x08, "Fabric Controller"},
	{0x02, 0x80, "Other Network Controller"},
	{0x03, 0x00, "VGA Compatible Controller"},
	{0x03, 0x01, "XGA Controller"},
	{0x03, 0x02, "3D Controller (Not VGA-Compatible)"},
	{0x03, 0x80, "Other Display Controller"},
	{0x04, 0x00, "Multimedia Video Controller"},
	{0x04, 0x01, "Multimedia Audio Controller"},
	{0x04, 0x02, "Computer Telephony Device"},
	{0x04, 0x03, "Audio Device"},
	{0x04, 0x80, "Other Multimedia Controller"},
	{0x05, 0x00, "RAM Controller"},
	{0x05, 0x01, "Flash Controller"},
	{0x05, 0x80, "Other Memory Controller"},
	{0x06, 0x00, "Host Bridge"},
	{0x06, 0x01, "ISA Bridge"},
	{0x06, 0x02, "EISA Bridge"},
	{0x06, 0x03, "MCA Bridge"},
	{0x06, 0x04, "PCI-to-PCI Brige"},
	{0x06, 0x05, "PCMCIA Bridge"},
	{0x06, 0x06, "NuBus Bridge"},
	{0x06, 0x07, "CardBus Bridge"},
	{0x06, 0x08, "RACEway Bridge"},
	{0x06, 0x09, "PCI-to-PCI Bridge"},
	{0x06, 0x0A, "Infiniband-to-PCI Host Bridge"},
	{0x06, 0x80, "Other Bridge"},
	{0x07, 0x00, "Serial Controller"},
	{0x07, 0x01, "Parallel Controller"},
	{0x07, 0x02, "Multiport Serial Controller"},
	{0x07, 0x03, "Modem"},
	{0x07, 0x04, "IEEE 488.1/2 (GPIB) Controller"},
	{0x07, 0x05, "Smart Card Controller"},
	{0x07, 0x80, "Other Simple Communication Controller"},
	{0x08, 0x00, "PIC"},
	{0x08, 0x01, "DMA Controller"},
	{0x08, 0x02, "Timer"},
	{0x08, 0x03, "RTC Controller"},
	{0x08, 0x04, "PCI Hot-Plug Controller"},
	{0x08, 0x05, "SD Host Controller"},
	{0x08, 0x07, "IOMMU"},
	{0x08, 0x80, "Other Base System Peripheral"},
	{0x09, 0x00, "Keyboard Controller"},
	{0x09, 0x01, "Digitizer Pen"},
	{0x09, 0x02, "Mouse Controller"},
	{0x09, 0x03, "Scanner Controller"},
	{0x09, 0x04, "Gameport Controller"},
	{0x09, 0x80, "Other Input Device Controller"},
	{0x0A, 0x00, "Generic Docking Station"},
	{0x0A, 0x80, "Other Docking Station"},
	{0x0B, 0x00, "386 Processor"},
	{0x0B, 0x01, "486 Processor"},
	{0x0B, 0x02, "Pentium Processor"},
	{0x0B, 0x03, "Pentioum Pro Processor"},
	{0x0B, 0x10, "Alpha Processor"},
	{0x0B, 0x20, "PowerPC Processor"},
	{0x0B, 0x30, "MIPS Processor"},
	{0x0B, 0x40, "Co-Processor"},
	{0x0B, 0x80, "Other Processor"},
	{0x0C, 0x00, "FireWire (IEEE 1394) Controller"},
	{0x0C, 0x01, "ACCESS Bus Controller"},
	{0x0C, 0x02, "SSA"},
	{0x0C, 0x03, "USB Controller"},
	{0x0C, 0x04, "Fibre Channel"},
	{0x0C, 0x05, "SMBus Controller"},
	{0x0C, 0x06, "InfiniBand Controller"},
	{0x0C, 0x07, "IPMI Interface"},
	{0x0C, 0x08, "SERCOS Interface (IEC 61491)"},
	{0x0C, 0x09, "CANbus Controller"},
	{0x0C, 0x80, "Other Serial Bus Controller"},
	{0x0D, 0x00, "iRDA Compatible Controller"},
	{0x0D, 0x00, "Consumer IR Controller"},
	{0x0D, 0x00, "RF Controller"},
	{0x0D, 0x00, "Bluetooth Controller"},
	{0x0D, 0x00, "Broadband Controller"},
	{0x0D, 0x00, "Ethernet Controller (802.1a)"},
	{0x0D, 0x00, "Ethernet Controller (802.1b)"},
	{0x0D, 0x00, "Other Wireless Controller"},
	{0x0E, 0x00, "I20"},
	{0x0F, 0x01, "Satellite TV Controller"},
	{0x0F, 0x02, "Satellite Audio Controller"},
	{0x0F, 0x03, "Satellite Voice Controller"},
	{0x0F, 0x04, "Satellite Data Controller"},
	{0x10, 0x00, "Network and Computing Encryption/Decryption"},
	{0x10, 0x10, "Entertainment Encryption/Decryption"},
	{0x10, 0x80, "Other Encryption Controller"},
	{0x11, 0x00, "DPIO Modules"},
	{0x11, 0x01, "Performance Counters"},
	{0x11, 0x10, "Communication Synchronizer"},
	{0x11, 0x20, "Signal Processing Management"},
	{0x11, 0x80, "Other Signal Processing Controller"},
};

const char *get_pci_class_name(unsigned char class, unsigned char subclass) 
{
	const char *unknown_class_name = "Unknown PCI Device";
	size_t i;
	size_t class_list_size = (sizeof(pci_class_name)/sizeof(pci_class_name_t));
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
			for (i = 0; i < class_list_size; i++) 
			{
				if (pci_class_name[i].class == class && pci_class_name[i].subclass == subclass) 
				{
					return pci_class_name[i].name;
				}
			}
		}
		break;
	}
	return unknown_class_name;
}

unsigned char pci_scan_device(unsigned char bus, unsigned char slot, unsigned char func, unsigned char index)
{
	unsigned short pci_vendor = pci_read_word(bus, slot, func, 2);
	if (index >= 32) return 0;
	if (pci_vendor == 0xFFFF)
	{
		return 0;
	}
	unsigned short *pci_buf = (unsigned short*)&pci_device[index].pci;
	for(unsigned char i=0;i<32;i++)
	{
		pci_buf[i] = pci_read_word(bus, slot, func, i*2);
	}
	unsigned char base_count=0;
	switch(pci_device[index].pci.header_type)
	{
		case 0:
		{
			base_count = 6;
		}
		break;
		case 1:
		{
			base_count = 2;
		}
		break;
		case 2:
		{
			base_count = 1;
			pci_scan_device(pci_read_byte(bus, slot, func, 0x18), slot, func, index);
			pci_count++;
		}
		break;
	};	
	if (base_count > 0)
	{
		for(int j=0;j<base_count;j++)
		{
			int is64=0;
			int pf=0;
			unsigned long x = pci_read_long(bus, slot, func, 16 + (j * 4));
			if ((!x) || (x == (unsigned long)~0))
			{
				continue;
			}
			if ((x & 0x01) == 0x01)
			{
				pci_device[index].pci.bar[j] = x;
			}
			else
			{
				if ((x & 0x06) != 0x04)
				{
					pci_device[index].pci.bar[j] = x;
				}
				else if (j == base_count-1)
				{
					// skip
				}
				else
				{
					unsigned long y = pci_read_long(bus, slot, func, 16 + ((++j) * 4));
					if (!y)
					{
						pci_device[index].pci.bar[j-1] = x;
					}
					else
					{
						is64=1;
					}
				}
			}
			unsigned long bar_x = pci_device[index].pci.bar[j];
			unsigned long bar_y;
			if (is64) 
			{
				bar_y = bar_x;
			}
			else
			{
				bar_y = ((~(bar_x & (~0x7ff))) + 1) & 0xffffffff;
				bar_y &= 0xFFFFFFF0;
				bar_y &= 0xFFFFFFFC;
			}
			if ((bar_y & 0x08) == 0x00)
			{
				if ((bar_y & 0xFF0000) == 0)
				{
					unsigned int bar_m_pre_fetch = -bar_y;
					// prefetch memory
					pci_device[index].pci.bar[j] = bar_m_pre_fetch;
					pf=1;
					if ((pci_device[index].pci.class == 0x03) && (pci_device[index].pci.subclass == 0x00))
					{
						if (pci_video_memory_found == 0)
						{
							pci_video_memory_address = bar_y;
							pci_video_memory_found = 1;
						}
					}
				}
			}
			if (!pf)
			{			
				if (is64) 
				{
					pci_device[index].pci.bar[j] = bar_y;
				}
				else
				{
					pci_device[index].pci.bar[j] &= 0xFFFFFFF0;
					pci_device[index].pci.bar[j] &= 0xFFFFFFFC;				
				}
			}
		}
	}
	pci_device[index].bus = bus;
	pci_device[index].slot = slot;
	pci_device[index].function = func;
	return 1;
}

void pci_read_data(unsigned char bus, unsigned char slot, unsigned char func, pci_t *pci)
{
	unsigned short pci_vendor = pci_read_word(bus, slot, func, 2);
	if (pci_vendor == 0xFFFF)
	{
		return;
	}
	unsigned short *pci_buf = (unsigned short*)pci;
	for(unsigned char i=0;i<32;i++)
	{
		pci_buf[i] = pci_read_word(bus, slot, func, i*2);
	}
}

void pci_write_data(unsigned char bus, unsigned char slot, unsigned char func, pci_t *pci)
{
	unsigned short pci_vendor = pci_read_word(bus, slot, func, 2);
	if (pci_vendor == 0xFFFF)
	{
		return;
	}
	unsigned short *pci_buf = (unsigned short*)pci;
	for(unsigned char i=0;i<32;i++)
	{
		pci_write_word(bus, slot, func, i*2, pci_buf[i]);
	}
}

void pci_scan_bus(unsigned char bus)
{
	unsigned char i;
	for (i=0;i<32;i++)
	{
		if (pci_scan_device(bus, i, 0, pci_count))
		{
			for(int j=0;j<8;j++)
			{
				if (pci_scan_device(bus, i, j, pci_count))
				{
					pci_count++;
				}
			}
		}
	}
}

void pci_scan(void)
{
	int i;
	pci_scan_bus(0);
	if ((pci_device[0].pci.header_type & 0x80) != 0x00)
	{
		for(i=1;i<256;i++)
		{
			pci_scan_bus(i);
		}
	}
}

void loadpci(void)
{
	int i;
	for(i=0;i<32;i++)
	{
		memset(&pci_device[i].pci, 0, sizeof(pci_t));
		pci_device[i].pci.vendor = 0xFFFF;
	}
	pci_scan();
	system_pci = (unsigned char *)0x840300;
	memset(system_pci, 0, 0x1000);
	sys_pci = (system_pci_t*)system_pci;
	strcpy(sys_pci->signature, "PCI");
	sys_pci->version = 1;
	sys_pci->count = pci_count;
	memcpy(&sys_pci->device[0], &pci_device[0], sizeof(pci_device_t)*32);
}

unsigned char is_module_stack_empty(void)
{
    if (module_stack_top == -1)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

unsigned char is_module_stack_full(void)
{
    if (module_stack_top >= MODULE_STACK_SIZE)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

char* get_current_module(void)
{
    if (module_stack_top == -1)
    {
        return NULL;
    }
    else if (module_stack_top >= MODULE_STACK_SIZE)
    {
        return NULL;
    }
    else
    {
        return module_stack[module_stack_top];
    }
}

char * push_module(const char *module_name)
{
    if (!is_module_stack_full())
    {
        module_stack_top = module_stack_top + 1;
        strcpy(module_stack[module_stack_top], module_name);
        return module_stack[module_stack_top];
    }
    else
    {
        printk("Stack Overflow\n");
    }
    return NULL;
}

char *pop_module(void)
{
    char *module_data;
    if (!is_module_stack_empty())
    {
        module_data = module_stack[module_stack_top];
        module_stack_top = module_stack_top - 1;
        return module_data;
    }
    else
    {
        printk("Stack Underflow\n");
    }
    return NULL;
}

void init_module_stack(void)
{
    module_stack = (char**)malloc(MODULE_STACK_SIZE*MODULE_NAME_SIZE);
    for(int i=0;i<MODULE_STACK_SIZE;i++)
    {
        module_stack[i] = (char*)malloc(MODULE_NAME_SIZE);
    }
    
}

void uninit_module_stack(void)
{
    for(int i=0;i<MODULE_STACK_SIZE;i++)
    {
        free(module_stack[i]);
    }
    free(module_stack);
}


void loaddefines(void)
{
	if (!is_external_kernel()) 
	{
		push_module(kernelfilename);
	}
}

void cmdnotfound(char *app, char *name)
{
	if (strlen(app) != 0)
	{
		printk("%s: ", app);
	}
	printk("%s: command not found\n", name);
}

void notfound(char *app, char *name)
{
	if (strlen(app) != 0)
	{
		printk("%s: ", app);
	}
	printk("%s: No such file or directory\n", strlwr(name));
}

void notdirectory(char *app, char *name)
{
	if (strlen(app) != 0)
	{
		printk("%s: ", app);
	}
	printk("%s: Is not a directory\n", strlwr(name));
}

void isdirectory(char *app, char *name)
{
	if (strlen(app) != 0)
	{
		printk("%s: ", app);
	}
	printk("%s: Is a directory\n", strlwr(name));
}

void presskey(void)
{
	printk("Press any key.");
	getch();
	printk("\n");
}


unsigned long fat32_free_space_sectors(unsigned long free_count, unsigned long sectors_per_fat) 
{
    unsigned long free_space_sectors = (free_count * sectors_per_fat);
    return free_space_sectors;
}

unsigned long fat32_free_space(unsigned long free_count, unsigned long bytes_per_sector, unsigned long sectors_per_fat) 
{
    unsigned long free_space = (free_count * bytes_per_sector * sectors_per_fat);
    return free_space;
}

unsigned long fat32_total_space(unsigned long total_sectors, unsigned long bytes_per_sector) 
{
    unsigned long total_space = (total_sectors * bytes_per_sector);
    return total_space;
}

unsigned long fat32_used_space(unsigned long free_count, unsigned long total_sectors, unsigned long bytes_per_sector, unsigned long sectors_per_fat) 
{
    unsigned long free_space = fat32_free_space(free_count, bytes_per_sector, sectors_per_fat);
    unsigned long total_space = fat32_total_space(total_sectors, bytes_per_sector);
    unsigned long used_space = (total_space - free_space);
    return used_space;
}

unsigned long getclustercount2(unsigned long totalSectors) 
{
    unsigned long reservedSectors = fat->bpb.bpb1.reserved_sectors_count;
    unsigned long bytesPerSector = fat->bpb.bpb1.bytes_per_sector;
    unsigned long sectorsPerCluster = fat->bpb.bpb1.sector_per_cluster;

    unsigned long fatSize = totalSectors - reservedSectors;
    unsigned long clusterCount = fatSize / sectorsPerCluster;

    return clusterCount;
}


unsigned long get_total_files_size(unsigned long sector)
{
	int total_size = 0;
	int filesize = 0;
	int sectcount = 0;
	int sector_count = 0;
	int offset = 0;
	int fsector = 0;
	int fcluster = 0;
	unsigned long bytes_per_sector = fat->bpb.bpb1.bytes_per_sector;
	char *data = (char*)malloc(bytes_per_sector);
	char filename[32];
	int q = 0;	
	int max_size = fat->bpb.bpb1.total_sectors_32;
	while (q == 0)
	{
		sector_count = sector+sectcount;
		if (sector_count >= max_size)
		{
			q = 1;
			break;
		}
		if (!readsector(sector_count, data))
		{
			q = 1;
			break;
		}
		if (data == NULL)
		{
			q = 1;
			break;			
		}
		if (data[0] == 0) 
		{
			q = 1;
			break;
		}
		else
		{
			offset = 0;
			while(offset < bytes_per_sector)
			{
				void *filep = (void*)data+offset;
				file_entry_t *file = (file_entry_t*)filep;
				if (file == NULL)
				{
					q = 1;
					break;
				}
				if (file->name[0] == 0)
				{
					q = 1;
					break;					
				}
				if (((file->attribute != F_ATTR_LNGFNM) && (file->attribute != F_ATTR_VOLMID)) &&
				    (file->name[0] != FILE_NAME_DELETED) && (file->name[0] != FILE_NAME_DIRECTORY) &&
					(file->name[0] != 0x00) && (file->name[0] != 0xFF))
				{
					memset(filename, 0, 32);
					memcpy(filename, file->name, 11);
					if (filename[0] == 0)
					{
						q = 1;
						break;						
					}
					filesize = file->size;
					total_size += filesize;
					fcluster = getfilefirstcluster(*file);
					fsector = getfirstsectorofcluster(fcluster);	
					if (fsector >= max_size)
					{
						q = 1;
						break;
					}
					if (fcluster == 0xFFFF)
					{
						q = 1;
						break;
					}
					if (file->attribute == 0x10)
					{
						total_size += get_total_files_size(fsector);
					}				
				}
				offset += 32;
			}
		}
		sectcount++;
		sector_count = sector+sectcount;
		if (sector_count >= max_size)
		{
			q = 1;
			break;
		}
	}	
	free(data);
	return total_size;
}

int chdir(const char *path)
{
	unsigned char result = 0;
	unsigned char has_dir = 0;
	unsigned long sector = getcurrentdirsector();
	char file_name[256];
	memset(file_name, 0, 256);
	if (strlen(path) > 0)
	{
		if (is_dir(path))
		{
			file_dir_t *find_file = (file_dir_t *)malloc(sizeof(file_dir_t));
			if (findfile(path, find_file, &sector, 1, file_name))
			{
				has_dir = 1;
			}
			else
			{
				has_dir = 0;
			}
			free(find_file);
		}
	}	
	if (has_dir)
	{
		result = 1;
		setcurrentdirsector(sector);
	}
	else
	{
		result = 0;
	}
	return result;
}

void dump_hex(const void *data, size_t size) {
    const unsigned char *buffer = (const unsigned char *)data;
    size_t i, j;

    for (i = 0; i < size; i += 16) {
        printk("%06x: ", (unsigned int)i);

        for (j = 0; j < 16; j++) {
            if (i + j < size)
                printk("%02x ", buffer[i + j]);
            else
                printk("   ");

            if (j % 16 == 7)
                printk(" ");
        }

        printk("\n");
    }
}



void loadsets(void)
{
	root_sector = (unsigned char *)SYSTEM_ROOT_SECTOR;
	mbr_sector = (unsigned char *)SYSTEM_MBR_SECTOR;
	boot_sector = (unsigned char *)SYSTEM_BOOT_SECTOR;
	disk_address_packet = (unsigned char *)SYSTEM_DISK_ADDRESS_PACKET;
	vga_memory = (unsigned char *)SYSTEM_VGA_MEMORY;
	video_memory = (unsigned char *)SYSTEM_VIDEO_MEMORY;
	vesa_info_buffer = (unsigned char *)SYSTEM_VESA_INFO_BUFFER;
	vesa_mode_buffer = (unsigned char *)SYSTEM_VESA_MODE_BUFFER;
	fat32_fsinfo = (unsigned char *)SYSTEM_FAT32_FSINFO;
	loader_kernel = (unsigned char *)SYSTEM_LOADER_KERNEL;
	external_kernel = (unsigned char *)SYSTEM_EXTERNAL_KERNEL;
	kernel = (unsigned char *)SYSTEM_KERNEL;
	system_variables = (unsigned char *)SYSTEM_ADDRESS_VARIABLES;
	system_variables_info = (unsigned char *)SYSTEM_ADDRESS_VARIABLES_INFO;
	system_variables_enum = (unsigned char *)SYSTEM_ADDRESS_VARIABLES_ENUM;
	system_variables_enum_info = (unsigned char *)SYSTEM_ADDRESS_VARIABLES_ENUM_INFO;
	system_info = (unsigned char *)SYSTEM_ADDRESS_INFO;
	system_pci = (unsigned char *)SYSTEM_ADDRESS_PCI;
	system_errno = (unsigned char *)SYSTEM_ADDRESS_ERRNO;
	system_memory_map_info = (unsigned char *)SYSTEM_ADDRESS_MEMORY_MAP_INFO;
	system_memory_map = (unsigned char *)SYSTEM_ADDRESS_MEMORY_MAP;
	ahci_ptr = (unsigned char *)SYSTEM_AHCI_PTR;
}


void set_dma_config(unsigned char channel, unsigned long address, unsigned short count) 
{
    outb(0x0A, channel); // Set DMA channel
    outb(0x04, address & 0xFF); // Set low byte of address
    outb(0x04, (address >> 8) & 0xFF); // Set middle byte of address
    outb(0x04, (address >> 16) & 0xFF); // Set high byte of address
    outb(0x05, count & 0xFF); // Set low byte of count
    outb(0x05, (count >> 8) & 0xFF); // Set high byte of count
}

void init_adma() 
{
    unsigned long *adma_base = (unsigned long *)ADMA_BASE_ADDRESS;
    *adma_base = 0x01; // enabled
}

void set_adma_cmd_packet(adma_cmd_packet_t *cpb, unsigned long command, unsigned long address, unsigned long length) 
{
    cpb->command = command;
    cpb->address = address;
    cpb->length = length;
}

void transfer_adma(adma_cmd_packet_t *cpb) 
{
    unsigned long *adma_base = (unsigned long *)ADMA_BASE_ADDRESS;
    *adma_base = cpb->command;
    *(adma_base + 1) = cpb->address;
    *(adma_base + 2) = cpb->length;
}

void initialize_dma() 
{
    for (int i = 0;i<DMA_CHANNELS;i++) 
	{
        dma_channels[i].address = 0;
        dma_channels[i].page = 0;
        dma_channels[i].count = 0;
    }
}

void set_dma_channel(int channel, unsigned short address, unsigned char count) 
{
    if ((channel < 0) || (channel >= DMA_CHANNELS)) 
	{
        printk("Invalid DMA channel\n");
        return;
    }
    dma_channels[channel].address = address;
    dma_channels[channel].count = count;
    outb(DMA_PAGE_REGISTER, dma_channels[channel].page);
    outb(0x00 + channel, dma_channels[channel].address & 0xFF);
    outb(0x00 + channel + 1, (dma_channels[channel].address >> 8) & 0xFF);
    outb(0x04 + channel, dma_channels[channel].count);
}

void start_dma_transfer(int channel) 
{
    if ((channel < 0) || (channel >= DMA_CHANNELS)) 
	{
        printk("Invalid DMA channel\n");
        return;
    }
	// start
    outb(DMA_COMMAND_REGISTER, channel);
}

void stop_dma_transfer(int channel) 
{
    if ((channel < 0) || (channel >= DMA_CHANNELS))
	{
        printk("Invalid DMA channel\n");
        return;
    }
	// stop
    outb(DMA_COMMAND_REGISTER, channel|0x04);
}

void init_dma_channel(int channel) 
{
    set_dma_channel(channel, 0x1234, 0xFF);
    start_dma_transfer(channel);
    stop_dma_transfer(channel);
}

void dma_setup_prd(prd_t *prd, unsigned long address, unsigned short byte_count)
{
    prd->address = address;
    prd->byte_count = byte_count ? byte_count : 0xFFFF;
    prd->reserved = 0;
}

void dma_write_command_byte(unsigned short base, unsigned char command)
{
    *((volatile unsigned char *)(base + DMA_COMMAND_BYTE_OFFSET)) = command;
}

unsigned char dma_read_status_byte(unsigned short base)
{
    return *((volatile unsigned char *)(base + DMA_STATUS_BYTE_OFFSET));
}

void start_dma(unsigned short base)
{
    dma_write_command_byte(base, 0x01);
}

void stop_dma(unsigned short base)
{
   dma_write_command_byte(base, 0x00);
}

void dma_handle_error(unsigned short base)
{
    unsigned char status = dma_read_status_byte(base);
    if (status & 0x02) 
	{
        *((volatile unsigned char *)(base + DMA_STATUS_BYTE_OFFSET)) = 0x02;
    }
}

void dma_send_command(unsigned short base, unsigned char command)
{
    dma_write_command_byte(base, command);
}

void dma_ata_send_command(unsigned short command, unsigned short bus) 
{
    *((volatile unsigned short *)(bus + DMA_ATA_COMMAND_REGISTER)) = command;
}

unsigned short dma_ata_read_status(unsigned short bus) 
{
    return *((volatile unsigned short *)(bus + DMA_ATA_STATUS_REGISTER));
}

void dma_ata_clear_error(unsigned short bus) 
{
    *((volatile unsigned short *)(bus + DMA_ATA_STATUS_REGISTER)) = 0x02; // Clear error
}

void dma_ata_start(unsigned short bus, prd_t *prdt) 
{
    *((volatile unsigned long *)(bus + DMA_ATA_PRDT_ADDRESS_REGISTER)) = (unsigned long)prdt;
    *((volatile unsigned short *)(bus + DMA_ATA_COMMAND_REGISTER)) = 0x01; // Start DMA
}

void handle_dma_transfer(unsigned short bus, prd_t *prdt) 
{
    dma_ata_start(bus, prdt);
    unsigned short status = dma_ata_read_status(bus);
    if (status & 0x02) 
	{
        dma_ata_clear_error(bus);
    }
}

void loaddma(void)
{
    prd_t prd, prdt;
	// DMA Configuration
	// Load DMA Devices
	// ATA/ATAPI using DMA
    // ISA DMA
	// ADMA
	
	memset(&prd, 0, sizeof(prd_t));
	memset(&prdt, 0, sizeof(prd_t));
	
	// Initialize DMA	
    initialize_dma();
	init_dma_channel(1);

    dma_setup_prd(&prd, 0x1000, 0x2000);

    start_dma(PRIMARY_ATA_BUS);
    dma_send_command(PRIMARY_ATA_BUS, 0xC8);
    dma_handle_error(PRIMARY_ATA_BUS);
    stop_dma(PRIMARY_ATA_BUS);
	
    dma_setup_prd(&prdt, 0x1000, 0xFFFF);
    dma_ata_send_command(0xC8, PRIMARY_ATA_BUS);
    handle_dma_transfer(PRIMARY_ATA_BUS, &prdt);
	
	// Load Floppy Device
	//set_dma_config(0x06, 0x00000000, 0x0000);	
	
	// Initialize ADMA
	adma_cmd_packet_t cpb;
    init_adma();
    set_adma_cmd_packet(&cpb, 0x30, 0x1000, 512);
    transfer_adma(&cpb);
}


/*
unsigned int fsallocsize(unsigned int n)
{
	 return (unsigned int)(n*sizeof(void*));
}

#define fssizeof(n) fsallocsize(n)

void * _fsmalloc(const size_t n)
{
	 char *a;
	 char b[fssizeof(n)];
	 int i;
	
	 for (i=0;i<=n;i++)
	 {
		 b[i] = 0;
	 }
	 a = (void*)b;
	 for (i=0;i<=n;i++)
	 {
		 a[i] = 0;
	 }
   
	 return (void*)a;
}

size_t _fsfree(const void *a)
{
	int i,j;
	j = sizeof(a);
	for(i=0;i<j;i++)
	{
		a--;
	}
	a = NULL;

	return (size_t)a;
}
*/

/*
//---------------------------------------------------------------------------
unsigned long listdir0(char *fn)
{
        char lfn[256];
        unsigned char sector[SECTORSIZE];
        int next_dir_sector = 1;
        unsigned long addr = getrootlbaaddress();
	    memset(lfn, 0, 256);
        while (next_dir_sector != 0)
        {
                readsector(addr, sector);
                if (sector[0] == 0x00)
                {
                 next_dir_sector = 0;
                 break;
                }
                int next_dir_entry = 1;
                int dir_entry_address = 0;
                while (next_dir_entry != 0)
                {
                        if (dir_entry_address >= SECTORSIZE)
                        {
                         next_dir_entry = 0;
                         break;
                        }
                        unsigned char dir_entry_data[32];
                        memcpy(dir_entry_data,&sector[dir_entry_address],32);
                        file_entry_t *file_entry;
                        file_entry = (file_entry_t*)dir_entry_data;
                        if (file_entry->name[0] != 0)
                        {
                                if (file_entry->attribute == F_ATTR_VOLMID) // Volume ID
                                {
                                        // char volume_str[11];
                                        // memcpy(volume_str,file_entry->name,11);
                                        // volume_str[11] = '\0';
                                        // strcpy(volume_id,trim(volume_str));
                                        // StatusBar1->SimpleText += AnsiString(volume_str);									
                                }
                                else if (file_entry->attribute != F_ATTR_LNGFNM) // if not lfn
                                {                           
                                       char filename_s[11];
                                       char filename[12];
                                       memcpy(filename_s,file_entry->name,11);
                                       filename_s[11] = '\0';
                                       strfilenamedot8e3s11(filename_s,filename);
                                       if (strlen(lfn) > 0) printk("%s\n", lfn);
                                       else printk("%s\n", strlwr(filename));
                                       strcpy(lfn, "");
                                }
                                else // if is lfn
                                {
                                       char filename_s1[11];
                                       char filename[256];
                                       file_lfn_entry_t* file_entry_lfn = (file_lfn_entry_t*)dir_entry_data;
                                       memcpy(filename_s1,file_entry_lfn->name1,11);
                                       filename_s1[11] = '\0';
                                       int i = 1;
                                       int j = 0;
                                       while (i < 11)
                                       {
                                           if (((unsigned char)filename_s1[i] != (unsigned char)0x00) && ((unsigned char)filename_s1[i] != (unsigned char)0xFF))
                                           {
                                                filename[j] = filename_s1[i];
                                                j++;
                                           }
                                           i+=2;
                                       }
                                       char filename_s2[20];
                                       memcpy(filename_s2,file_entry_lfn->name2,20);
                                       filename_s2[19] = '\0';
                                       i = 0;
                                       while ((i < 20))
                                       {
                                           if ((unsigned char)filename_s2[i] != (unsigned char)0x00)
                                           {
                                                if ((unsigned char)filename_s2[i] != (unsigned char)0xFF)
                                                {
                                                        filename[j] = filename_s2[i];
                                                        j++;
                                                }
                                                else
                                                {
                                                        filename[j] = '\0';
                                                        break;
                                                }
                                           }
                                           i+=2;
                                       }
                                       filename[j] = '\0';
                                       strcatb(lfn, filename);
                                }
                        }
                        else
                        {
                           next_dir_entry = 0;
                           next_dir_sector = 0;
                           break;
                        }
                        dir_entry_address += 32;
                }
                addr++;
        }
		return 1;
}
//---------------------------------------------------------------------------
unsigned long listdir1(char *fn)
{
        char lfn[256];
        unsigned char sector[SECTORSIZE];
        int next_dir_sector = 1;
        unsigned long addr = getrootlbaaddress();
	    memset(lfn, 0, 256);
        while (next_dir_sector != 0)
        {
                readsector(addr, sector);
                if (sector[0] == 0x00)
                {
                 next_dir_sector = 0;
                 break;
                }
                int next_dir_entry = 1;
                int dir_entry_address = 0;
                while (next_dir_entry != 0)
                {
                        if (dir_entry_address >= SECTORSIZE)
                        {
                         next_dir_entry = 0;
                         break;
                        }
                        unsigned char dir_entry_data[32];
                        memcpy(dir_entry_data,&sector[dir_entry_address],32);
                        file_entry_t *file_entry;
                        file_entry = (file_entry_t*)dir_entry_data;
                        if (file_entry->name[0] != 0)
                        {
                                if (file_entry->attribute == F_ATTR_VOLMID) // Volume ID
                                {
									    //char volume_str[11];
                                        //memcpy(volume_str,file_entry->name,11);
                                        //volume_str[11] = '\0';
                                        //strcpy(volume_id,trim(volume_str));
                                        //StatusBar1->SimpleText += AnsiString(volume_str);									
                                }
                                else if (file_entry->attribute != F_ATTR_LNGFNM) // if not lfn
                                {                           
                                       char filename_s[11];
                                       char filename[12];
                                       memcpy(filename_s,file_entry->name,11);
                                       filename_s[11] = '\0';
                                       strfilenamedot8e3s11(filename_s,filename);
                                       if (strlen(lfn) > 0) printk("%s\n", lfn);
                                       else printk("%s\n", strlwr(filename));
                                       strcpy(lfn, "");
                                }
                                else // if is lfn
                                {
                                       char filename_s1[11];
                                       char filename[256];
                                       file_lfn_entry_t* file_entry_lfn = (file_lfn_entry_t*)dir_entry_data;
                                       memcpy(filename_s1,file_entry_lfn->name1,11);
                                       filename_s1[11] = '\0';
                                       int i = 1;
                                       int j = 0;
                                       while (i < 11)
                                       {
                                           if (((unsigned char)filename_s1[i] != (unsigned char)0x00) && ((unsigned char)filename_s1[i] != (unsigned char)0xFF))
                                           {
                                                filename[j] = filename_s1[i];
                                                j++;
                                           }
                                           i+=2;
                                       }
                                       char filename_s2[20];
                                       memcpy(filename_s2,file_entry_lfn->name2,20);
                                       filename_s2[19] = '\0';
                                       i = 0;
                                       while ((i < 20))
                                       {
                                           if ((unsigned char)filename_s2[i] != (unsigned char)0x00)
                                           {
                                                if ((unsigned char)filename_s2[i] != (unsigned char)0xFF)
                                                {
                                                        filename[j] = filename_s2[i];
                                                        j++;
                                                }
                                                else
                                                {
                                                        filename[j] = '\0';
                                                        break;
                                                }
                                           }
                                           i+=2;
                                       }
                                       filename[j] = '\0';
                                       strcatb(lfn, filename);
                                }
                        }
                        else
                        {
                           next_dir_entry = 0;
                           next_dir_sector = 0;
                           break;
                        }
                        dir_entry_address += 32;
                }
                addr++;
        }
		return 1;
}

//---------------------------------------------------------------------------
unsigned long listdir_sector2(unsigned long sector, char *file_name)
{
	int d;
	int q;
	unsigned long totalfiles;
	unsigned long entryoffset;
	unsigned long entrycount;
	unsigned long filecount;
	char shortfilename[12];
	char longfilename[256];
	char filename[256];
	file_entry_t *readdir_entries;
	file_entry_t* file;
	path_sub_t path_sub;
	if (!has_partition_active()) return 0;
	if (!isfattype()) return 0;
	entrycount = 0;
	filecount = 0;
	totalfiles = 0;
	entryoffset = 0;
	d = 0;
	q = 0;
	path_sub = getpath(strupr(file_name));
	strcpy(longfilename, "");	
	while(q == 0)
	{
		filecount = 0;
		file = getfileentryofsector(sector+entrycount);
		if (file == NULL)
		{
			q = 1;
			break;
		}
		while (filecount < 16)
		{
			if (file[filecount].name[0] == 0)
			{
				q = 1;
				break;
			}
			if (((unsigned char)file[filecount].name[0] != FILE_NAME_DELETED) && 
			    (file[filecount].attribute != F_ATTR_VOLMID))
			{
				if (file[filecount].attribute != F_ATTR_LNGFNM)
				{
					if ((file[filecount].name[0] == '.') && (file[filecount].name[1] == ' ') && (file[filecount].attribute & F_ATTR_DIRECT))
					{
						d++;
					}
					if (d > 1)
					{
						q = 1;
						break;
					}
					getshortfilename(file[filecount].name, shortfilename);
					if (strlen(longfilename) > 0)
					{
						strcpy(filename, longfilename);
					}
					else
					{
						strcpy(filename, strlwr(shortfilename));
					}
					if (strlen(file_name) > 0)
					{
						if (strcmp(strlwr(file_name), strlwr(filename)) == 0)
						{
							printk("%s\n", filename);
							strcpy(file_name, filename);
							strcpy(longfilename, "");
							return 1;
						}
					}
					else
					{
					   printk("%s\n", filename);				
					}
					strcpy(longfilename, "");
				}
				else
				{
					char filename_s1[11];
					char filename_s2[20];
					char lfn_s[256];
					file_lfn_entry_t *file_entry_lfn = (file_lfn_entry_t*)&file[filecount];
					memcpy(filename_s1, file_entry_lfn->name1, 11);
					filename_s1[11] = '\0';
					int a = 1;
					int b = 0;
					while(a < 11)
					{
						if ((filename_s1[a] != 0x00) && ((char)filename_s1[a] != (char)0xFF))
						{
							lfn_s[b] = filename_s1[a];
							b++;
						}
						a += 2;
					}
					memcpy(filename_s2, file_entry_lfn->name2, 20);
					filename_s2[19] = '\0';
					a = 0;
					while (a < 20)
					{
						if (filename_s2[a] != 0x00)
						{
							if ((char)filename_s2[a] != (char)0xFF)
							{
								lfn_s[b] = filename_s2[a];
								b++;
							}
							else
							{
								lfn_s[b] = '\0';
								break;
							}
						}
						a += 2;
					}
					lfn_s[b] = '\0';
					strcatb(longfilename, lfn_s);
				}
			}
			filecount++;
		}
		entrycount++;
	}
	return 0;	
}


unsigned char listdir2(const char *filename)
{
	unsigned char result = 0;
	unsigned char has_file = 1;
	unsigned long sector = getcurrentdirsector();
	char file_name[256];
	memset(file_name, 0, 256);
	if (strlen(filename) > 0)
	{
		file_dir_t *find_file = (file_dir_t *)malloc(sizeof(file_dir_t));
		if (findfile(filename, find_file, &sector, 1, file_name))
		{
			has_file = 1;
		}
		else
		{
			has_file = 0;
		}
		free(find_file);
	}	
	if (has_file)
	{
		result = 1;
		listdir_sector2(sector, file_name);
	}
	else
	{
		result = 0;
	}
	return result;
}
*/
	
int main(void)
{
	unsigned long physical_memory, physical_memory_l;
	unsigned long used_memory;
	unsigned long used_memory_percent;
	char *cpu_id_name;
	char *cpu_id_str;
	char *cpu_brand_str;
	int cpu_id_type, cpu_id_family, cpu_id_model, cpu_id_stepping;
	int screen_width, screen_height, screen_depth, screen_cols, screen_rows;
	int video_addr, video_lfb, video_memory_size;
	unsigned long root_sector_start;
	unsigned long _heap_size_1, _heap_size_2, _heap_size_;
	char filename[256];
	char *sh;
	int argc;
	char argv[256][256];
	char tmp_argv[256];
	
	multiboot_address = get_ebx();
	
	//memset(0x8800, 0, 1024);
	
	printk("Checking device sets.\n");
		
	loadsysteminfo();
	
	for(int i=0;i<256;i++)
	{
		memset(tmp_argv, 0, 256);
		memset(argv[i], 0, 256);
	}
	
	//putchar('\n');
		
	disable_interrupt();
	
	io_wait();
	
	loadgdt();	
	loadtss();
	loadisr();
	loadirq();
	loadidt();	
	#ifdef USE_DAP	
	loaddap();	
	#endif
	loadpci();
	enable_interrupt();
	
	init_timer(timer_frequency);
	init_keyboard();
	init_interrupts();
	
	init_heap();
	
	reloadsysteminfo();
	
	srand(timertick);
	
	loadpages();
	
	init_module_stack();
	
	loaddefines();
	
	info = (system_info_t*)system_info;
	cpu_id_type = info->cpu_id_type;
	cpu_id_family = info->cpu_id_family;
	cpu_id_model = info->cpu_id_model;
	cpu_id_stepping = info->cpu_id_stepping;
	physical_memory_l = info->physical_memory;
	_heap_size_1 = info->heap_1_size;
	_heap_size_2 = _heap_size;
	_heap_size_ = _heap_size_1 + _heap_size_2;
	
	// get_cpu_info(&cpu_id_type, &cpu_id_family, &cpu_id_model, &cpu_id_stepping);

	physical_memory = physical_memory_l / 1024 / 1024;
	used_memory = _heap_size_ / 1024 / 1024;
	used_memory_percent = (100/(physical_memory/used_memory));

	cpu_id_name = (char*)malloc(32);
	cpu_id_str = (char*)malloc(64);
	cpu_brand_str = (char*)malloc(256);
	sh = (char*)malloc(65536);

	lastcmd = (char*)malloc(65536);
	history = (cmd_line_text_t*)malloc(65536);
	
	memset(history, 0, 65536);
	memset(sh, 0, 65536);
	memset(cpu_id_name, 0, 32);
	memset(cpu_id_str, 0, 64);
	memset(cpu_brand_str, 0, 256);
	memset(lastcmd, 0, 65536);
	memset(pwd, 0, 1024);

	// printk("Extended Memory Manager loaded.\n");
	
	strcpy(cpu_id_name, info->cpu_id_name);
	strcpy(cpu_id_str, info->cpu_id_str);
	strcpy(cpu_brand_str, info->cpu_brand_str);
	
	//get_cpu_vendor_id_string(cpu_id_name);
	//get_cpu_vendor_string(cpu_id_str);
	//get_cpu_brand_str(cpu_brand_str);
	
	video_mode = get_video_mode();
	
	has_vesa = get_video_vesa_info();
	has_vesa_mode = get_video_vesa_mode(video_mode);
	if (has_vesa)
	{
		video_memory_size = vesa_info->video_memory_size;
	}
	if (has_vesa_mode)
	{
		video_addr_vesa_vga:
		video_addr = get_video_vesa_buffer(video_mode);
		if (video_addr == 0)
		{
			video_addr = (unsigned long)video_memory;
		}
		screen_width = vesa_mode->width;
		screen_height = vesa_mode->height;
		if (video_mode < 0x10)
		{
			screen_width *= vesa_mode->char_width;
			screen_height *= vesa_mode->char_height;
			screen_cols = vesa_mode->width;
			screen_rows = vesa_mode->height;
		}
		else
		{
			screen_cols = TEXT_COLS;
			screen_rows = TEXT_ROWS;			
		}
	}
	else
	{
		screen_width = TEXT_COLS;
		screen_height = TEXT_ROWS;
		screen_cols = vesa_mode->width;
		screen_rows = vesa_mode->height;
		if (pci_video_memory_found)
		{
			if (pci_video_memory_address)
			{
				video_addr = pci_video_memory_address;
			}
			else
			{
				video_addr = (unsigned long)video_memory;
			}
		}
		else
		{
			video_addr = (unsigned long)video_memory;
		}
	}
	
	loadvars();
	init_variables();
	
	if ((enum_loaded) && (total_enum > 0) && (has_enum(SYSTEM_STDIO_SERIAL))) 
	{
		init_keyboard();
	}
	
	if (has_enum(SYSTEM_VERBOSE))
	{
		if (pci_count)
		{
			printk("\n");	
			for(int i=0;i<pci_count;i++) 
			{
				printk("PCI Device: 0x%X:0x%X, bus: %d, device: %d, function: %d\n    Name: %s\n", 
				pci_device[i].pci.vendor, pci_device[i].pci.device, pci_device[i].bus, pci_device[i].slot, pci_device[i].function, 
				get_pci_class_name(pci_device[i].pci.class, pci_device[i].pci.subclass));
				if ((pci_device[i].pci.interrupt_line != 0) && (pci_device[i].pci.interrupt_pin != 0))
				{
					char irq_pin = ('A' + (pci_device[i].pci.interrupt_pin - 1));
					printk("    IRQ: %d, Pin: %c\n", pci_device[i].pci.interrupt_line, irq_pin);
				}
				for(int j=0;j<6;j++)
				{
					 if (pci_device[i].pci.bar[j] != 0)
					 {
						printk("    BAR%d: 0x%X\n", j, pci_device[i].pci.bar[j]);
						if ((pci_device[i].pci.class == 1) && (pci_device[i].pci.subclass == 6) && (pci_device[i].pci.vendor != 0xFFFF))
						{
							if (j == 5)
							{
								//pci_device[i].pci.bar[j]
								//pci_write_byte
							}
						}
					 }
				}
				printk("\n");		
			}
		}
		
		if (pci_video_memory_found)
		{
			if (pci_video_memory_address)
			{
				printk("Video Memory Address: 0x%X\n", pci_video_memory_address);
			}
		}
	}

	if (has_enum(SYSTEM_TIMER_RATE))
	{
		disable_interrupt();
		reset_timer();
		timer_frequency = get_enum(SYSTEM_TIMER_RATE);
		init_timer(timer_frequency);
		enable_interrupt();
	}

	if (init_ahci())
	{
		storage_drive_controller = STORAGE_CONTROLLER_AHCI;
	}
	else
	{
		storage_drive_controller = STORAGE_CONTROLLER_IDE;
	}

	if (loadmbr())
	{
		partition = (partition_entry_t*)mbr->partition;
		if (has_partition_active())
		{
			main_partition = (partition_entry_t*)&partition[active_partition];
		}
		else
		{
			panic((unsigned long)partition);
		}
	}
	else
	{
		// printk("Disk Error\n");
		panic((unsigned long)mbr_sector);
	}

	if (has_partition_active())
	{
		if (isfattype())
		{
			if (loadfat())
			{
				root_sector_start = getrootdirsector();
				setcurrentdirsector(root_sector_start);
				if ((enum_loaded) && (total_enum > 0) && (has_enum(SYSTEM_STDIO_SERIAL))) printk("\n");
				printk("filesystem device loaded.\n");
			} else
			{
				panic((unsigned long)boot_sector);
			}
		}
		else
		{
			panic((unsigned long)mbr_sector);
		}
	}
	
	
	printk("\n");
	printk("Fast System kernel loaded.\n");
	printk("Build number: 1000.10, rev: 2025.02\n");
	printk("\n");
	
	printk("Loading DMA Configuration.\n");
	loaddma();
	
	printk("Loading address sets.\n");
	loadsets();
	
    printk("Loading Memory Manager.\n");	
    loadmmap();
	
    //printk("\n");	
	
	
	if (enum_loaded)
	{
		if (total_enum > 0)
		{
			if (!has_enum(SYSTEM_KERNEL_MODE))
			{
				usermode = 1;
				kernelmode = 0;
			}
		}
	}
	
	if (usermode)
	{
		switchtousermode();
	}
	
	

	char *info_str = (char*)malloc(512);
	strcpy(info_str, "\nFast System Kernel Loader\n     Created by Mario Freire\n\nCopyright (C) 2025 DSP Interactive.\n");
	printk(info_str);
	free(info_str);

	if ((enum_loaded) && (total_enum > 0) && (has_enum(SYSTEM_STDIO_SERIAL)))
	{
		printk("\n");
		printk("stdout: serial (0x3F8).\n");
		printk("\n");
	}
	else
	{
		printk("Wait for ");
		for(int j=0;j<5;j++)
		{
			if (j > 0)
			{
				for(int k=0;k<=strlen(" seconds.");k++)
				{
					putch('\b');
				}
			}
			printk("%d seconds.", 5-j);
			msleep(1000);
		}
		for(int k=0;k<=strlen(" seconds.");k++)
		{
			putch('\b');
		}
		printk("0 seconds.\n");
	}
	
    //__asm__ volatile ("mov $0x4f, %ah; int $0x10");
    //__asm__ volatile ("mov $0x0f, %ah; int $0x10");

    //__asm__ __volatile__("int $2");
    //__asm__ __volatile__("int $3");
	//printk("Extended Memory Manager loaded.\n");

	/*
	char lc1 = getch();
	printk("last char: %c\n", lc1);
	*/

	// printk("%d\n", has_cpu_support_longmode());
	
	if ((enum_loaded) && (total_enum > 0) && (has_enum(SYSTEM_STDIO_SERIAL)))
	{
		for(int i=0;i<key_handlers_count;i++)
		{
			key_handlers[key_handlers_count] = 0;
		}
		interrupt_handlers[IRQ1] = 0;
		key_handlers_count = 0;
	}
	else
	{
		init_keys();	
	}
	reloadvars();
	
	char uuid1[37];
	uuidv4(uuid1);
	if (uuidv4_validate(uuid1))
	{
		if (has_enum(SYSTEM_VERBOSE))
		{
			printk("\n");
			printk("System Token ID: %s\n", uuid1);
			printk("\n");
		}
	}
	else
	{
		panic((unsigned long)uuid1);
	}
	

	while(1)
	{
		if (current_sector_pwd == 0)
		{
			memset(sh, 0, 65536);
			root_sector_start = getrootdirsector();
			setcurrentdirsector(root_sector_start);
		}		
		printk("%s]", pwd);
		if (gets(sh) != 0)
		{
			for(int i=0;i<256;i++)
			{
				memset(tmp_argv, 0, 256);
				memset(argv[i], 0, 256);
			}
			if (strlen(sh) > 0) store_cmd_line_str(sh);
			shell_exec_cmd:
			argc = getargc(sh);
			if (argc > 0)
			{
				for(int argcount=0;argcount<argc;argcount++)
				{
					memset(tmp_argv, 0, 256);
					getargv(tmp_argv, sh, argcount);
					strcpy(argv[argcount], tmp_argv);
				}
				if (strcmp(argv[0], "cpu") == 0)
				{
					printk("%s\n", cpu_id_str);
					printk("%s\n", cpu_brand_str);
					printk("%s\n", cpu_id_name);
					printk("%d MHz\n", cpu_speed);
				}
				else
				if (strcmp(argv[0], "memory") == 0)
				{
					printk("%d MB\n", physical_memory);
				}
				else
				if (strcmp(argv[0], "restart") == 0)
				{
					__asm__ volatile ( "int $0x80" :: "a" (88), "b" (0xffff), "c" (0xfff0), "d" (0x10000) );
				}
				else
				if (strcmp(argv[0], "videomemory") == 0)
				{
					if (has_vesa)
					{
						video_memory_size = vesa_info->video_memory_size;
						printk("%d MB\n", video_memory_size);
					}
				}
				else
				if (strcmp(argv[0], "videoaddress") == 0)
				{
					video_mode = 0x112;
					has_vesa = get_video_vesa_info();
					has_vesa_mode = get_video_vesa_mode(video_mode);
					if (pci_video_memory_found)
					{
						if (pci_video_memory_address)
						{
							video_addr = pci_video_memory_address;
						}
					}
					else
					{
						if (has_vesa && has_vesa_mode)
						{
							video_mode = 0x112;
							video_addr = get_video_vesa_buffer(video_mode);
							if (video_addr == 0)
							{
								video_mode = 0x102;
								has_vesa = get_video_vesa_info();
								has_vesa_mode = get_video_vesa_mode(video_mode);
								video_addr = get_video_vesa_buffer(video_mode);
								if (video_addr == 0)
								{
									video_addr = pci_video_memory_address;
									if (video_addr == 0)
									{
										video_addr = (unsigned long)video_memory;
										if (video_addr == 0) video_addr = 0xA0000;
									}
								}
							}
						}
						else
						{
							if (video_mode < 0x100)
							{
								if (video_mode < 0x10)
								{
									if (video_mode > 7)
									{
										video_addr = 0xB0000;
									}
									else
									{
										video_addr = 0xB8000;
									}
								}
								else
								{
									video_addr = 0xA0000;
								}
							}
							else
							{
								video_addr = (unsigned long)video_memory;
							}						
						}						
					}
					printk("Video Memory Address: 0x%X\n", video_addr);
					video_mode = 0x03;
				}
				else
				if (strcmp(argv[0], "videotest256") == 0)
				{
					if (usermode == 1)
					{
						printk("Cannot execute this command.\n");
					}
					else
					{	
						set_video_mode(0x13);
						video_mode = get_video_mode();
						has_vesa = get_video_vesa_info();
						has_vesa_mode = get_video_vesa_mode(video_mode);
						if (has_vesa_mode)
						{
							screen_width = vesa_mode->width;
							screen_height = vesa_mode->height;
							if (video_mode < 0x10)
							{
								screen_width *= vesa_mode->char_width;
								screen_height *= vesa_mode->char_height;
							}
							video_memory_size = vesa_info->video_memory_size;
							video_addr = (unsigned long)video_memory;
							screen_width = vesa_mode->width;
							screen_height = vesa_mode->height;
							screen_depth = vesa_mode->depth;
							video_lfb = has_vesa_framebuffer;	
						}
						else
						{
							screen_width = TEXT_COLS;
							screen_height = TEXT_ROWS;
						}
						for(int x=10;x<100;x++)
						{
							for(int y=20;y<100;y++)
							{
								set_video_pixel(x, y, 15);
							}
						}
						getch();
						set_video_mode(0x3);
						if (has_vesa && has_vesa_mode)
						{
							char addrs[32];
							itoa(video_addr, addrs, 16);
							printk("Screen Width: %d\n", screen_width);
							printk("Screen Height: %d\n", screen_height);
							printk("Screen Depth: %d\n", screen_depth);
							printk("Screen Buffer: 0x%s\n", strupr((char*)addrs));
							printk("Video Memory Size: %d MB\n", video_memory_size);
							printk("Video Frame Buffer: %d\n", video_lfb);
							printk("Has VESA Support: %d\n", has_vesa);
							printk("Has VESA Mode Support: %d\n", has_vesa_mode);
						}
						video_mode = get_video_mode();
						has_vesa = get_video_vesa_info();
						has_vesa_mode = get_video_vesa_mode(video_mode);
						if (has_vesa_mode)
						{
							screen_width = vesa_mode->width;
							screen_height = vesa_mode->height;
							if (video_mode < 0x10)
							{
								screen_width *= vesa_mode->char_width;
								screen_height *= vesa_mode->char_height;
								screen_cols = vesa_mode->width;
								screen_rows = vesa_mode->height;
							}
							else
							{
								screen_cols = TEXT_COLS;
								screen_rows = TEXT_ROWS;								
							}
						}
						else
						{
							screen_width = TEXT_COLS;
							screen_height = TEXT_ROWS;
							screen_cols = screen_width;
							screen_rows = screen_height;
						}
					}
				}
				else
				if (strcmp(argv[0], "videotest32") == 0)
				{
					if (usermode == 1)
					{
						printk("Cannot execute this command.\n");
					}
					else
					{	
						video_mode = get_video_mode();
						has_vesa = get_video_vesa_info();
						has_vesa_mode = get_video_vesa_mode(video_mode);
						if (has_vesa_mode)
						{
							screen_width = vesa_mode->width;
							screen_height = vesa_mode->height;
							if (video_mode < 0x10)
							{
								screen_width *= vesa_mode->char_width;
								screen_height *= vesa_mode->char_height;
							}
						}
						else
						{
							screen_width = TEXT_COLS;
							screen_height = TEXT_ROWS;
						}
						if (has_vesa && has_vesa_mode && set_video_vesa_mode(0x4118))
						{
							video_memory_size = vesa_info->video_memory_size;
							video_addr = (unsigned long)video_memory;
							screen_width = vesa_mode->width;
							screen_height = vesa_mode->height;
							screen_depth = vesa_mode->depth;
							video_lfb = has_vesa_framebuffer;
							for(int x=10;x<100;x++)
							{
								for(int y=20;y<100;y++)
								{
									set_vesa_pixel(x, y, rgb(255,255,255));
								}
							}
							char addrs[32];
							itoa(video_addr, addrs, 16);
							getch();
							set_video_vesa_mode(0x3);		
							printk("Screen Width: %d\n", screen_width);
							printk("Screen Height: %d\n", screen_height);
							printk("Screen Depth: %d\n", screen_depth);
							printk("Screen Buffer: 0x%s\n", strupr((char*)addrs));
							printk("Video Memory Size: %d MB\n", video_memory_size);
							printk("Video Frame Buffer: %d\n", video_lfb);
							printk("Has VESA Support: %d\n", has_vesa);
							printk("Has VESA Mode Support: %d\n", has_vesa_mode);
						}
						else
						{
							printk("Error\n");
						}
						video_mode = get_video_mode();
						has_vesa = get_video_vesa_info();
						has_vesa_mode = get_video_vesa_mode(video_mode);
						if (has_vesa_mode)
						{
							screen_width = vesa_mode->width;
							screen_height = vesa_mode->height;
							if (video_mode < 0x10)
							{
								screen_width *= vesa_mode->char_width;
								screen_height *= vesa_mode->char_height;
								screen_cols = vesa_mode->width;
								screen_rows = vesa_mode->height;
							}
							else
							{
								screen_cols = TEXT_COLS;
								screen_rows = TEXT_ROWS;								
							}
						}
						else
						{
							screen_width = TEXT_COLS;
							screen_height = TEXT_ROWS;
							screen_cols = screen_width;
							screen_rows = screen_height;
						}
					}
				}
				else
				if (strcmp(argv[0], "videoinfo") == 0)
				{
					if (usermode == 1)
					{
						printk("Cannot execute this command.\n");
					}
					else
					{
						unsigned short tmp_video_mode = get_video_mode();
						if (argc > 1) video_mode = StrToHex(argv[1]);
						else video_mode = get_video_mode();
						has_vesa = get_video_vesa_info();
						has_vesa_mode = get_video_vesa_mode(video_mode);
						if (has_vesa_mode)
						{
							screen_width = vesa_mode->width;
							screen_height = vesa_mode->height;
							if (video_mode < 0x10)
							{
								screen_width *= vesa_mode->char_width;
								screen_height *= vesa_mode->char_height;
								screen_cols = vesa_mode->width;
								screen_rows = vesa_mode->height;
							}
							else
							{
								screen_cols = TEXT_COLS;
								screen_rows = TEXT_ROWS;								
							}
						}
						else
						{
							screen_width = TEXT_COLS;
							screen_height = TEXT_ROWS;
							screen_cols = screen_width;
							screen_rows = screen_height;
						}
						if (has_vesa && has_vesa_mode)
						{
							video_memory_size = vesa_info->video_memory_size;
							screen_depth = vesa_mode->depth;
							if (argc > 1) 
							{
								video_lfb = has_video_vesa_framebuffer(video_mode);
								video_addr = get_video_vesa_buffer(video_mode);
							}
							else
							{
								video_addr = (unsigned long)video_memory;
								video_lfb = has_vesa_framebuffer;		
								if (video_mode < 0x100)
								{
									if (video_mode < 0x10)
									{
										if (video_mode > 7)
										{
											video_addr = 0xB0000;
										}
										else
										{
											video_addr = 0xB8000;
										}
									}
									else
									{
										video_addr = 0xA0000;
									}
								}
								else
								{
									video_addr = (unsigned long)video_memory;
								}							
							}
							char addrs[32];
							itoa(video_addr, addrs, 16);
							printk("Screen Width: %d\n", screen_width);
							printk("Screen Height: %d\n", screen_height);
							if (video_mode > 7)
							{
								printk("Screen Depth: %d\n", screen_depth);
							}
							else
							{
								printk("Screen Columns: %d\n", screen_cols);
								printk("Screen Rows: %d\n", screen_rows);								
							}
							printk("Screen Buffer: 0x%s\n", strupr((char*)addrs));
							printk("Video Memory Size: %d MB\n", video_memory_size);
							if (video_mode > 7)
							{
								printk("Video Frame Buffer: %d\n", video_lfb);
							}
							printk("Has VESA Support: %d\n", has_vesa);
							printk("Has VESA Mode Support: %d\n", has_vesa_mode);
						}
						else
						{
							printk("Screen Width: %d\n", screen_width);
							printk("Screen Height: %d\n", screen_height);
							if (video_mode < 0x100)
							{
								if (video_mode < 0x10)
								{
									if (video_mode > 7)
									{
										video_addr = 0xB0000;
									}
									else
									{
										video_addr = 0xB8000;
									}
								}
								else
								{
									video_addr = 0xA0000;
								}
							}
							else
							{
								video_addr = (unsigned long)video_memory;
							}
							video_lfb = has_vesa_framebuffer;
							printk("Screen Buffer: 0x%X\n", video_addr);
							printk("Has VESA Support: %d\n", has_vesa);
							printk("Has VESA Mode Support: %d\n", has_vesa_mode);
						}
						video_mode = tmp_video_mode;
					}
				}
				else
				if (strcmp(argv[0], "videotest") == 0)
				{
					unsigned short v_mode;
					if (argc > 1) v_mode = StrToHex(argv[1]);
					else v_mode = 0x4112;
					unsigned short v_result;
					if (get_video_vesa_mode(v_mode))
					{
					v_result = set_video_vesa_mode(v_mode);
					for(int x=10;x<100;x++)
					{
						for(int y=20;y<100;y++)
						{
							set_vesa_pixel(x, y, rgb(255,255,255));
						}
					}					
					} else v_result = 0;
					getch();
					set_video_vesa_mode(0x03);
					printk("Video Mode: 0x%X\n", v_mode);
					if (v_result) printk("Status: OK\n");
					else printk("Status: Error\n");
				}
				else
				if (strcmp(argv[0], "videotest_blank") == 0)
				{
					registers16_t r1, r2, r3, r4;
					memset(&r1, 0, sizeof(registers16_t));
					memset(&r2, 0, sizeof(registers16_t));
					memset(&r3, 0, sizeof(registers16_t));
					memset(&r4, 0, sizeof(registers16_t));
					r1.ax = 0x4f02;
					if (argc > 1) r1.bx = StrToHex(argv[1]);
					else r1.bx = 0x4112;
					unsigned short v_mode = r1.bx;
					int86(0x10, &r1, &r2);
					unsigned short v_result = r2.ax;		
					getch();
					r3.ax = 0x4f02;
					r3.bx = 0x03;
					int86(0x10, &r3, &r4);
					printk("Video Mode: 0x%X\n", v_mode);
					printk("Return Value: 0x%X\n", v_result);
					if (v_result == 0x4f) printk("Status: OK\n");
					else printk("Status: Error\n");
				}
				else
				if (strcmp(argv[0], "checkvideomode") == 0)
				{
					registers16_t r1, r2, r3, r4;
					memset(&r1, 0, sizeof(registers16_t));
					memset(&r2, 0, sizeof(registers16_t));
					memset(&r3, 0, sizeof(registers16_t));
					memset(&r4, 0, sizeof(registers16_t));
					r1.ax = 0x4f02;
					if (argc > 1) r1.bx = StrToHex(argv[1]);
					else r1.bx = 0x4112;
					unsigned short v_mode = r1.bx;
					int86(0x10, &r1, &r2);
					unsigned short v_result = r2.ax;
					//getch();
					msleep(10);
					r3.ax = 0x4f02;
					r3.bx = 0x03;
					int86(0x10, &r3, &r4);
					printk("Video Mode: 0x%X\n", v_mode);
					printk("Return Value: 0x%X\n", v_result);
					if (v_result == 0x4f) printk("Status: OK\n");
					else printk("Status: Error\n");
				}
				else
				if (strcmp(argv[0], "int41") == 0)
				{
					__asm__ __volatile__("int $0x41");
				}
				else
				if (strcmp(argv[0], "int6a") == 0)
				{
					__asm__ __volatile__("int $0x6a");
				}
				else
				if (strcmp(argv[0], "int7f") == 0)
				{
					__asm__ __volatile__("int $0x7f");
				}
				else
				if (strcmp(argv[0], "int80") == 0)
				{
					__asm__ __volatile__("int $0x80");
				}
				else
				if (strcmp(argv[0], "printmessage") == 0)
				{
					if (argc > 1)
					{
						int len = strlen(argv[1]);
						asm( "int $0x80" :: "a" (4), "b" (1), "c" (argv[1]), "d" (len) );
						printk("\n");
					}
				}
				else
				if (strcmp(argv[0], "sysvideo") == 0)
				{
					asm( "int $0x80" :: "a" (225), "b" (0x4112) );
					for(int y=0;y<480;y++)
						for(int x=0;x<640;x++)
							set_vesa_pixel(x,y,rgb(0,0,255));
					for(int y=10;y<110;y++)
						for(int x=10;x<110;x++)
							set_vesa_pixel(x,y,rgb(255,255,255));
					getch();
					asm( "int $0x80" :: "a" (225), "b" (0x3) );
				}
				else
				if ((strcmp(argv[0], "readm8") == 0) || (strcmp(argv[0], "memory_read_byte") == 0) || (strcmp(argv[0], "memory_read") == 0))
				{
					if (argc > 1)
					{
						unsigned long l_addr = StrToHex(argv[1]);
						unsigned char *x_ptr = (unsigned char*)l_addr;
						unsigned char x_val = (unsigned char)x_ptr[0];
						printk("%s: 0x%X\n", argv[0],  x_val);
					}
				}
				else
				if ((strcmp(argv[0], "readm16") == 0) || (strcmp(argv[0], "memory_read_word") == 0))
				{
					if (argc > 1)
					{
						unsigned long l_addr = StrToHex(argv[1]);
						unsigned char *x_ptr = (unsigned char*)l_addr;
						unsigned short x_val = (unsigned short)UINT16(x_ptr[0], x_ptr[1]);
						printk("%s: 0x%X\n", argv[0],  x_val);
					}
				}
				else
				if ((strcmp(argv[0], "readm32") == 0) || (strcmp(argv[0], "memory_read_long") == 0) || (strcmp(argv[0], "memory_read_dword") == 0))
				{
					if (argc > 1)
					{
						unsigned long l_addr = StrToHex(argv[1]);
						unsigned char *x_ptr = (unsigned char*)l_addr;
						unsigned long x_val = (unsigned long)UINT32(x_ptr[0], x_ptr[1], x_ptr[2], x_ptr[3]);
						printk("%s: 0x%X\n", argv[0], x_val);
					}
				}
				else
				if ((strcmp(argv[0], "writem8") == 0) || (strcmp(argv[0], "memory_write_byte") == 0) || (strcmp(argv[0], "memory_write") == 0))
				{
					if (argc > 2)
					{
						unsigned long l_addr = StrToHex(argv[1]);
						unsigned char *x_ptr = (unsigned char*)l_addr;
						unsigned long v_set = StrToHex(argv[2]);
						unsigned char v_ptr[4] = {UCHAR8A(v_set), UCHAR8B(v_set), UCHAR8C(v_set), UCHAR8D(v_set)};
						x_ptr[0] = v_ptr[0];
						unsigned char x_val = (unsigned char)x_ptr[0];
						printk("%s: 0x%X\n", argv[0],  x_val);
					}
				}
				else
				if ((strcmp(argv[0], "writem16") == 0) || (strcmp(argv[0], "memory_write_word") == 0))
				{
					if (argc > 2)
					{
						unsigned long l_addr = StrToHex(argv[1]);
						unsigned char *x_ptr = (unsigned char*)l_addr;
						unsigned long v_set = StrToHex(argv[2]);
						unsigned char v_ptr[4] = {UCHAR8A(v_set), UCHAR8B(v_set), UCHAR8C(v_set), UCHAR8D(v_set)};
						x_ptr[0] = v_ptr[0];
						x_ptr[1] = v_ptr[1];
						unsigned short x_val = (unsigned short)UINT16(x_ptr[0], x_ptr[1]);
						printk("%s: 0x%X\n", argv[0],  x_val);
					}
				}
				else
				if ((strcmp(argv[0], "writem32") == 0) || (strcmp(argv[0], "memory_write_long") == 0) || (strcmp(argv[0], "memory_write_dword") == 0))
				{
					if (argc > 2)
					{
						unsigned long l_addr = StrToHex(argv[1]);
						unsigned char *x_ptr = (unsigned char*)l_addr;
						unsigned long v_set = StrToHex(argv[2]);
						unsigned char v_ptr[4] = {UCHAR8A(v_set), UCHAR8B(v_set), UCHAR8C(v_set), UCHAR8D(v_set)};
						x_ptr[0] = v_ptr[0];
						x_ptr[1] = v_ptr[1];
						x_ptr[2] = v_ptr[2];
						x_ptr[3] = v_ptr[3];
						unsigned long x_val = (unsigned long)UINT32(x_ptr[0], x_ptr[1], x_ptr[2], x_ptr[3]);
						printk("%s: 0x%X\n", argv[0], x_val);
					}
				}
				else
				if (strcmp(argv[0], "partinfo") == 0)
				{
					if (isfattype())
					{
						char partition_type_name[16];
						unsigned char partition_type;
						unsigned char has_lba;
						unsigned long partition_size;
						unsigned long partition_start;
						unsigned long partition_lba_start;
						unsigned long cluster_start;
						unsigned long root_lba_address;
						unsigned long root_address;
						unsigned long root_cluster;
						unsigned long volume_code;
						unsigned char number_fats;
						unsigned long sectors_per_fat;
						unsigned char drive_number;
						unsigned char boot_signature;
						unsigned char fat_reserved;
						unsigned short root_entries_count;
						unsigned short reserved_sectors_count;
						unsigned char sector_per_cluster;
						unsigned short bootstrap_size;
						unsigned short fat_fsinfo;
						unsigned short fat_backup;
						unsigned short fat_version;
						unsigned short fat_flags;
						unsigned char fat_media;
						unsigned short fat_num_heads;
						unsigned long fat_hidden_sectors;
						unsigned long fat_total_sectors;
						unsigned long fsinfo_lead_sign;
						unsigned char fsinfo_reserv1[480];
						unsigned long fsinfo_struc_sign;
						unsigned long fsinfo_free_count;
						unsigned long fsinfo_next_free;
						unsigned char fsinfo_reserv2[12];
						unsigned long fsinfo_trail_sign;
						unsigned short bytes_per_sector;
						unsigned long free_sectors;
						unsigned long free_space;
						unsigned long total_space;
						unsigned long used_space;
						unsigned char is_fat_32;
						char oem_name[16];
						char volume_label[16];
						char fat_type_label[16];
						int fat_struct_all = 0;
						int fat_more = 0;
						has_lba = 0;
						is_fat_32 = 0;
						root_lba_address = getrootlbaaddress();
						if (argc > 1)
						{
							for(int k=1;k<argc;k++)
							{
								if ((strcmp(argv[k], "-m") == 0) || (strcmp(argv[k], "--more") == 0))
								{
									fat_more = 1;
								}
								if ((strcmp(argv[k], "-a") == 0) || (strcmp(argv[k], "--all") == 0))
								{
									fat_struct_all = 1;
								}
							}
						}
						if (root_lba_address != 0)
						{
							if (loadrootentries())
							{
								printk("Partition Entry: %u\n", active_partition);
								partition_type = getpartitiontype();							
								switch(partition_type)
								{
									case PARTITION_FAT16:
									case PARTITION_FAT16_LBA:
									{
										strcpy(partition_type_name, "FAT16");
										bytes_per_sector = fat->bpb.bpb1.bytes_per_sector;
										volume_code = fat->bpb.bpb2.fat16.volume_id;
										root_cluster = 0;
										drive_number = fat->bpb.bpb2.fat16.drive_number;
										sectors_per_fat = fat->bpb.bpb1.fat_size_16;
										sector_per_cluster = fat->bpb.bpb1.sector_per_cluster;
										root_entries_count = fat->bpb.bpb1.root_entries_count;
										reserved_sectors_count = fat->bpb.bpb1.reserved_sectors_count;
										boot_signature = fat->bpb.bpb2.fat16.boot_signature;
										fat_reserved = fat->bpb.bpb2.fat16.reserved;
										bootstrap_size = sizeof(fat->bpb.bpb2.fat16.bootstrap);
										fat_total_sectors = fat->bpb.bpb1.total_sectors_32;//16;
										number_fats = fat->bpb.bpb1.number_fats;
										fat_flags = 0;
										fat_fsinfo = 0;
										fat_backup = 0;
										fat_version = 0;
										fsinfo_lead_sign = 0;
										for(int k=0;k<480;k++) fsinfo_reserv1[k] = 0;
										fsinfo_struc_sign = 0;
										fsinfo_free_count = 0;
										fsinfo_next_free = 0;
										for(int k=0;k<12;k++) fsinfo_reserv2[k] = 0;
										fsinfo_trail_sign = 0;
										total_space = fat_total_sectors * bytes_per_sector;
										used_space = total_size_files;
										free_space = total_space - used_space;
										free_sectors = free_space / bytes_per_sector;
										is_fat_32 = 0;
									};
									break;
									case PARTITION_FAT32:
									case PARTITION_FAT32_LBA:
									{
										strcpy(partition_type_name, "FAT32");
										bytes_per_sector = fat->bpb.bpb1.bytes_per_sector;
										volume_code = fat->bpb.bpb2.fat32.volume_id;
										root_cluster = fat->bpb.bpb2.fat32.root_cluster;
										drive_number = fat->bpb.bpb2.fat32.drive_number;
										sectors_per_fat = fat->bpb.bpb2.fat32.fat_size_32;
										boot_signature = fat->bpb.bpb2.fat32.boot_signature;
										bootstrap_size = sizeof(fat->bootstrap);
										fat_total_sectors = fat->bpb.bpb1.total_sectors_32;
										fat_flags = fat->bpb.bpb2.fat32.flags;
										fat_fsinfo = fat->bpb.bpb2.fat32.fs_info;
										fat_backup = fat->bpb.bpb2.fat32.backup_boot_sector;
										fat_version = fat->bpb.bpb2.fat32.version;
										fat_reserved = fat->bpb.bpb2.fat32.reserved_1;
										fsinfo_lead_sign = fsinfo->lead_signature;
										for(int k=0;k<480;k++) fsinfo_reserv1[k] = fsinfo->reserved_1[k];
										fsinfo_struc_sign = fsinfo->struc_signature;
										fsinfo_free_count = fsinfo->free_count;
										fsinfo_next_free = fsinfo->next_free;
										for(int k=0;k<12;k++) fsinfo_reserv2[k] = fsinfo->reserved_2[k];
										fsinfo_trail_sign = fsinfo->trail_signature;
										free_sectors = fat32_free_space_sectors(fsinfo_free_count, sectors_per_fat);
										free_space = fat32_free_space(fsinfo_free_count, bytes_per_sector, sectors_per_fat);
										total_space = fat32_total_space(fat_total_sectors, bytes_per_sector);
										used_space = fat32_used_space(fsinfo_free_count, fat_total_sectors, bytes_per_sector, sectors_per_fat);
										is_fat_32 = 1;
									};
									break;				
								};
								if ((partition_type == PARTITION_FAT16_LBA) || 
								    (partition_type == PARTITION_FAT32_LBA))
								{
									has_lba = 1;	
								}
								partition_size = getpartitionsize();
								partition_start = getpartitionstart();
								partition_lba_start = getpartitionlbastart();
								cluster_start = getclusterstart();
								
								root_address = getrootaddress();
								getoemname(oem_name);
								getvolumelabel(volume_label);
								getfattypelabel(fat_type_label);
								fat_media = fat->bpb.bpb1.media;
								fat_num_heads = fat->bpb.bpb1.number_heads;
								fat_hidden_sectors = fat->bpb.bpb1.hidden_sectors;
								printk("Partition Type Name: %s\n", partition_type_name);
								printk("Partition Type Code: 0x%X (%d)\n", partition_type, partition_type);
								printk("Partition Size: %u\n", partition_size);
								printk("Partition Start: %u\n", partition_start);
								printk("Partition LBA Start: 0x%X (%d)\n", partition_lba_start, partition_lba_start);
								if (fat_more) presskey();
								printk("Has LBA: %u\n", has_lba);
								printk("OEM Name: %s\n", oem_name);
								printk("Volume Label: %s\n", volume_label);
								printk("Volume ID: %s\n", volume_id);
								printk("Volume Code: 0x%X\n", volume_code);
								if (fat_more) presskey();
								printk("Root Address: 0x%X (%d)\n", root_address, root_address);
								printk("Root LBA Address: 0x%X (%d)\n", root_lba_address, root_lba_address);
								printk("FAT Type Label: %s\n", fat_type_label);
								printk("Drive Number: 0x%X\n", drive_number);
								printk("Root Entry Count: %u\n", fat->bpb.bpb1.root_entries_count);
								if (fat_more) presskey();
								printk("Bytes Per Sector: %u\n", bytes_per_sector);
								printk("Sector Per Cluster: %u\n", fat->bpb.bpb1.sector_per_cluster);
								printk("Sectors Per Track: %u\n", fat->bpb.bpb1.sectors_per_track);
								printk("Sectors Per FAT: %u\n", sectors_per_fat);
								printk("Table Count / Number of FAT: %u\n", fat->bpb.bpb1.number_fats);
								if (fat_more) presskey();
								printk("Reserved Sectors Count %u\n", fat->bpb.bpb1.reserved_sectors_count);
								if (is_fat_32) printk("Root Cluster: %u\n", root_cluster);
								printk("Cluster Start: %u\n", cluster_start);
								printk("Boot Signature: 0x%X (%d)\n", boot_signature, boot_signature);
								printk("BIOS Signature Code: 0x%X (%d)\n", fat->signature, fat->signature);
								if (fat_struct_all)
								{
									if (fat_more && is_fat_32) presskey();
									printk("Total Sectors: 0x%X (%d)\n", fat_total_sectors, fat_total_sectors);
									if (fat_more && (!is_fat_32)) presskey();
									printk("Hidden Sectors: 0x%X (%d)\n", fat_hidden_sectors, fat_hidden_sectors);
									printk("Number of Heads: 0x%X (%d)\n", fat_num_heads, fat_num_heads);
									printk("Media: 0x%X (%d)\n", fat_media, fat_media);
									printk("Bootstrap Size: 0x%X (%d)\n", bootstrap_size, bootstrap_size);
									if (fat_more && is_fat_32) presskey();
									printk("Reserved: 0x%X (%d)\n", fat_reserved, fat_reserved);
									if (fat_more && (!is_fat_32)) presskey();
									if (is_fat_32)
									{
										printk("FAT Version: 0x%X (%d)\n", fat_version, fat_version);
										printk("Flags: 0x%X (%d)\n", fat_flags, fat_flags);
										printk("FS Info: 0x%X (%d)\n", fat_fsinfo, fat_fsinfo);
										printk("Backup Boot Sector: 0x%X (%d)\n", fat_backup, fat_backup);
										if (fat_more) presskey();
										printk("Lead Signature: 0x%X (%d)\n", fsinfo_lead_sign, fsinfo_lead_sign);
										printk("Struc Signature: 0x%X (%d)\n", fsinfo_struc_sign, fsinfo_struc_sign);
										printk("Trail Signature: 0x%X (%d)\n", fsinfo_trail_sign, fsinfo_trail_sign);
										printk("Free Count: 0x%X (%d)\n", fsinfo_free_count, fsinfo_free_count);
										printk("Next Free: 0x%X (%d)\n", fsinfo_next_free, fsinfo_next_free);
										if (fat_more) presskey();
										printk("Free Sectors: 0x%X (%d)\n", free_sectors, free_sectors);	
										printk("Total Space: 0x%X (%d)\n", total_space, total_space);	
										printk("Free Space: 0x%X (%d)\n", free_space, free_space);	
										printk("Used Space: 0x%X (%d)\n", used_space, used_space);										
									}
								}
							}
							else
							{
								printk("Cannot read root entries in file system.\n");
							}
						}
						else
						{
							printk("Cannot read root address in file system.\n");
						}
					}
					else
					{
						printk("Cannot find FAT partition in disk.\n");
					}
				}
				else
				if (strcmp(argv[0], "dumpsector") == 0)
				{
					if (argc > 1)
					{
						unsigned char tmp_sector[SECTORSIZE];
						unsigned long cur_sector = StrToHex(argv[1]);
						readsector((unsigned long)cur_sector, tmp_sector);
						dump_hex(tmp_sector, SECTORSIZE);
					}
				}
				else
				if (strcmp(argv[0], "shutdown") == 0)
				{
					shutdown();
				}
				else
				if (strcmp(argv[0], "clear") == 0)
				{
					clrscr();
				}
				else
				if (strcmp(argv[0], "pwd") == 0)
				{
					printk("%s\n", pwd);
				}
				else
				if (strcmp(argv[0], "cd") == 0)
				{
					unsigned char has_dir;
					char dir_name[256];
					if (argc > 1)
					{
						strcpy(dir_name, argv[1]);
					}
					else
					{
						strcpy(dir_name, "/");
					}
					if (fileexists(dir_name))
					{
						if (!is_dir(dir_name))
						{
							notdirectory(argv[0], (char*)argv[1]);
						}
						else
						{
							has_dir = chdir(dir_name);
							if (!has_dir)
							{
								notfound(argv[0], (char*)argv[1]);
							}
						}
					}
					else
					{
						notfound(argv[0], (char*)argv[1]);
					}
				}
				else
				/*if (strcmp(argv[0], "listdir1") == 0)
				{
					unsigned char has_file;
					char file_name[256];
					if (argc > 1)
					{
						strcpy(file_name, argv[1]);
					}
					else
					{
						strcpy(file_name, "");
					}
					has_file = listdir1(file_name);
					if (!has_file)
					{
						notfound(argv[0], (char*)argv[1]);
					}
				}
				else
				if (strcmp(argv[0], "listdir2") == 0)
				{
					unsigned char has_file;
					char file_name[256];
					if (argc > 1)
					{
						strcpy(file_name, argv[1]);
					}
					else
					{
						strcpy(file_name, "");
					}
					has_file = listdir2(file_name);
					if (!has_file)
					{
						notfound(argv[0], (char*)argv[1]);
					}
				}
				else*/
				if (strcmp(argv[0], "ls") == 0)
				{
					unsigned char has_file;
					char file_name[256];
					if (argc > 1)
					{
						strcpy(file_name, argv[1]);
					}
					else
					{
						strcpy(file_name, "");
					}
					has_file = listdir(file_name);
					if (!has_file)
					{
						notfound(argv[0], (char*)argv[1]);
					}
				}
				else
				if (strncmp(argv[0], "echo", 4) == 0)
				{
					if (argc > 1)
					{
						for(int k=1;k<argc;k++)
						{
							if (k > 1) putch(' ');
							printk("%s", argv[k]);
						}
						putch('\n');
					}
				}
				else
				if (strncmp(argv[0], "filesize", 3) == 0)
				{
					if (argc > 1)
					{
						if (fileexists(argv[1]))
						{
							if (is_file(argv[1]))
							{
								int fsz = getfilesize(argv[1]);
								printk("%d\n", fsz);
							}
							else
							{
								isdirectory(argv[0], (char*)argv[1]);
							}
						}
						else
						{
							notfound(argv[0], (char*)argv[1]);
						}
					}
				}
				else
				if (strncmp(argv[0], "cat", 3) == 0)
				{
					if (argc > 1)
					{
						if (fileexists(argv[1]))
						{
							if (is_dir(argv[1]))
							{
								isdirectory(argv[0], (char*)argv[1]);
							}
							else
							{
								int fsz = getfilesize(argv[1]);
								unsigned char *fdata = (unsigned char*)malloc(fsz);
								getfiledata(argv[1], fdata);
								puts(fdata);
								free(fdata);
							}
						}
						else
						{
							notfound(argv[0], (char*)argv[1]);
						}
					}
					else
					{
						char tempbuf[65536];
						gets(tempbuf);
						printk("%s\n", tempbuf);
					}
				}
				else
				if (strcmp(argv[0], "pause") == 0)
				{
					presskey();
				}
				else
				if (strncmp(argv[0], "printf", 6) == 0)
				{
					if (argc > 1)
					{						
						long int x1 = auto_dstr(argv[2]);
						long int x2 = auto_dstr(argv[3]);
						long int x3 = auto_dstr(argv[4]);
						long int x4 = auto_dstr(argv[5]);
						cprintf(10, argv[1], x1, x2, x3, x4);
						putch('\n');
					}
				}
				else
				if (strncmp(argv[0], "printmemory", 6) == 0)
				{
					if (argc > 1)
					{
						unsigned long memory_char_addr_x = StrToHex(argv[1]);
						unsigned char *memory_char_addr = (unsigned char*)memory_char_addr_x;
						char memory_char = (char)*(char*)memory_char_addr;
						char m_hex[32];
						itoa(memory_char, m_hex, 16);
						cprintf(10, "%s", strupr((char*)m_hex));
						putch('\n');
					}
				}
				else
				if (strcmp(argv[0], "basememory16") == 0)
				{
					if (usermode == 1)
					{
						printk("Cannot execute this command.\n");
					}
					else
					{
						registers16_t r1,r2;
						r1.ax = 0;
						int86(0x12, &r1, &r2);
						printk("%d K\n", r2.ax);
					}
				}
				else 
				if (strcmp(argv[0], "inittimer") == 0)
				{				
					if (usermode == 1)
					{
						printk("Cannot execute this command.\n");
					}
					else
					{
						init_timer(timer_frequency);
					}
				}
				else 
				if (strcmp(argv[0], "sti") == 0)
				{
					enable_interrupt();
				}
				else 
				if (strcmp(argv[0], "cli") == 0)
				{
					disable_interrupt();
				}
				else
				if (strncmp(argv[0], "inb",3) == 0)
				{
					if (argc > 1)
					{
						unsigned short port = StrToHex(argv[1]);
						printk("inb: 0x%X: 0x%X\n", port, inb(port));
					}
				}
				else 
				if (strncmp(argv[0], "outb",4) == 0)
				{
					if (argc > 1)
					{
						unsigned short port = StrToHex(argv[1]);
						unsigned char value = StrToHex(argv[2]);
						printk("outb: 0x%X: 0x%X\n", port, value);
						outb(port, value);
					}
				}
				else 
				if (strncmp(argv[0], "inw",3) == 0)
				{
					if (argc > 1)
					{
						unsigned short port = StrToHex(argv[1]);
						printk("inw: 0x%X: 0x%X\n", port, inw(port));
					}
				}
				else 
				if (strncmp(argv[0], "outw",4) == 0)
				{
					if (argc > 1)
					{
						unsigned short port = StrToHex(argv[1]);
						unsigned short value = StrToHex(argv[2]);
						printk("outw: 0x%X: 0x%X\n", port, value);
						outw(port, value);
					}
				}
				else 
				if (strncmp(argv[0], "inl",3) == 0)
				{
					if (argc > 1)
					{
						unsigned short port = StrToHex(argv[1]);
						printk("inl: 0x%X: 0x%X\n", port, inl(port));
					}
				}
				else 
				if (strncmp(argv[0], "outl",4) == 0)
				{
					if (argc > 1)
					{
						unsigned short port = StrToHex(argv[1]);
						unsigned long value = StrToHex(argv[2]);
						printk("outl: 0x%X: 0x%X\n", port, value);
						outl(port, value);
					}
				}
				else 
				if ((strcmp(argv[0], "pci_read") == 0) || (strcmp(argv[0], "pci_read_word") == 0))
				{
					if (argc == 5)
					{
						unsigned char bus = StrToHex(argv[1]);
						unsigned char device = StrToHex(argv[2]);
						unsigned char func = StrToHex(argv[3]);
						unsigned char offset = StrToHex(argv[4]);
						unsigned short value = pci_read_word(bus, device, func, offset);
						printk("%s: 0x%X: 0x%X\n", argv[0], offset, value);
					}
				}
				else 
				if ((strcmp(argv[0], "pci_write") == 0) || (strcmp(argv[0], "pci_write_word") == 0))
				{
					if (argc == 6)
					{
						unsigned char bus = StrToHex(argv[1]);
						unsigned char device = StrToHex(argv[2]);
						unsigned char func = StrToHex(argv[3]);
						unsigned char offset = StrToHex(argv[4]);
						unsigned short value = StrToHex(argv[5]);
						printk("%s: 0x%X: 0x%X\n", argv[0], offset, value);
						pci_write_word(bus, device, func, offset, value);
					}
				}
				else 
				if (strcmp(argv[0], "pci_read_byte") == 0)
				{
					if (argc == 5)
					{
						unsigned char bus = StrToHex(argv[1]);
						unsigned char device = StrToHex(argv[2]);
						unsigned char func = StrToHex(argv[3]);
						unsigned char offset = StrToHex(argv[4]);
						unsigned char value = pci_read_byte(bus, device, func, offset);
						printk("%s: 0x%X: 0x%X\n", argv[0], offset, value);
					}
				}
				else 
				if (strcmp(argv[0], "pci_write_byte") == 0)
				{
					if (argc == 6)
					{
						unsigned char bus = StrToHex(argv[1]);
						unsigned char device = StrToHex(argv[2]);
						unsigned char func = StrToHex(argv[3]);
						unsigned char offset = StrToHex(argv[4]);
						unsigned char value = StrToHex(argv[5]);
						printk("%s: 0x%X: 0x%X\n", argv[0], offset, value);
						pci_write_byte(bus, device, func, offset, value);
					}
				}
				else 
				if ((strcmp(argv[0], "pci_read_long") == 0) || (strcmp(argv[0], "pci_read_dword") == 0))
				{
					if (argc == 5)
					{
						unsigned char bus = StrToHex(argv[1]);
						unsigned char device = StrToHex(argv[2]);
						unsigned char func = StrToHex(argv[3]);
						unsigned char offset = StrToHex(argv[4]);
						unsigned long value = pci_read_long(bus, device, func, offset);
						printk("%s: 0x%X: 0x%X\n", argv[0], offset, value);
					}
				}
				else 
				if ((strcmp(argv[0], "pci_write_long") == 0) || (strcmp(argv[0], "pci_write_dword") == 0))
				{
					if (argc == 6)
					{
						unsigned char bus = StrToHex(argv[1]);
						unsigned char device = StrToHex(argv[2]);
						unsigned char func = StrToHex(argv[3]);
						unsigned char offset = StrToHex(argv[4]);
						unsigned long value = StrToHex(argv[5]);
						printk("%s: 0x%X: 0x%X\n", argv[0], offset, value);
						pci_write_long(bus, device, func, offset, value);
					}
				}
				else 
				if (strcmp(argv[0], "pci") == 0)
				{
					if (argc > 1)
					{
						int i = atol(argv[1]);
						if (i < pci_count)
						{
							unsigned char _pci_bus = pci_device[i].bus;
							unsigned char _pci_dev = pci_device[i].slot;
							unsigned char _pci_fnc = pci_device[i].function;
							pci_scan_device(_pci_bus, _pci_dev, _pci_fnc, i);
							if (pci_device[i].pci.vendor != 0xFFFF)
							{
								printk("\n");	
								printk("PCI Device: 0x%X:0x%X, bus: %d, device: %d, function: %d\n    Name: %s\n", 
								pci_device[i].pci.vendor, pci_device[i].pci.device, pci_device[i].bus, pci_device[i].slot, pci_device[i].function, 
								get_pci_class_name(pci_device[i].pci.class, pci_device[i].pci.subclass));
								if ((pci_device[i].pci.interrupt_line != 0) && (pci_device[i].pci.interrupt_pin != 0))
								{
									char irq_pin = ('A' + (pci_device[i].pci.interrupt_pin - 1));
									printk("    IRQ: %d, Pin: %c\n", pci_device[i].pci.interrupt_line, irq_pin);
								}
								for(int j=0;j<6;j++)
								{
									 if (pci_device[i].pci.bar[j] != 0)
									 {
										printk("    BAR%d: 0x%X\n", j, pci_device[i].pci.bar[j]);
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
								printk("\n");		
							}	
							else
							{
								printk("Cannot read pci port.\n");
							}
						}
						else
						{
							printk("Cannot read pci port.\n");
						}
					}
					else
					{
						if (pci_count)
						{
							printk("\n");	
							for(int i=0;i<pci_count;i++) 
							{
								printk("PCI Device: 0x%X:0x%X, bus: %d, device: %d, function: %d\n    Name: %s\n", 
								pci_device[i].pci.vendor, pci_device[i].pci.device, pci_device[i].bus, pci_device[i].slot, pci_device[i].function, 
								get_pci_class_name(pci_device[i].pci.class, pci_device[i].pci.subclass));
								if ((pci_device[i].pci.interrupt_line != 0) && (pci_device[i].pci.interrupt_pin != 0))
								{
									char irq_pin = ('A' + (pci_device[i].pci.interrupt_pin - 1));
									printk("    IRQ: %d, Pin: %c\n", pci_device[i].pci.interrupt_line, irq_pin);
								}
								for(int j=0;j<6;j++)
								{
									 if (pci_device[i].pci.bar[j] != 0)
									 {
										printk("    BAR%d: 0x%X\n", j, pci_device[i].pci.bar[j]);
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
								printk("\n");		
							}
						}						
					}
				}
				else 
				if (strncmp(argv[0], "jump",4) == 0)
				{
					if (argc > 1)
					{
						unsigned long address = StrToHex(argv[1]);
						printk("Jumping to address: 0x%X\n", address);
						__asm__ volatile ("jmp *%0\n" : : "a"(address));
					}
				}
				else 
				if (strcmp(argv[0], "sys_vars") == 0)
				{
					if (sys_vars_info->count == 0)
					{
						printk("Does not have system variables yet.\n");
					}
					else
					{
						list_sys_vars();
					}
				}
				else
				if (strcmp(argv[0], "sys_enum") == 0)
				{
					if (sys_enum_info->count == 0)
					{
						printk("Does not have system enumerations yet.\n");
					}
					else
					{
						list_sys_enum();
					}
				}
				else
				if (strcmp(argv[0], "delay") == 0)
				{
					if (usermode == 1)
					{
						printk("Cannot execute this command.\n");
					}
					else
					{
						msleep(1000);
					}
				}
				else
				if (strcmp(argv[0], "kernelmode") == 0)
				{
					if (usermode == 0)
					{
						printk("Kernel Mode already active.\n");
					}
					else
					{
						printk("Switching to Kernel Mode.\n");
						switchtokernelmode();
					}
				}
				else
				if (strcmp(argv[0], "usermode") == 0)
				{
					if (usermode == 1)
					{
						printk("User Mode already active.\n");
					}
					else
					{
						printk("Switching to User Mode.\n");
						switchtousermode();
					}
				}
				else
				if (strcmp(argv[0], "realmode") == 0)
				{
					test_realmode();
				}
				else 
				if (strcmp(argv[0], "meminfo") == 0)
				{
					unsigned long total_memory_kb = physical_memory_l / 1024;
					unsigned long available_memory_kb = (_heap_size/1024)+(_heap_alloc_available/1024);
					unsigned long free_memory_kb = total_memory_kb-available_memory_kb;
					printk("%-15s %15u kB\n", "MemTotal:", total_memory_kb);
					printk("%-15s %15u kB\n", "MemFree:", free_memory_kb);
					printk("%-15s %15u kB\n", "MemAvailable:", available_memory_kb);
				}
				else 
				if (strcmp(argv[0], "memorymap") == 0)
				{
					unsigned long mm_opts = 0;
					
					if (argc > 1)
					{
						for(int k=1;k<argc;k++)
						{
							if ((strcmp(argv[k], "-a") == 0) || (strcmp(argv[k], "--all") == 0))
							{
								mm_opts |= PRINT_MEMORY_MAP_BIOS_MMAP;
								mm_opts |= PRINT_MEMORY_MAP_SYSTEM_MMAP;
								mm_opts |= PRINT_MEMORY_MAP_ENTRY_COUNT;
								mm_opts |= PRINT_MEMORY_MAP_RAM_AVAILABLE;
								mm_opts |= PRINT_MEMORY_MAP_RAM_RESERVED;
								mm_opts |= PRINT_MEMORY_MAP_LIST_OFFSET;
								mm_opts |= PRINT_MEMORY_MAP_LIST_DETAIL;
							}
							else
							{
								if ((strcmp(argv[k], "-b") == 0) || (strcmp(argv[k], "--bios") == 0))
								{
									mm_opts |= PRINT_MEMORY_MAP_BIOS_MMAP;
								}
								if ((strcmp(argv[k], "-s") == 0) || (strcmp(argv[k], "--system") == 0))
								{
									mm_opts |= PRINT_MEMORY_MAP_SYSTEM_MMAP;
								}
								if (
								((strcmp(argv[k], "-e") == 0) || (strcmp(argv[k], "--entry") == 0))
								||
								((strcmp(argv[k], "-c") == 0) || (strcmp(argv[k], "--count") == 0))
								)
								{
									mm_opts |= PRINT_MEMORY_MAP_ENTRY_COUNT;
								}
								if (
								((strcmp(argv[k], "-u") == 0) || (strcmp(argv[k], "--usable") == 0))
								||
								((strcmp(argv[k], "-use") == 0) || (strcmp(argv[k], "--used") == 0))
								||
								((strcmp(argv[k], "--avail") == 0) || (strcmp(argv[k], "--available") == 0))
								)
								{
									mm_opts |= PRINT_MEMORY_MAP_RAM_AVAILABLE;
								}
								if (
								((strcmp(argv[k], "-r") == 0) || (strcmp(argv[k], "--reserved") == 0))
								||
								((strcmp(argv[k], "-f") == 0) || (strcmp(argv[k], "--free") == 0))
								||
								((strcmp(argv[k], "-p") == 0) || (strcmp(argv[k], "--space") == 0))
								)
								{
									mm_opts |= PRINT_MEMORY_MAP_RAM_RESERVED;
								}
								if (
								((strcmp(argv[k], "-o") == 0) || (strcmp(argv[k], "--offset") == 0))
								||
								((strcmp(argv[k], "-q") == 0) || (strcmp(argv[k], "--area") == 0))
								)
								{
									mm_opts |= PRINT_MEMORY_MAP_LIST_OFFSET;
								}
								if (
								((strcmp(argv[k], "-l") == 0) || (strcmp(argv[k], "--list") == 0))
								||
								((strcmp(argv[k], "-d") == 0) || (strcmp(argv[k], "--detail") == 0))
								)
								{
									mm_opts |= PRINT_MEMORY_MAP_LIST_DETAIL;
								}
							}
						}
					}
					else
					{
						mm_opts |= PRINT_MEMORY_MAP_LIST_DETAIL;
					}
					printmemorymap(mm_opts);
				}
				else
				{
					char fn[256];
					int is_fn_exec=0;
					strcpy(fn, argv[0]);
					is_fn_exec = execv(fn, (const char **)argv);
					if (!is_fn_exec)
					{
						cmdnotfound("", argv[0]);
					}
					/*
					if (strcmp(strchr(strlwr(fn), '.'), ".bin") == 0)
					{
						if (fileexists(fn))
						{
							is_fn_exec = 1;
						}
					}
					else
					{
						strcat(fn, ".bin");
						if (fileexists(fn))
						{
							is_fn_exec = 1;
						}
					}
										
					if (is_fn_exec)
					{
						strcpy(argv[0], strlwr(fn));
						push_module(strlwr(fn));
						int fsz = getfilesize(fn);
						unsigned char *fdata = (unsigned char*)malloc(fsz);
						getfiledata(fn, fdata);
						syscall_exec(fdata, argc, (const char **)argv);
						free(fdata);
					}
					else
					{
						cmdnotfound("", argv[0]);
					}
					*/
				}
			}
			else
			{
				putch('\n');
				if (strlen(lastcmd) > 0)
				{
					strcpy(sh, lastcmd);
					strcpy(lastcmd, "");
					goto shell_exec_cmd;
				}
			}
		}
	}
	
	free(sh);
	uninit_module_stack();
	
	return 0;
}
