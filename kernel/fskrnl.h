// Fast System Kernel
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2024 DSP Interactive.

#ifndef __FSKRNL_H__
#define __FSKRNL_H__

#include "itypes.h"

#define SYSTEM_ROOT_SECTOR                    0x9000
#define SYSTEM_MBR_SECTOR                     0x8400
#define SYSTEM_BOOT_SECTOR                    0x8600
#define SYSTEM_DISK_ADDRESS_PACKET            0x8200
#define SYSTEM_VGA_MEMORY                     0xB8000
#define SYSTEM_VIDEO_MEMORY                   0xA0000
#define SYSTEM_VESA_INFO_BUFFER               0x8800
#define SYSTEM_VESA_MODE_BUFFER               0x8A00
#define SYSTEM_FAT32_FSINFO                   0x8C00
#define SYSTEM_LOADER_KERNEL                  0xA000
#define SYSTEM_KERNEL                         0xC000000
#define SYSTEM_ADDRESS_VARIABLES              0x800000
#define SYSTEM_ADDRESS_VARIABLES_INFO         0x808000
#define SYSTEM_ADDRESS_VARIABLES_ENUM         0x810000
#define SYSTEM_ADDRESS_VARIABLES_ENUM_INFO    0x818000
#define SYSTEM_ADDRESS_INFO                   0x840000
#define SYSTEM_ADDRESS_PCI                    0x840300
#define SYSTEM_ADDRESS_ERRNO                  0x850000
#define SYSTEM_ADDRESS_MEMORY_MAP_INFO        0x5A00
#define SYSTEM_ADDRESS_MEMORY_MAP             0x85A000
#define SYSTEM_AHCI_PTR                       0x841500


#define MEMORY_MAP_AVAILABLE                                   1
#define MEMORY_MAP_RESERVED                                    2
#define MEMORY_MAP_ACPI_RECLAIMABLE                            3
#define MEMORY_MAP_NVS                                         4
#define MEMORY_MAP_BADRAM                                      5



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


#define USE_DAP
#define DAP_ONLY_TRY_WHEN_ERROR

#define TEXT_COLS                             80
#define TEXT_ROWS                             25

#define CODE_SEGMENT 0x08
#define DATA_SEGMENT 0x10
#define TSS_RING 0x03
#define TSS_CODE_SEGMENT (CODE_SEGMENT|TSS_RING)
#define TSS_DATA_SEGMENT (DATA_SEGMENT|TSS_RING)
#define KERNEL_MODE_CODE_SEGMENT CODE_SEGMENT
#define KERNEL_MODE_DATA_SEGMENT DATA_SEGMENT
#define USER_MODE_CODE_SEGMENT (KERNEL_MODE_DATA_SEGMENT|TSS_CODE_SEGMENT)
#define USER_MODE_DATA_SEGMENT (KERNEL_MODE_DATA_SEGMENT|TSS_DATA_SEGMENT)

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

#define isspace(c)                      (c == ' ')
#define isnumber(c)                      ((c >= '0') && (c <= '9'))
#define isalpha(c)                      (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')))
#define isalnum(c)                      (isalpha(c) || isnumber(c))
#define issign(c)                      ((c == '-') || (c == '+') || (c == '*') || (c == '/') || (c == '='))
#define isspecial(c)                      ((c == '\\') || (c == ';') || (c == '\'') || (c == '[') || (c == ']') || (c == ',') || (c == '.'))



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


#define MAXKEYSHANDLERS 2


#define RAND_MAX 32767


#define nearptr(segment, offset) ((char*)((segment)*16UL+(offset)))

#define MAX_ENUM 128
#define MAX_VARS 128


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
	unsigned long prev_tss;
	unsigned long esp0;
	unsigned long ss0;
	unsigned long esp1;
	unsigned long ss1;
	unsigned long esp2;
	unsigned long ss2;
	unsigned long cr3;
	unsigned long eip;
	unsigned long eflags;
	unsigned long eax;
	unsigned long ecx;
	unsigned long edx;
	unsigned long ebx;
	unsigned long esp;
	unsigned long ebp;
	unsigned long esi;
	unsigned long edi;
	unsigned long es;
	unsigned long cs;
	unsigned long ss;
	unsigned long ds;
	unsigned long fs;
	unsigned long gs;
	unsigned long ldt;
	unsigned short trap;
	unsigned short iomap_base;
} tss_t;

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
    unsigned long edi;
	unsigned long esi;
	unsigned long ebp;
	unsigned long esp;
	unsigned long ebx;
	unsigned long edx;
	unsigned long ecx;
	unsigned long eax;
    unsigned short ds;
	unsigned short es;
	unsigned short fs;
	unsigned short gs;
	unsigned short ss;
    unsigned long eflags;
} registers32_t;

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
	unsigned char size;
	unsigned char unused;
	unsigned short sector_count;
	unsigned long buffer_ptr;
	unsigned long lba_start_1;
	unsigned long lba_start_2;
} dap_t;

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
	unsigned long lead_signature;
	unsigned char reserved_1[480];
	unsigned long struc_signature;
	unsigned long free_count;
	unsigned long next_free;
	unsigned char reserved_2[12];
	unsigned long trail_signature;
} fat32_fsinfo_t;

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
	char name[256];
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
  char name1[11];
  unsigned char attribute;
  char name2[20];
} file_lfn_entry_t;

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

typedef struct
{
	unsigned short vendor;
	unsigned short device;
	unsigned short command;
	unsigned short status;
	unsigned char revision;
	unsigned char progif;
	unsigned char subclass;
	unsigned char class;
	unsigned char cache;
	unsigned char lat_timer;
	unsigned char header_type;
	unsigned char bist;
	unsigned long bar[6];
	unsigned long cardbus;
	unsigned short subsystem_vendor;
	unsigned short subsystem_id;
	unsigned long rom_base_addr;
	unsigned char cap_ptr;
	unsigned char reserved0[3];
	unsigned long reserved1;
	unsigned char interrupt_line;
	unsigned char interrupt_pin;
	unsigned char min_gnt;
	unsigned char max_lat;
} pci_t;

typedef struct
{
	unsigned char bus;
	unsigned char slot;
	unsigned char function;
	pci_t pci;
} pci_device_t;

typedef struct 
{
	unsigned char class;
	unsigned char subclass;
	const char *name;
} pci_class_name_t;

typedef struct
{
	char signature[4];
	unsigned char version;
	unsigned char count;
	pci_device_t device[32];
} system_pci_t;

typedef struct
{
	char *cmdline;
	char *hypervisor;
	char *filename;
	int paramcount;
	char **params;
} external_kernel_t;

typedef struct 
{
    unsigned long present :1;
    unsigned long read_write :1;
    unsigned long user_supervisor :1;
    unsigned long write_through :1;
    unsigned long cache_disable :1;
    unsigned long accessed :1;
    unsigned long dirty :1;
    unsigned long page_size :1;
    unsigned long global :1;
    unsigned long available :3;
    unsigned long frame :20;
} page_directory_t;

typedef struct 
{
    unsigned long present :1;
    unsigned long read_write :1;
    unsigned long user_supervisor :1;
    unsigned long write_through :1;
    unsigned long cache_disable :1;
    unsigned long accessed :1;
    unsigned long dirty :1;
    unsigned long page_size :1;
    unsigned long global :1;
    unsigned long available :3;
    unsigned long frame :20;
} page_table_t;

typedef struct
{
	unsigned long flags;
	unsigned long memory_low;
	unsigned long memory_high;
	unsigned long boot_device;
	unsigned long cmdline;
	unsigned long module_count;
	unsigned long module_address;
} multiboot_info_t;

typedef struct
{
	unsigned long module_start;
	unsigned long module_end;
	unsigned long cmdline;
	unsigned long padding;
} multiboot_module_t;

typedef struct
{
	unsigned long clb;
	unsigned long clbu;
	unsigned long fb;
	unsigned long fbu;
	unsigned long is;
	unsigned long ie;
	unsigned long cmd;
	unsigned long rsv0;
	unsigned long tfd;
	unsigned long sig;
	unsigned long ssts;
	unsigned long sctl;
	unsigned long serr;
	unsigned long sact;
	unsigned long ci;
	unsigned long sntf;
	unsigned long fbs;
	unsigned long rsv1[11];
	unsigned long vendor[4];
} ahci_hba_port_t;

typedef struct
{
	unsigned long cap;
	unsigned long ghc;
	unsigned long is;
	unsigned long pi;
	unsigned long vs;
	unsigned long ccc_ctl;
	unsigned long ccc_pts;
	unsigned long em_loc;
	unsigned long em_ctl;
	unsigned long cap2;
	unsigned long bohc;
	unsigned char  rsv[0xA0-0x2C];
	unsigned char  vendor[0x100-0xA0];
	ahci_hba_port_t	ports[32];
} ahci_hba_memory_t;

typedef struct
{
	unsigned long dba;
	unsigned long dbau;
	unsigned long rsv0;
	unsigned long dbc:22;
	unsigned long rsv1:9;
	unsigned long i:1;
} ahci_hba_prdt_entry_t;

typedef struct
{
	unsigned char cfis[64];
	unsigned char acmd[16];
	unsigned char rsv[48];
	ahci_hba_prdt_entry_t prdt_entry[1];
} ahci_hba_cmd_tbl_t;

typedef struct
{
	unsigned char  cfl:5;
	unsigned char  a:1;
	unsigned char  w:1;
	unsigned char  p:1;
	unsigned char  r:1;
	unsigned char  b:1;
	unsigned char  c:1;
	unsigned char  rsv0:1;
	unsigned char  pmp:4;
	unsigned short prdtl;
	volatile unsigned long prdbc;
	unsigned long ctba;
	unsigned long ctbau;
	unsigned long rsv1[4];
} ahci_hba_cmd_header_t;

typedef struct
{
	unsigned char fis_type;
	unsigned char pmport:4;
	unsigned char rsv0:3;
	unsigned char c:1;
	unsigned char command;
	unsigned char featurel;
	unsigned char lba0;
	unsigned char lba1;
	unsigned char lba2;
	unsigned char device;
	unsigned char lba3;
	unsigned char lba4;
	unsigned char lba5;
	unsigned char featureh;
	unsigned char countl;
	unsigned char counth;
	unsigned char icc;
	unsigned char control;
	unsigned char rsv1[4];
} ahci_fis_reg_h2d_t;

typedef struct
{
	unsigned long type;
} ahci_port_t;

typedef struct
{
	char signature[4];
	unsigned char version;
	unsigned char count;
	ahci_port_t port[32];
	unsigned char list_count;
	unsigned char list[32];
} ahci_t;

typedef struct
{
	unsigned long address_ptr;
	unsigned long size;
	unsigned long entry_count;
	unsigned long max_entries;
} memory_map_info_t;

typedef struct 
{
	unsigned long base_lo;
	unsigned long base_hi;
	unsigned long length_lo;
	unsigned long length_hi;
	unsigned long type;
	unsigned long acpi;
} memory_map_entry_t;

typedef struct
{
	char signature[4];
	unsigned char version;
	unsigned short count;
	memory_map_entry_t entry[384];
} memory_map_t;

typedef struct 
{
    unsigned long command;
    unsigned long address;
    unsigned long length;
} adma_cmd_packet_t;

typedef struct 
{
    unsigned short address;
    unsigned char page;
    unsigned char count;
} dma_channel_t;

typedef struct 
{
    unsigned long address;
    unsigned short byte_count;
    unsigned short reserved;
} prd_t;

#pragma pack (pop)

#define KERNEL_STACK_SIZE 2048

#define PAGE_SIZE 0x1000

#define PAGE_PRESENT             0x01
#define PAGE_READWRITE           0x02
#define PAGE_USER                0x04

#define HEAP_START 0xC00000
#define HEAP_END   0x8000000 // 0x1800000
#define ALLOC_SIZE_HEADER  8

extern void kernel_end_address();

#define SYSTEM_EXTERNAL_KERNEL                ((((unsigned long)kernel_end_address) & 0xFFFF000) + 0x1000)

extern unsigned char *root_sector;

extern unsigned char *mbr_sector;
extern unsigned char *boot_sector;
extern unsigned char *disk_address_packet;

extern unsigned char *vga_memory;
extern unsigned char *video_memory;
extern unsigned char *vesa_info_buffer;
extern unsigned char *vesa_mode_buffer;

extern unsigned char *fat32_fsinfo;

extern unsigned char *loader_kernel;

extern unsigned char *external_kernel;

extern unsigned char *kernel;
extern unsigned char *system_variables;
extern unsigned char *system_variables_info;

extern unsigned char *system_variables_enum;
extern unsigned char *system_variables_enum_info;

extern unsigned char *system_info;

extern unsigned char *system_pci;

extern unsigned char *system_errno;

extern unsigned char *system_memory_map_info;
extern unsigned char *system_memory_map;

extern unsigned char *ahci_ptr;

extern unsigned long page_directory[1024] __attribute__((aligned(4096)));
extern unsigned long page_table[1024][1024] __attribute__((aligned(4096)));

extern void * _heap_start;
extern void * _heap_end;
extern void * _heap_current;
extern void * _heap_prev;

extern unsigned long _heap_prev_position;
extern unsigned long _heap_last_size_alloc;
extern unsigned long _heap_last_position;
extern unsigned long _heap_position;
extern unsigned long _heap_size;
extern unsigned long _heap_last_size;

extern unsigned long _heap_alloc_last_clean_start;
extern unsigned long _heap_alloc_last_clean_end;

extern unsigned long _heap_alloc_available;

extern unsigned long malloc_count;
extern unsigned long free_count;

extern unsigned long malloc_history[65536];
extern unsigned long free_history[65536];

extern dap_t* dap;
extern mbr_t* mbr;

extern sys_vars_t *sys_vars;
extern sys_vars_info_t *sys_vars_info;

extern sys_enum_t *sys_enum;
extern sys_enum_info_t *sys_enum_info;

extern system_info_t *info;

extern multiboot_info_t *multiboot_info;

extern int active_partition;
extern partition_entry_t* partition;
extern partition_entry_t *main_partition;

extern fat_t *fat;
extern fat32_fsinfo_t *fsinfo;

extern file_entry_t file_dir_sector[16];

extern char volume_id[11];

extern system_pci_t *sys_pci;

extern memory_map_t *memory_map;
extern memory_map_info_t *memory_map_info;

extern unsigned short storage_drive_controller;

extern external_kernel_t extern_kernel;

extern gdt_t gdt;
extern idt_t idt;
extern tss_t tss;

extern vesa_info_t *vesa_info;
extern vesa_mode_t *vesa_mode;

extern unsigned char textattr;

extern unsigned long kernel_stack;

extern int mbr_loaded;
extern int mbr_active;

extern unsigned char bootstrap[FAT32_BOOTSTRAP_SIZE];

extern pci_device_t pci_device[32];
extern unsigned char pci_count;

extern int pci_video_memory_found;
extern unsigned long pci_video_memory_address;

extern int sys_vars_loaded;
extern int sys_enum_loaded;
extern unsigned long placement_address;

extern void print_hex(unsigned short v);
extern unsigned char getdrivenumber();
extern unsigned char diskread(void *buffer, unsigned long seek);
extern void oserror();
extern unsigned long probememory(void);
extern void switchtousermode(void);
extern void switchtokernelmode(void);

extern void shell_code_exec(void *, int argc, char **argv);
extern void shell_code_exit(unsigned long);

extern void kernelmode_start(void);

extern void pic_set(void);
extern void pic_restore(void);

void copy_function_to_base(void *function_base, unsigned long function_size, unsigned long new_base);

unsigned long kmalloc_page();
unsigned long kmalloc_int(unsigned long sz, int a, unsigned long *pa);
unsigned long kmalloc_a(unsigned long sz);
unsigned long kmalloc(unsigned long sz);

void init_memory_map();
void loadmmap();
void loadmemmgr();
void printmemory();
unsigned long getavailablememorymap();
unsigned long getreservedmemorymap();
void loaddma(void);
void loadsets(void);

unsigned char get_video_vesa_mode(unsigned short mode);
unsigned char set_video_vesa_mode(unsigned short mode);
unsigned char get_video_mode(void);
void set_video_mode(unsigned short mode);
unsigned char get_video_vesa_info(void);
unsigned long get_video_vesa_buffer(unsigned short mode);
unsigned char has_video_vesa_framebuffer(unsigned short mode);
unsigned long get_vesa_pixel(int x, int y);
void set_vesa_pixel(int x, int y, unsigned long c);
unsigned long rgb(unsigned char r, unsigned char g, unsigned char b);

void printk(const char *msg, ...);

void *get_ptr(unsigned long offset);
unsigned long get_addr(void *ptr);

void kernelmode_preinit(void);
void kernelmode_init(void);

void presskey(void);
int chdir(const char *path);

void sputchar(char c);
void cputchar(unsigned char a, char c);
void cputs(unsigned char a, const char *s);
void cputch(unsigned char a, const char c);
void cprint(unsigned char a, const char *s);

void putchar(const char c);
void putch(const char c);
void puts(const char *s);
void print(const char *s);
void print_int(int n);

void clrscr(void);
void gotoxy(int x, int y);

unsigned char get_csi_color(unsigned char n, unsigned char c);
void get_sgr_color(unsigned char sgr_code, unsigned char csi_code);
void get_sgr_line_feed(unsigned char sgr_code, unsigned char csi_code);
void get_sgr_cursor(unsigned char sgr_code, unsigned char csi_code);

void tprint(const char *s);
void tprintl(const char *s, unsigned long l);

void strtrm(char *s1, char *s2);

void remap_mbr(void);

void *sbrk(size_t len);

unsigned char fileexists(const char *filename);
unsigned long getfilesize(const char *filename);
unsigned char getfiledata(const char *filename, unsigned char *data);

void int86(unsigned char int_no, registers16_t *regs_in, registers16_t *regs_out);
void int386(unsigned char int_no, registers32_t *regs_in, registers32_t *regs_out);

void call86(void *function);
void call386(void *function);

void page_fault(registers_t *registers);

void uuidv4(char *str);

void add_sys_var(const char* _sys_var_name_, const char* _sys_var_value_);

int has_sys_var(const char* _sys_var_);
char* get_sys_var(const char* _sys_var_);

#define vars_loaded                     (sys_vars_loaded == 1)
#define enum_loaded                     (sys_enum_loaded == 1)

#define has_var(_var)                   (has_sys_var(_var))
#define get_var(_var)                   (get_sys_var(_var))

#define has_not_enum(_enum)             ((((sys_enum->value[_enum]) == 0) ? 1 : 0) && (sys_enum_info->count > _enum) && (sys_enum_loaded == 1))
#define has_enum(_enum)                 ((((sys_enum->value[_enum]) != 0) ? 1 : 0) && (sys_enum_info->count > _enum) && (sys_enum_loaded == 1))
#define get_enum(_enum)                 (sys_enum->value[_enum])
#define total_enum                      (sys_enum_info->count)


unsigned int atoh(char *s);
long int atol(char *s);
int atoi(char *s);
char *itob(unsigned long num, unsigned long base);
char *itob64(unsigned long long num, unsigned long long base);
char * itoa( int value, char * str, int base );
void* memcpy(void *s1, const void *s2, size_t n);
void* memset(void *s, int c, size_t n);
char *strcpy(char *s1, const char *s2);
size_t strlen(const char *s);
char *strrev(const char *s);
int strcmp(const char *s1, const char *s2);
char *strcat(char *s1, const char *s2);
char* strncpy (char *s1, const char *s2, size_t n);
int strncmp(const char *s1, const char *s2, size_t n);
char *strupr(const char *s);
char *strlwr(const char *s);
char *strchr (const char *s, int c);
void strcatb(char* s1, char* s2);
void panic(unsigned long exception_code);
void enable_interrupt(void);
void disable_interrupt(void);
void sound(unsigned long freq);
void nosound();
void restart(void);
void shutdown_int(void);
void shutdown(void);


#define PRINT_MEMORY_MAP_BIOS_MMAP                     (1 << 0)
#define PRINT_MEMORY_MAP_SYSTEM_MMAP                   (1 << 1)
#define PRINT_MEMORY_MAP_ENTRY_COUNT                   (1 << 2)
#define PRINT_MEMORY_MAP_RAM_AVAILABLE                 (1 << 3)
#define PRINT_MEMORY_MAP_RAM_RESERVED                  (1 << 4)
#define PRINT_MEMORY_MAP_LIST_OFFSET                   (1 << 5)
#define PRINT_MEMORY_MAP_LIST_DETAIL                   (1 << 6)

void printmemorymap(unsigned long opts);

unsigned char inb(unsigned short port);
unsigned short inw(unsigned short port);
unsigned long inl(unsigned short port);
void outb(unsigned short port, unsigned char value);
void outw(unsigned short port, unsigned short value);
void outl(unsigned short port, unsigned long value);

#define DMA_CHANNELS 8

#define DMA_CHANNEL_1               1
#define DMA_CHANNEL_2               2
#define DMA_CHANNEL_3               3
#define DMA_CHANNEL_4               4
#define DMA_CHANNEL_5               5
#define DMA_CHANNEL_6               6
#define DMA_CHANNEL_7               7

#define DMA_PAGE_REGISTER    0x87
#define DMA_STATUS_REGISTER  0x8B
#define DMA_COMMAND_REGISTER 0x0B

#define DMA_CHANNEL_SOUND           DMA_CHANNEL_1
#define DMA_CHANNEL_FLOPPY          DMA_CHANNEL_2

#define ADMA_BASE_ADDRESS 0x20
#define ADMA_REGISTER_SIZE 1024

#define PRIMARY_ATA_BUS          0x1F0
#define SECONDARY_ATA_BUS        0x170

#define DMA_COMMAND_BYTE_OFFSET      0x0
#define DMA_STATUS_BYTE_OFFSET       0x2
#define DMA_PRDT_ADDRESS_OFFSET      0x4

#define DMA_ATA_COMMAND_REGISTER        0x00
#define DMA_ATA_STATUS_REGISTER         0x02
#define DMA_ATA_PRDT_ADDRESS_REGISTER   0x04

extern dma_channel_t dma_channels[DMA_CHANNELS];

void loadisr(void);
void loadirq(void);
void loadidt(void);

void init_heap(void);

unsigned short get_ds(void);
unsigned long get_ebx(void);
unsigned long get_ecx(void);
unsigned long get_edx(void);
unsigned long get_eax(void);
unsigned long get_ebp(void);
unsigned long get_esp(void);
unsigned long get_edi(void);
unsigned long get_esi(void);

void createpageblank(void);
void setpagedirs(void);
void setpagetables(void);
void flushpage(unsigned long addr);
void loadpages(void);

extern char *exception_messages[];

extern void loadPageDirectory(unsigned long*);
extern void enablePaging();
extern void enablePSE();

extern unsigned long kernel_start;
extern unsigned long kernel_end;

typedef void (*isr_t)(registers_t *);
void register_interrupt_handler(unsigned char n, isr_t handler);

extern isr_t interrupt_handlers[256];

extern unsigned char kernelmode;
extern unsigned char usermode;
extern unsigned char enter_kernelmode;

extern unsigned char restart_init;
extern unsigned char shutdown_init;

extern void halt(void);

void reload_devices(void);

void settss_entries(void);

void msleep(unsigned int milliseconds);

unsigned long getrootdirsector(void);
unsigned char readsector(unsigned long sector, unsigned char *buffer);
unsigned long get_total_files_size(unsigned long sector);

unsigned char getdiskcount(void);

unsigned char bios_sector_read(int id, void *buffer, unsigned long sector);

void mouse_handler(registers_t *registers);

int init_mouse(void);
void getmouse(int *x, int *y, int *b);

#define MOUSE_LEFT    0x01
#define MOUSE_RIGHT   0x02
#define MOUSE_MIDDLE  0x04

#define PORT_KB_CMD            0x64
#define PORT_KB_DATA           0x60

#define MOUSE_CMD_BYTE         0xD4
#define MOUSE_CMD_READ_BYTE    0x20
#define MOUSE_CMD_WRITE_BYTE   0x60

#define ENABLE_IRQ12     0x02

#define MOUSE_RESET      0xFF
#define MOUSE_SET_REMOTE 0xF0
#define MOUSE_SET_STREAM 0xEA
#define MOUSE_ENABLE     0xF4
#define MOUSE_DISABLE    0xF5
#define MOUSE_SET_RATE   0xF3
#define MOUSE_GET_TYPE   0xF2

typedef struct 
{
    signed char  dx, dy, dw;
    unsigned char buttons;
    unsigned char packet[4];
    unsigned char phase;
    unsigned char packet_size;
    unsigned char ready;
} mouse_state_t;

int mouse_init(void);
void mouse_set_rate(unsigned char rate);
void mouse_input_handler(void);
void mouse_uninit(void);

signed char mouse_get_x(void);
signed char mouse_get_y(void);
signed char mouse_get_wheel(void);

unsigned char mouse_get_buttons(void);

signed char mouse_get_dx(void);
signed char mouse_get_dy(void);
signed char mouse_get_dw(void);

int mouse_get_type(void);

void irq12_handler(void);

unsigned char kbhit(unsigned char key);

#endif // __FSKRNL_H__
