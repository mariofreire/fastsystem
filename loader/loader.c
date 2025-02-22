// Fast System Kernel Loader
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2024 DSP Interactive.

#include <stdarg.h>
#include "enum.h"
#include "funcbase.h"
#include "ecs.h"
#include "pci.h"
#include "ahci.h"
//#include "ehci.h"
//#include "ohci.h"
//#include "uhci.h"
//#include "xhci.h"

extern unsigned long probememory(void);
extern void startkernel(void);

unsigned char *root_sector = (unsigned char *)0x9000;
//unsigned char *root_dir_sector = (unsigned char*)0xa000;

unsigned char *mbr_sector = (unsigned char *)0x8400;
unsigned char *boot_sector = (unsigned char *)0x8600;
unsigned char *disk_address_packet = (unsigned char *)0x8200;

unsigned char *vga_memory = (unsigned char *)0xB8000;
unsigned char *video_memory = (unsigned char *)0xA0000;
unsigned char *vesa_info_buffer = (unsigned char *)0x8800;
unsigned char *vesa_mode_buffer = (unsigned char *)0x8A00;


unsigned char *kernel = (unsigned char *)0xC000000;

unsigned char *system_variables = (unsigned char *)0x800000;
unsigned char *system_variables_info = (unsigned char *)0x808000;

unsigned char *system_variables_enum = (unsigned char *)0x810000;
unsigned char *system_variables_enum_info = (unsigned char *)0x818000;

unsigned char *system_info = (unsigned char *)0x840000;


#define TEXT_COLS                             80
#define TEXT_ROWS                             25


#define SECTORSIZE 512

#define PARTITION_ACTIVE 0x80
#define PARTITION_INACTIVE 0x00

#define MAX_PARTITION 4

#define BOOT_SIGNATURE 0xAA55

#define PARTITION_FAT16 0x06
#define PARTITION_FAT16_LBA 0x0E
#define PARTITION_FAT32 0x0B
#define PARTITION_FAT32_LBA 0x0C

#define PARTITION_UNKNOWN 0x00
#define PARTITION_EXTENDED 0x05
#define PARTITION_EXTENDED_LBA 0x0F


#define MBR_BOOTSTRAP_SIZE 0x1BE
#define FAT32_BOOTSTRAP_SIZE 0x1A4
//#define FAT16_BOOTSTRAP_SIZE 0x1C0
#define FAT16_BOOTSTRAP_SIZE 0x1C

#define FAT_ENTRY_SIZE 32

#define F_ATTR_NORMAL 0x00
#define F_ATTR_RDONLY 0x01
#define F_ATTR_HIDDEN 0x02
#define F_ATTR_SYSTEM 0x04
#define F_ATTR_VOLMID 0x08
#define F_ATTR_DIRECT 0x10
#define F_ATTR_ARCHVE 0x20
#define F_ATTR_LNGFNM 0x0F

#define FILE_NAME_DELETED 0xE5
#define FILE_NAME_DIRECTORY 0x2E


typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef unsigned long long QWORD;


#define INT86_BASE_ADDRESS 0x7C00

#define CHAR_BACKSPACE 8
#define CHAR_TAB 9
#define CHAR_RETURN 13

#define xyoffset(_x,_y,_w) ((_w*_y) + _x)
#define xyoffset16(_x,_y,_w)  ((_w*_y) + (_x * 2))
#define xyoffset24(_x,_y,_w)  ((_w*_y) + (_x * 3))
#define xyoffset32(_x,_y,_w)  ((_w*_y) + (_x * 4))

#define BLACK                           0
#define BLUE                            1
#define GREEN                           2
#define CYAN                            3
#define RED                             4
#define MAGENTA                         5
#define BROWN                           6
#define SILVER                          7
#define GRAY                            8
#define LIGHTBLUE                       9
#define LIGHTGREEN                      10
#define LIGHTCYAN                       11
#define LIGHTRED                        12
#define LIGHTMAGENTA                    13
#define YELLOW                          14
#define WHITE                           15


#define TEXTCOLOR_DEFAULT               SILVER

#define textoffset(_x,_y) (2 * xyoffset(_x,_y,TEXT_COLS))
#define textoffsety(_offset) (_offset/(2*TEXT_COLS))
#define textoffsetx(_offset) ((_offset-(textoffsety(_offset)*2*TEXT_COLS))/2)




#define IRQ0 32
#define IRQ1 33
#define IRQ2 34
#define IRQ3 35
#define IRQ4 36
#define IRQ5 37
#define IRQ6 38
#define IRQ7 39
#define IRQ8 40
#define IRQ9 41
#define IRQ10 42
#define IRQ11 43
#define IRQ12 44
#define IRQ13 45
#define IRQ14 46
#define IRQ15 47



// Vendor strings from CPUs.
#define CPUID_VENDOR_AMD           "AuthenticAMD"
#define CPUID_VENDOR_AMD_OLD       "AMDisbetter!"
#define CPUID_VENDOR_INTEL         "GenuineIntel"
#define CPUID_VENDOR_VIA           "VIA VIA VIA "
#define CPUID_VENDOR_TRANSMETA     "GenuineTMx86"
#define CPUID_VENDOR_TRANSMETA_OLD "TransmetaCPU"
#define CPUID_VENDOR_CYRIX         "CyrixInstead"
#define CPUID_VENDOR_CENTAUR       "CentaurHauls"
#define CPUID_VENDOR_NEXGEN        "NexGenDriven"
#define CPUID_VENDOR_UMC           "UMC UMC UMC "
#define CPUID_VENDOR_SIS           "SiS SiS SiS "
#define CPUID_VENDOR_NSC           "Geode by NSC"
#define CPUID_VENDOR_RISE          "RiseRiseRise"
#define CPUID_VENDOR_VORTEX        "Vortex86 SoC"
#define CPUID_VENDOR_AO486         "MiSTer AO486"
#define CPUID_VENDOR_AO486_OLD     "GenuineAO486"
#define CPUID_VENDOR_ZHAOXIN       "  Shanghai  "
#define CPUID_VENDOR_HYGON         "HygonGenuine"
#define CPUID_VENDOR_ELBRUS        "E2K MACHINE "

// Vendor strings from hypervisors.
#define CPUID_VENDOR_QEMU          "TCGTCGTCGTCG"
#define CPUID_VENDOR_KVM           " KVMKVMKVM  "
#define CPUID_VENDOR_VMWARE        "VMwareVMware"
#define CPUID_VENDOR_VIRTUALBOX    "VBoxVBoxVBox"
#define CPUID_VENDOR_XEN           "XenVMMXenVMM"
#define CPUID_VENDOR_HYPERV        "Microsoft Hv"
#define CPUID_VENDOR_PARALLELS     " prl hyperv "
#define CPUID_VENDOR_PARALLELS_ALT " lrpepyh vr "
#define CPUID_VENDOR_BHYVE         "bhyve bhyve "
#define CPUID_VENDOR_QNX           " QNXQVMBSQG "

// Vendor id from CPUs.
#define VENDOR_INTEL      1
#define VENDOR_UMC        2
#define VENDOR_AMD        3
#define VENDOR_CYRIX      4
#define VENDOR_NEXGEN     5
#define VENDOR_CENTAUR    6
#define VENDOR_RISE       7
#define VENDOR_SIS	  	  8
#define VENDOR_TRANSMETA  9
#define VENDOR_NSC	     10
#define VENDOR_HYGON	 11
#define VENDOR_ZHAOXIN   12
#define VENDOR_UNKNOWN   99

// CPU vendor id string from CPUs.
#define CPUID_ID_INTEL      	"Intel"
#define CPUID_ID_UMC        	"UMC"
#define CPUID_ID_AMD        	"AMD"
#define CPUID_ID_CYRIX      	"Cyrix"
#define CPUID_ID_NEXGEN     	"NexGen"
#define CPUID_ID_CENTAUR    	"Centaur"
#define CPUID_ID_RISE       	"Rise"
#define CPUID_ID_SIS	  	  	"SiS"
#define CPUID_ID_TRANSMETA  	"Transmeta"
#define CPUID_ID_NSC	     	"NSC"
#define CPUID_ID_HYGON	 		"Hygon"
#define CPUID_ID_ZHAOXIN   		"Zhaoxin"
#define CPUID_ID_UNKNOWN   		"x86"
#define CPUID_ID_GENERIC_X86 	"x86"
#define CPUID_ID_GENERIC_X64	"x64"



#define ATA_FLAGS_ECC	0x80
#define ATA_FLAGS_LBA	0x40
#define ATA_FLAGS_SEC	0x20
#define ATA_FLAGS_SLV	0x10

#define ATA_STATUS_BSY	0x80
#define ATA_STATUS_RDY	0x40

#define STORAGE_CONTROLLER_NONE    	0x0000
#define STORAGE_CONTROLLER_IDE     	0x0010
#define STORAGE_CONTROLLER_SCSI    	0x0020
#define STORAGE_CONTROLLER_AHCI    	0x0040
#define STORAGE_CONTROLLER_UHCI    	0x0080
#define STORAGE_CONTROLLER_OHCI    	0x0100
#define STORAGE_CONTROLLER_EHCI    	0x0200
#define STORAGE_CONTROLLER_XHCI    	0x0400
#define STORAGE_CONTROLLER_NVME    	0x0800
#define STORAGE_CONTROLLER_OTHER   	0x1000
#define STORAGE_CONTROLLER_UNKNOWN 	0xFFFF

#define MAXKEYSHANDLERS 2

#define MAX_ENUM 128
#define MAX_VARS 128


#define nearptr(segment, offset) ((char*)((segment)*16UL+(offset)))

#pragma pack (push, 1)

typedef struct
{
	unsigned long edi;
	unsigned long esi;
	unsigned long ebp;
	unsigned long esp;
	unsigned long ebx;
	unsigned long edx;
	unsigned long ecx;
	unsigned long eax;
} cpu_state_t;

typedef struct
{
	unsigned long int_no;
	unsigned long err_code;
	unsigned long eip;
	unsigned long cs;
	unsigned long eflags;
	unsigned long useresp;
	unsigned long ss;
} stack_state_t;

typedef struct
{
	unsigned short segment_limit;
	unsigned short low_base;
	unsigned char mid_base;
	unsigned char access;
	unsigned char flags;
	unsigned char high_base;
} gdt_entry_t;

typedef struct
{
	unsigned short limit;
	unsigned long base;
} gdt_register_t;

typedef struct
{
	gdt_entry_t entry[8];
	unsigned char limit;
	gdt_register_t descriptor;
} gdt_t;

typedef struct
{
	unsigned short low_offset;
	unsigned short segment_sel;
	unsigned char unused;
	unsigned char flags;
	unsigned short high_offset;
} idt_gate_t;

typedef struct
{
	unsigned short limit;
	unsigned long base;
} idt_register_t;

typedef struct
{
	idt_gate_t entry[256];
	unsigned char limit;
	idt_register_t descriptor;
} idt_t;

typedef struct
{
	unsigned long ds;
	unsigned long edi;
	unsigned long esi;
	unsigned long ebp;
	unsigned long esp;
	unsigned long ebx;
	unsigned long edx;
	unsigned long ecx;
	unsigned long eax;
	unsigned long int_no;
	unsigned long error_code;
	unsigned long eip;
	unsigned long cs;
	unsigned long eflags;
	unsigned long useresp;
	unsigned long ss;
} registers_t;

typedef struct
{
    unsigned short di;
	unsigned short si;
	unsigned short bp;
	unsigned short sp;
	unsigned short bx;
	unsigned short dx;
	unsigned short cx;
	unsigned short ax;
    unsigned short gs;
	unsigned short fs;
	unsigned short es;
	unsigned short ds;
	unsigned short flags;
} registers16_t;

typedef struct
{
	unsigned char head;
	unsigned char sector;
	unsigned char cylinder;
} chs_t;

typedef struct
{
	unsigned char flag;
	chs_t chs_start;
	unsigned char type;
	chs_t chs_end;
	unsigned long lba_start;
	unsigned long lba_end;
} partition_entry_t;

typedef struct
{
	unsigned char bootstrap[MBR_BOOTSTRAP_SIZE];
	partition_entry_t partition[4];
	unsigned short signature;
} mbr_t;

typedef struct
{
	unsigned short bytes_per_sector;
	unsigned char sector_per_cluster;
	unsigned short reserved_sectors_count;
	unsigned char number_fats;
	unsigned short root_entries_count;
	unsigned short total_sectors_16;
	unsigned char media;
	unsigned short fat_size_16;
	unsigned short sectors_per_track;
	unsigned short number_heads;
	unsigned long hidden_sectors;
	unsigned long total_sectors_32;
} fat_bpb1_t;

typedef union
{
	struct
	{
		unsigned char drive_number;
		unsigned char reserved;
		unsigned char boot_signature;
		unsigned long volume_id;
		unsigned char volume_label[11];
		unsigned char type[8];
		unsigned char bootstrap[FAT16_BOOTSTRAP_SIZE];
	} fat16;
	struct
	{
		unsigned long fat_size_32;
		unsigned short flags;
		unsigned short version;
		unsigned long root_cluster;
		unsigned short fs_info;
		unsigned short backup_boot_sector;
		unsigned char reserved_0[12];
		unsigned char drive_number;
		unsigned char reserved_1;
		unsigned char boot_signature;
		unsigned long volume_id;
		unsigned char volume_label[11];
		unsigned char type[8];
	} fat32;
} fat_bpb2_t;

typedef struct
{
	fat_bpb1_t bpb1;
	fat_bpb2_t bpb2;
} fat_bpb_t;

typedef struct
{
	unsigned char jump_opcode;
	unsigned char jump_boot;
	unsigned char jump_boot2;
	unsigned char oem_name[8];
	fat_bpb_t bpb;
	unsigned char bootstrap[FAT32_BOOTSTRAP_SIZE];
	unsigned short signature;
} fat_t;

typedef struct
{
	char name[11];
	unsigned char attribute;
	unsigned char reserved;
	unsigned char creation_time_tenth;
	unsigned short creation_time;
	unsigned short creation_date;
	unsigned short last_date;
	unsigned short first_cluster_hi;
	unsigned short write_time;
	unsigned short write_date;
	unsigned short first_cluster_lo;
	unsigned long size;
} file_entry_t;

typedef struct
{
    char path[256];
} path_t;

typedef struct
{
path_t path[32];
int pathcount;
} path_sub_t;

typedef struct
{
	char name[12];
	unsigned short attribute;
	unsigned char creation_time_tenth;
	unsigned short creation_time;
	unsigned short creation_date;
	unsigned short last_date;
	unsigned short write_time;
	unsigned short write_date;
	unsigned long first_cluster;
	unsigned long size;
} file_dir_t;

typedef struct
{
	char signature[4];
	unsigned short version;
	unsigned long oem;
	unsigned long capabilities;
	unsigned long mode_list;
	unsigned short video_memory_size;
	char reserved_0[236];
	char reserved_1[256];
} vesa_info_t;

typedef struct
{
	unsigned short mode_attributes;
	unsigned char window_a_attributes;
	unsigned char window_b_attributes;
	unsigned short window_granularity;
	unsigned short window_size;
	unsigned short window_a_segment;
	unsigned short window_b_segment;
	unsigned long window_far_ptr;
	unsigned short scan_line_size;
	unsigned short width;
	unsigned short height;
	unsigned char char_width;
	unsigned char char_height;
	unsigned char planes;
	unsigned char depth;
	unsigned char banks;
	unsigned char memory_model;
	unsigned char bank_size;
	unsigned char pages;
	char reserved_0;
	unsigned char red_width;
	unsigned char red_shift;
	unsigned char green_width;
	unsigned char green_shift;
	unsigned char blue_width;
	unsigned char blue_shift;
	char reserved_1[3];
	unsigned long lfb_address;
	char reserved_2[212];
} vesa_mode_t;

typedef struct
{
	char name[128];
	char value[128];
} sys_vars_t;

typedef struct
{
	char signature[4];
	unsigned short version;
	unsigned char reserved;
	unsigned char machine;
	unsigned short flags;
	unsigned long count;
	unsigned long checksum;
	unsigned long key;
	unsigned long crc;
	char id[38];
} sys_vars_info_t;

typedef struct
{
	char name[MAX_ENUM][65];
	int value[MAX_ENUM];
} sys_enum_t;

typedef struct
{
	char signature[4];
	unsigned short version;
	unsigned char reserved;
	unsigned char machine;
	unsigned short flags;
	unsigned long count;
	unsigned long checksum;
	unsigned long key;
	unsigned long crc;
	char id[38];
} sys_enum_info_t;

typedef struct
{
	char signature[4];
	unsigned long version;
	unsigned short flags;
	unsigned char unused;
	unsigned long multi_boot;
	unsigned long heap_1_size;
	unsigned long heap_2_size;
	unsigned long heap_3_size;
	unsigned long heap_4_size;
	unsigned long physical_memory;
	unsigned long cpu_speed;
	unsigned long cpu_id_family;
	unsigned long cpu_id_model;
	unsigned long cpu_id_stepping;
	unsigned long cpu_id_type;
	unsigned char cpu_id_longmode;
	char cpu_id_name[32];
	char cpu_id_str[64];
	char cpu_brand_str[256];
	char reserved[104];
} system_info_t;

#pragma pack (pop)

mbr_t* mbr;

sys_vars_t *sys_vars;
sys_vars_info_t *sys_vars_info;

sys_enum_t *sys_enum;
sys_enum_info_t *sys_enum_info;

system_info_t *info;


int active_partition=-1;
partition_entry_t* partition;
partition_entry_t *main_partition;

fat_t *fat;

file_entry_t file_dir_sector[16];

unsigned short storage_drive_controller = STORAGE_CONTROLLER_NONE;

int mbr_loaded=0;

void remap_mbr(void);
void remap_boot(void);

gdt_t gdt;
idt_t idt;

vesa_info_t *vesa_info;
vesa_mode_t *vesa_mode;

unsigned char bootstrap[FAT32_BOOTSTRAP_SIZE];

int sys_vars_loaded = 0;
int sys_enum_loaded = 0;

#define vars_loaded                     (sys_vars_loaded == 1)
#define enum_loaded                     (sys_enum_loaded == 1)

#define has_not_enum(_enum)             ((((sys_enum->value[_enum]) == 0) ? 1 : 0) && (sys_enum_info->count > _enum) && (sys_enum_loaded == 1))
#define has_enum(_enum)                 ((((sys_enum->value[_enum]) != 0) ? 1 : 0) && (sys_enum_info->count > _enum) && (sys_enum_loaded == 1))
#define get_enum(_enum)                 (sys_enum->value[_enum])
#define total_enum                      (sys_enum_info->count)

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


extern void halt(void);


typedef void (*isr_t)(registers_t *);
void register_interrupt_handler(unsigned char n, isr_t handler);

isr_t interrupt_handlers[256];

char *loaderfilename = "loader";
char *kernelfilename = "FSKRNL";

int current_text_x = 0;
int current_text_y = 0;

unsigned short timer_frequency = 100;

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

unsigned short putchar_xy(int x, int y, const char c, unsigned char a) 
{
	int i,j;
	unsigned char *vga;
	unsigned short cursor_offset;	
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
		sputchar(c);
	}
	else cputchar_a(a, c);
}

void cprint(unsigned char a, const char *s)
{
    while(*s)
	{
		cputchar(a, *s++);
    }
}

void putchar(const char c) 
{
	cputchar(TEXTCOLOR_DEFAULT, c);
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

void panic(unsigned long exception_code)
{
	printk("System Halted: Panic at %s: KernelException(0x%x).\n", loaderfilename, exception_code);
	// printk("Error: Cann't open a operating system.\n");
	halt();
}

void enable_interrupt(void)
{
	__asm__ ("sti");
}

void disable_interrupt(void)
{
	__asm__ ("cli");
}

void setgdt(int n, unsigned long limit, unsigned long base, unsigned char access, unsigned char flags)
{
	gdt.entry[n].segment_limit = (limit & 0xFFFF);
	gdt.entry[n].low_base = (base & 0xFFFF);
	gdt.entry[n].mid_base = ((base & 0xFF0000) >> 16);
	gdt.entry[n].access = access;
	gdt.entry[n].flags = (limit >> 16) & 0x0F;
	gdt.entry[n].flags |= flags & 0xF0;
	gdt.entry[n].high_base = ((base & 0xFF000000) >> 24);
	gdt.limit = n + 1;
}

void setgdt_entries(void)
{
	setgdt(0, 0x00000000, 0x00000000, 0x00, 0x00);
	setgdt(1, 0xFFFFFFFF, 0x00000000, 0x9A, 0xCF);
	setgdt(2, 0xFFFFFFFF, 0x00000000, 0x92, 0xCF);
	setgdt(3, 0x00000000, 0x00000000, 0x00, 0x00);
	setgdt(4, 0x00000000, 0x00000000, 0x00, 0x00);
	setgdt(5, 0x00000000, 0x00000000, 0x00, 0x00);
	setgdt(6, 0xFFFFFFFF, 0x00000000, 0x9A, 0x0F);
	setgdt(7, 0xFFFFFFFF, 0x00000000, 0x92, 0x0F);
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
	idt.limit = n + 1;
}

void loadidt(void)
{
	idt.descriptor.limit = 256*sizeof(idt_gate_t)-1;
	idt.descriptor.base = (unsigned long)&idt.entry;
	__asm__ volatile ("lidt %[idtr]"
				 : 
				 : [idtr] "m" (idt.descriptor));
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
	printk("Exception at interrupt %d", registers->int_no);
	
	if (registers->int_no < 32)
	{
		printk(": %s", exception_messages[registers->int_no]);
	}
	
	putchar('\n');
	panic(exc);
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

void loadirq(void)
{	
	remap_irq();
    setidt(32, (unsigned long)irq0, 0x08, 0x8E);
    setidt(33, (unsigned long)irq1, 0x08, 0x8E);	
    setidt(45, (unsigned long)irq13, 0x08, 0x8E);	
    setidt(46, (unsigned long)irq14, 0x08, 0x8E);
    setidt(47, (unsigned long)irq15, 0x08, 0x8E);
}

void register_interrupt_handler(unsigned char n, isr_t handler)
{
	interrupt_handlers[n] = handler;
}

void irq_handler(registers_t *registers)
{
	if (registers->int_no >= 40)
	{
		outb(0xA0, 0x20);
	}
	outb(0x20, 0x20);

	if (interrupt_handlers[registers->int_no] != 0)
	{
		isr_t handler = interrupt_handlers[registers->int_no];
		handler(registers);
	}
	outb(0x20, 0x20);
}

volatile unsigned long timertick = 0;
volatile double timertick_ms = 0;

void timer_handler(registers_t *registers)
{
	timertick++;
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

void msleep(unsigned int milliseconds)
{
	unsigned long ms;
	//remap_irq();
	init_timer(timer_frequency);
	ms = timertick + ((timer_frequency * milliseconds) / 1000);
	while (timertick < ms);
}

void sleep(int seconds)
{
	unsigned long ms;
	//remap_irq();
	init_timer(timer_frequency);
	ms = timertick + (seconds * timer_frequency);
	while (timertick < ms);
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
  msleep(10);
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

#define HEAP_START 0xC00000
#define HEAP_END 0x1800000
#define PAGE_SIZE 0x1000
#define ALLOC_SIZE_HEADER  8

void * _heap_start;
void * _heap_end;
void * _heap_current;

unsigned long _heap_last_size_alloc;
unsigned long _heap_last_position;
unsigned long _heap_position;
unsigned long _heap_size;

unsigned long _heap_alloc_last_clean_start;
unsigned long _heap_alloc_last_clean_end;

void init_heap(void)
{
	_heap_last_size_alloc = 0;
	_heap_position = HEAP_START + PAGE_SIZE + ALLOC_SIZE_HEADER;
	_heap_last_position = _heap_position;
	_heap_start = (void*)HEAP_START;
	_heap_end = (void*)HEAP_END;
	_heap_current = (void*)_heap_position;
	_heap_size = HEAP_END-HEAP_START;
	_heap_alloc_last_clean_start = 0;
	_heap_alloc_last_clean_end = 0;
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
	_heap_current += _heap_last_size_alloc+ALLOC_SIZE_HEADER;
	_heap_last_size_alloc = size;
	_heap_last_position += _heap_last_size_alloc+ALLOC_SIZE_HEADER;
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
	if (alloc_size == 0 || alloc_pos == 0)
	{
		for(int i=0;i<4;i++) pos.d[i] = alloc_header[i];
		for(int i=0;i<4;i++) siz.d[i] = alloc_header[i+4];
		alloc_pos = pos.l;
		alloc_size = siz.l;
	}
	int i;
	i = 0;
	while(i < alloc_size+ALLOC_SIZE_HEADER)
	{
		if (*alloc_ptr == 0) break;
		*alloc_ptr++ = 0;
		alloc_ptr--;
		i++;
	};
	_heap_alloc_last_clean_start = alloc_pos;
	_heap_alloc_last_clean_end = alloc_pos+alloc_size;
}

const int ata_delay = 1;

const unsigned short ata_base[4] =
{
	0x1F0,
	0x1F0,
	0x170,
	0x170
};

void ata_reset(int id)
{
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

void detectide(void)
{
	int i;
	char ata_ide_name[40][4];
	char *ata_ide_order[4] = {"Primary IDE Master", "Primary IDE Slave", "Secondary IDE Master", "Secondary IDE Slave"};
	enable_interrupt();
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
extern void int86_int_no();

#define get_base_address(x)  (void*)(INT86_BASE_ADDRESS + (void*)x - (unsigned long)int86_start)
void (*exec_int86_code16)() = (void *)INT86_BASE_ADDRESS;


void int86(unsigned char int_no, registers16_t *regs_in, registers16_t *regs_out)
{
	unsigned long sz = (unsigned long)int86_end - (unsigned long)int86_start;
	void *code_base_16 = (void*)INT86_BASE_ADDRESS;
	void *code_regs_1 = (void*)int86_regs;
	void *code_regs_16 = (void*)get_base_address(code_regs_1);
	void *code_int_1 = (void*)int86_int_no;
	void *code_int_16 = get_base_address(code_int_1);
	memcpy(code_base_16, int86_start, sz);
	memcpy(code_regs_16, regs_in,  sizeof(registers16_t));
	memset(code_int_16, int_no, 1);
	exec_int86_code16();
	memcpy(regs_out, code_regs_16, sizeof(registers16_t));
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
	registers16_t regs;
	memset(vesa_mode_buffer, 0, 256);
	memset(&regs, 0, sizeof(registers16_t));
	regs.ax = 0x4F01;
	regs.cx = mode;
	regs.es = 0;
	regs.di = (unsigned long)vesa_mode_buffer;
	int86(0x10, &regs, &regs);
	vesa_mode = (vesa_mode_t*)vesa_mode_buffer;
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

int pvsnprintf(char* str, size_t size, const char* format, va_list ap)
{
	char buf[64];
	char *s;
	int i = 0;
	for (;*format;++format)
	{
		switch (*format)
		{
			case '%':
			{
				switch(*(++format))
				{
					case 's':
					{
						s = va_arg(ap, char*);
						while(*s)
						{
							*str++ = *s++;
						}
						*s++ = '\0';
					}
					break;
					case 'd':
					case 'i':
					{
						itoa(va_arg(ap, long), buf, 10);
						s = buf;
						while(*s)
						{
							*str++ = *s++;
						}
						*s++ = '\0';
					}
					break;
					case 'x':
					{
						s = itob(va_arg(ap, long), 16);
						while(*s)
						{
							*str++ = *s++;
						}
						*s++ = '\0';
					}
					break;
					case 'X':
					{
						s = strupr(itob(va_arg(ap, long), 16));
						while(*s)
						{
							*str++ = *s++;
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

void printk(const char *msg, ...)
{
	int slen = strlen(msg)*4+1;
	char msgBuf[1024];// = (char*)malloc(slen);
	va_list va_alist;

	if (!msg) return;

	msgBuf[slen - 1] = '\0';
	msgBuf[1024 - 1] = '\0';
	va_start(va_alist, msg);
	vsprintf(msgBuf, msg, va_alist);
	va_end(va_alist);

	cprint(TEXTCOLOR_DEFAULT, msgBuf);
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
			panic((unsigned long)buffer);
		}
		break;
		default:
		{
			panic((unsigned long)buffer);			
		}
		break;
	};
	return r;
}

unsigned char readsector(unsigned long sector, unsigned char *buffer)
{
	unsigned char result;
	unsigned long offset = (unsigned long)buffer;
	int id = 0;
	int i = 30;
	
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
	mbr_loaded = 0;
	enable_interrupt();
	if (!readsector(0, mbr_sector)) return 0;
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
	unsigned long sector;
	if (!has_partition_active()) return 0;
	if (!isfattype()) return 0;
	sector = main_partition->lba_start;
	readsector(sector, boot_sector);
	fat = (fat_t*)boot_sector;
	if (fat->signature != BOOT_SIGNATURE) return 0;
	memcpy(bootstrap, fat->bootstrap, FAT32_BOOTSTRAP_SIZE);
	remap_boot();
	return 1;
}

void remap_fat(void)
{
	fat = (fat_t*)boot_sector;
	if (fat->signature != BOOT_SIGNATURE)
	{
		panic((unsigned long)fat);
	}
}

void remap_boot(void)
{
	remap_mbr();
	remap_fat();
}

unsigned long getrootdirsector(void)
{
	unsigned long start;
	unsigned long sectors;
	unsigned long root_dir;
	unsigned long fat_size;
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

unsigned long getfirstsectorofcluster(unsigned long cluster)
{
	unsigned long first_sector;
	unsigned long data_sector;
	unsigned long sector_per_cluster;
	remap_boot();
	if (!has_partition_active()) return 0;
	if (!isfattype()) return 0;
	sector_per_cluster = fat->bpb.bpb1.sector_per_cluster;
	data_sector = getdatasector();
	first_sector = (data_sector + (cluster-2) * sector_per_cluster);
	return first_sector;
}

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
	remap_boot();
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

unsigned long getfilefirstcluster(file_entry_t entry)
{
	unsigned short cluster_hi;
	unsigned short cluster_lo;
	unsigned long cluster;
	remap_boot();
	if (!has_partition_active()) return false;
	if (!isfattype()) return false;
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


unsigned char *opendir_entry(unsigned long sector)
{
	int q;
	unsigned long totalfiles;
	unsigned long entryoffset;
	unsigned long entrycount;
	unsigned long filecount;
	unsigned char dir_entry_data[FAT_ENTRY_SIZE];
	unsigned char *dir_sector;
	file_entry_t *readdir_entries;
	file_entry_t* file;
	if (!has_partition_active()) return NULL;
	if (!isfattype()) return NULL;
	entrycount = 0;
	filecount = 0;
	totalfiles = 0;
	entryoffset = 0;
	q = 0;
	dir_sector = (unsigned char*)malloc(4096*FAT_ENTRY_SIZE);
	if (dir_sector == NULL) return NULL;
	readdir_entries = (file_entry_t*)dir_sector;
	memset(readdir_entries, 0, 4096*FAT_ENTRY_SIZE);
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
				(file[filecount].attribute != F_ATTR_LNGFNM) &&
				(file[filecount].attribute != F_ATTR_VOLMID))
			{				
				if (totalfiles < 4096)
				{
					entryoffset = (totalfiles*FAT_ENTRY_SIZE)+4;
					memcpy(&dir_sector[entryoffset], &file[filecount], FAT_ENTRY_SIZE);
					totalfiles++;
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

unsigned char closedir_entry(unsigned char *dir_ptr)
{
	if (dir_ptr == NULL) return 0;	
	memset(dir_ptr, 0, 4096*FAT_ENTRY_SIZE);
	free(dir_ptr);
	return 1;
}

file_entry_t *readdir_entry(unsigned char *dir_ptr)
{
	unsigned short totalfiles = 0;
	unsigned short filecount = 0;
	file_entry_t *entry;
	file_entry_t *readdir_entries;
	totalfiles = UINT16(dir_ptr[0], dir_ptr[1]);
	filecount = UINT16(dir_ptr[2], dir_ptr[3]);
	if (!has_partition_active()) return NULL;
	if (!isfattype()) return  NULL;
	if (filecount < 0) return NULL;
	if (filecount >= totalfiles) return NULL;
	readdir_entries = (file_entry_t*)&dir_ptr[4];
	entry = (file_entry_t*)&readdir_entries[filecount];
	if (entry->name == NULL) return NULL;
	filecount++;
	dir_ptr[2] = UCHAR8A(filecount);
	dir_ptr[3] = UCHAR8B(filecount);
	return entry;
}

void file_entry_to_file_dir(file_entry_t* entry, file_dir_t* dir) {
	char fn[12];
	char filename[12];
    strncpy(dir->name, entry->name, 11);
    dir->name[11] = '\0';
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
	strfilenamedot8e3s11(fn, filename);
	strcpy(dir->name, filename);
}

void *opendir(unsigned long sector)
{
	remap_boot();
	if (!has_partition_active()) return NULL;
	if (!isfattype()) return NULL;	
	return opendir_entry(sector);
}

file_entry_t *readdir(void *dir_ptr)
{
	remap_boot();
	if (!has_partition_active()) return NULL;
	if (!isfattype()) return NULL;	
	file_entry_t* file_ptr = readdir_entry(dir_ptr);
	return file_ptr;
}

void closedir(void *dir_ptr)
{
	remap_boot();
	closedir_entry(dir_ptr);
}

unsigned char getfiledata(const char *filename, unsigned long sector, unsigned char *data)
{
	unsigned char result = 0;
	unsigned long fsecs = 0;
	unsigned long fsec_cnt = 0;
	unsigned long fsiz_pos = 0;
	unsigned long fsecstotal = 0;
	void *dir;
	file_entry_t* entry;
	file_dir_t *file = (file_dir_t*)malloc(sizeof(file_dir_t));	
	if ((dir = opendir(sector)) != NULL)
	{
		while((entry = readdir(dir)) != NULL)
		{
			file_entry_to_file_dir(entry, file);
			if (strcmp(file->name, strupr(filename)) == 0)
			{
				if ((file->attribute != F_ATTR_DIRECT) && (file->name[0] != FILE_NAME_DIRECTORY))
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
		}
		closedir(dir);		
	}
	free(file);
	return result;
}

unsigned long getfilesize(const char *filename, unsigned long sector)
{
	unsigned long result = 0;
	void *dir;
	file_entry_t* entry;
	file_dir_t *file = (file_dir_t*)malloc(sizeof(file_dir_t));	
	if ((dir = opendir(sector)) != NULL)
	{
		while((entry = readdir(dir)) != NULL)
		{
			file_entry_to_file_dir(entry, file);
			if (strcmp(file->name, strupr(filename)) == 0)
			{
				if ((file->attribute != F_ATTR_DIRECT) && (file->name[0] != FILE_NAME_DIRECTORY))
				{
					result = file->size;
				}
			}
		}
		closedir(dir);		
	}
	free(file);
	return result;
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

void loadvars(void)
{
	sys_vars = (sys_vars_t *)system_variables;
	sys_vars_info = (sys_vars_info_t *)system_variables_info;
	sys_enum = (sys_enum_t *)system_variables_enum;
	sys_enum_info = (sys_enum_info_t *)system_variables_enum_info;
	if (sys_vars_info->signature[0] == 0)
	{
		strcpy(sys_vars_info->signature, "VARS");
		sys_vars_info->version = 1;
	}
	else
	{
		if (sys_vars_info->id[0] != 0)
		{
			if (uuidv4_validate(sys_vars_info->id))
			{
				if (has_enum(SYSTEM_VERBOSE))
				{
					printk("System Variables UUID: %s\n", sys_vars_info->id);
				}
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
	}
	if (sys_enum_info->signature[0] == 0)
	{
		strcpy(sys_enum_info->signature, "ENUM");
		sys_enum_info->version = 1;
	}
	else
	{
		if (sys_enum_info->id[0] != 0)
		{
			if (uuidv4_validate(sys_enum_info->id))
			{
				if (has_enum(SYSTEM_VERBOSE))
				{
					printk("System Enumeration UUID: %s\n", sys_enum_info->id);
				}
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
	sys_vars_loaded = 1;
	sys_enum_loaded = 1;	
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
	for(i=0;i<sys_vars_info->count;i++)
	{
		strcpy(var_name, sys_vars[i].name);
		strcpy(var_value, sys_vars[i].value);
		if (strlen(var_value) == 0)
		{
			strcpy(var_value, "0");
		}
		if (strlen(var_name) != 0)
		{
			if (has_enum(SYSTEM_VERBOSE))
			{
				printk("sys_vars[\"%s\"] = %s\n", var_name, var_value);
			}
		}
	}
	for(i=0;i<sys_enum_info->count;i++)
	{
		strcpy(enum_name, sys_enum->name[i]);
		enum_value = sys_enum->value[i];
		if (strlen(enum_name) != 0)
		{
			if (has_enum(SYSTEM_VERBOSE))
			{
				printk("sys_enum[\"%s\"] = %d\n", enum_name, enum_value);
			}
		}
	}	
}


void kernelerror(void)
{
	printk("Error: Can't load kernel.\n");
	panic((unsigned long)kernel);
}

void loadkernel(void)
{
	startkernel();
}

int main(void)
{
	unsigned long physical_memory_l;
	unsigned long physical_memory;
	unsigned long used_memory;
	unsigned long used_memory_percent;
	char *cpu_id_name;
	char *cpu_id_str;
	char *cpu_brand_str;
	int cpu_id_type, cpu_id_family, cpu_id_model, cpu_id_stepping, cpu_id_longmode;
	int screen_width, screen_height, screen_depth;
	unsigned long root_sector_start;
	char filename[12];
	
	putchar('\n');
	
	printk("Loading base configuration...\n");
	
	disable_interrupt();
	loadgdt();
	loadisr();
	loadirq();
	loadidt();	
	
	loadpci();
	
	enable_interrupt();
	init_timer(timer_frequency);
	init_heap();

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
	
	printk("Physical Memory Available: %d MB\n", physical_memory);
	printk("Memory in Use: %d%%\n", used_memory_percent);
	printk("Checking Current Processor.\n");
	printk("Processor Name: %s\n", cpu_brand_str);
	printk("Processor Type: %s Family %d Model %d Stepping %d\n", cpu_id_name, cpu_id_family, cpu_id_model, cpu_id_stepping);
	printk("Processor Vendor: %s\n", cpu_id_str);
	printk("Processor Speed: %d MHz\n", cpu_speed);
	
	if (init_ahci())
	{
		storage_drive_controller = STORAGE_CONTROLLER_AHCI;
		printk("Detecting Virtual AHCI...\n");
		detectahci();
	}
	else
	{
		storage_drive_controller = STORAGE_CONTROLLER_IDE;
		printk("Detecting Virtual IDE...\n");
		detectide();
	}
	
	printk("Adjusting Screen Size.\n");
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
	printk("System Screen Width: %d\n", screen_width);
	printk("System Screen Height: %d\n", screen_height);
		
	printk("Setting system variables...\n");
	loadvars();
	init_variables();
	
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
		printk("Disk Error\n");
		panic((unsigned long)mbr_sector);
	}	
	
	if (has_partition_active())
	{
		if (isfattype())
		{
			if (loadfat())
			{
				root_sector_start = getrootdirsector();
				strcpy(filename, kernelfilename);
				unsigned long ksize = filesizeondisk(getfilesize(filename, root_sector_start));
				if (ksize == 0)
				{					
					kernelerror();
				}
				else
				{
					if (!getfiledata(filename, root_sector_start, kernel))
					{
						kernelerror();
					}
				}
			}
			else
			{
				panic((unsigned long)boot_sector);
			}
		}
		else
		{
			panic((unsigned long)mbr_sector);
		}
	}	
	else
	{
		panic((unsigned long)mbr_sector);
	}
	
	if (has_not_enum(SYSTEM_KERNEL_LOAD)) 
	{
		while(1);
	}

	loadkernel();
	
	halt();

	return 0;
}
