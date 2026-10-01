// Fast System Kernel
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2024 DSP Interactive.

#ifndef __FSKRNL_H__
#define __FSKRNL_H__

#include "itypes.h"
#include "errno.h"
#include "fcntl.h"
#include "memory.h"
#include "fat.h"
#include "io.h"
#include "pcidev.h"
#include "storage.h"
#include "ata.h"
#include "ahci.h"
//#include "uhci.h"
//#include "ohci.h"
#include "ehci.h"
//#include "xhci.h"
#include "nvme.h"
//#include "scsi.h"

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
#define TSS_SEL  0x38 //(0x38|TSS_RING)
#define TSS_CODE_SEGMENT (CODE_SEGMENT|TSS_RING)
#define TSS_DATA_SEGMENT (DATA_SEGMENT|TSS_RING)
#define KERNEL_MODE_CODE_SEGMENT CODE_SEGMENT
#define KERNEL_MODE_DATA_SEGMENT DATA_SEGMENT
#define USER_MODE_CODE_SEGMENT 0x1B //(KERNEL_MODE_DATA_SEGMENT|TSS_CODE_SEGMENT)
#define USER_MODE_DATA_SEGMENT 0x23 //(KERNEL_MODE_DATA_SEGMENT|TSS_DATA_SEGMENT)

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

#define TASK_UNUSED           0
#define TASK_READY            1
#define TASK_RUNNING          2
#define TASK_SLEEP            3
#define TASK_BLOCK            4
#define TASK_TERMINATE        5

#define MAX_TASKS 65536

#define MAX_PRIORITY   7
#define PRIORITY_LEVELS (MAX_PRIORITY + 1)

#define PRIORITY_IDLE         0
#define PRIORITY_LOW          1
#define PRIORITY_NORMAL       3
#define PRIORITY_HIGH         5
#define PRIORITY_DRIVER       6
#define PRIORITY_CRITICAL     7

#define TASK_THREAD           (0<<1)
#define TASK_PROCESS          (1<<1)

#define PTHREAD_JOIN_SUCCESS       0
#define PTHREAD_JOIN_INVALID      -1
#define PTHREAD_JOIN_DEADLOCK     -2
#define PTHREAD_JOIN_ALREADY      -3
#define PTHREAD_JOIN_NO_JOINER    -4

#define WEXITSTATUS_NOT_FOUND     127

#define EFAULT 14

#define TASK_STACK_SIZE 2048

#define STACK_SIZE      4096
#define STACK_WORDS     STACK_SIZE
#define STACK_BYTES     (STACK_WORDS * sizeof(unsigned long))
#define CONTEXT_WORDS (sizeof(context_t) / sizeof(unsigned long))
#define INITIAL_STACK_GUARD_WORDS 1

#define TLS_GDT_INDEX       8
#define TLS_SELECTOR        (TLS_GDT_INDEX << 3)

#define TLS_SIZE            256
#define TLS_ALIGNMENT       16

#define TLS_MAGIC           0x544C5301UL   /* "TLS1" */

#define TLS_OFFSET_COUNTER     4
#define TLS_OFFSET_VALUE       8
#define TLS_OFFSET_POINTER     16

#define MAXKEYSHANDLERS 2


#define RAND_MAX 32767


#define MK_FP(seg, off) ((((unsigned long)(seg)) << 16) | (unsigned long)(off))
#define FP_SEG(fp) ((unsigned short)((unsigned long)(fp) >> 16))
#define FP_OFF(fp) ((unsigned short)((unsigned long)(fp) & 0xFFFFUL))
#define FP_LINEAR(fp) ((((unsigned long)FP_SEG(fp)) << 4) + (unsigned long)FP_OFF(fp))
#define FP_MAKE_LINEAR(seg, off) ((((unsigned long)(seg)) << 4) + (unsigned long)(off))

#define mk_fp(seg, off)   MK_FP(seg, off)
#define fp_seg(fp)        FP_SEG(fp)
#define fp_off(fp)        FP_OFF(fp)

#define nearptr(segment, offset) ((char*)((segment)*16UL+(offset)))

#define MAX_ENUM 128
#define MAX_VARS 128


#define STRINGIFY(x) #x

#define create_task(func) (createthread( \
        NULL, \
        STRINGIFY(func), \
    	0, \
    	0, \
        (void*)func, \
        NULL, \
        0, \
        NULL \
    ))


#define ELF_MAGIC0 0x7f
#define ELF_MAGIC1 'E'
#define ELF_MAGIC2 'L'
#define ELF_MAGIC3 'F'

#define EI_NIDENT 16

#define ELFCLASS32 1
#define ELFDATA2LSB 1

#define ET_EXEC 2
#define EM_386  3

#define PT_LOAD 1

#define TASK_KERNEL_STACK_SIZE 4096


typedef struct
{
	int argc;
	char argv[256][256];
} argparam_t;

typedef struct
{
	int argc;
	const char **argv;
} argparams_t;

//typedef void (*entry_fn)(void);
typedef int (*entry_fn)(void *param);

typedef void *(*thread_entry_t)(void *param);


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
	gdt_entry_t entry[16];
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
    unsigned long gs;
    unsigned long fs;
    unsigned long es;
    unsigned long ds;
    unsigned long edi;
    unsigned long esi;
    unsigned long ebp;
    unsigned long esp;
    unsigned long ebx;
    unsigned long edx;
    unsigned long ecx;
    unsigned long eax;
    unsigned long eip;
    unsigned long cs;
    unsigned long eflags;
    unsigned long useresp;
    unsigned long ss;
} context_t;

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
    unsigned long eip;
    unsigned long eflags;
} cpu_context_t;

typedef struct 
{
    unsigned long eax;
    unsigned long ebx;
    unsigned long ecx;
    unsigned long edx;
    unsigned long esi;
    unsigned long edi;
    unsigned long ebp;
    unsigned long ds;
    unsigned long es;
    unsigned long fs;
    unsigned long gs;
    unsigned long eip;
    unsigned long cs;
    unsigned long eflags;
    unsigned long esp;
} cpu_context_1_t;

typedef struct 
{
    unsigned long gs;
    unsigned long fs;
    unsigned long es;
    unsigned long ds;
    unsigned long edi;
    unsigned long esi;
    unsigned long ebp;
    unsigned long esp;
    unsigned long ebx;
    unsigned long edx;
    unsigned long ecx;
    unsigned long eax;
    unsigned long eip;
    unsigned long cs;
    unsigned long eflags;
} cpu_context_2_t;

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

typedef struct task
{
	char name[256];
	context_t context;
    context_t *esp;
    uint32_t *stack_top;
    uint32_t stack_size;    
    uint32_t *kernel_stack;
    uint32_t kernel_stack_size;
    uint32_t kernel_cs;
    uint32_t kernel_ds;
    uint32_t user_cs;
    uint32_t user_ds;
    struct task *parent;
    struct task *prev;
    struct task *next;
    int priority;
    int state;
    int quantum_left;   
    unsigned int wake_tick; 
    unsigned int thread_id;
    thread_entry_t entry;
    void *param;
    void *result;
    void *args;
    int joiner_id;
    int joined;    
    int pid;
    void *module;
    void *start_brk;
    void *brk;
    void *tls_area;
    unsigned long tls_size;
    uint32_t stack[STACK_SIZE];
} task_t;

typedef struct 
{
    unsigned char  e_ident[EI_NIDENT];
    unsigned short e_type;
    unsigned short e_machine;
    unsigned long e_version;
    unsigned long e_entry;
    unsigned long e_phoff;
    unsigned long e_shoff;
    unsigned long e_flags;
    unsigned short e_ehsize;
    unsigned short e_phentsize;
    unsigned short e_phnum;
    unsigned short e_shentsize;
    unsigned short e_shnum;
    unsigned short e_shstrndx;
} elf_ehdr_t;

typedef struct 
{
    unsigned long p_type;
    unsigned long p_offset;
    unsigned long p_vaddr;
    unsigned long p_paddr;
    unsigned long p_filesz;
    unsigned long p_memsz;
    unsigned long p_flags;
    unsigned long p_align;
} elf_phdr_t;

typedef struct 
{
    task_t *handle_instance;
    const char *name;
    int attributes;
    int stack_size;
    unsigned long *start_address;
    void *param;
    int flags;
    int *new_thread_id;
} createthread_args_t;

#pragma pack (pop)

#define KERNEL_STACK_SIZE 16384 //2048

#define PAGE_SIZE 0x1000
#define PAGE_MASK (~(PAGE_SIZE - 1))

#define PAGE_PRESENT             0x01
#define PAGE_READWRITE           0x02
#define PAGE_USER                0x04
#define PAGE_PAGESIZE            0x80

#define NUM_PAGES                (1024 * 1024)

#define TOTAL_FRAMES              NUM_PAGES

#define TOTAL_MEMORY_MB                 64
#define TOTAL_FRAMES_LIMIT_64_MB        ((TOTAL_MEMORY_MB * NUM_PAGES) / PAGE_SIZE)

#define PMM_BITMAP_SIZE    32768

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

extern sys_vars_t *sys_vars;
extern sys_vars_info_t *sys_vars_info;

extern sys_enum_t *sys_enum;
extern sys_enum_info_t *sys_enum_info;

extern system_info_t *info;

extern multiboot_info_t *multiboot_info;

extern char volume_id[11];

extern system_pci_t *sys_pci;

extern memory_map_t *memory_map;
extern memory_map_info_t *memory_map_info;

extern unsigned short storage_drive_controller;

extern external_kernel_t extern_kernel;

extern gdt_t gdt;
extern idt_t idt;
extern tss_t tss;

//extern task_t thread[MAX_TASKS];
extern task_t *thread;
extern int thread_count;
extern int current_thread;

extern int idle_thread_id;

extern int scheduler_initialized;

extern int process_count;
extern int current_process;

extern vesa_info_t *vesa_info;
extern vesa_mode_t *vesa_mode;

extern unsigned char textattr;

extern unsigned long kernel_stack;

extern int mbr_loaded;
extern int mbr_active;

extern pci_device_t pci_device[32];
extern unsigned char pci_count;
extern int pci_initialized;

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
extern void switchtomultitask(void);
extern void switchtousermode(void);
extern void switchtokernelmode(void);

extern void kernelmode_start(void);

extern void pic_set(void);
extern void pic_restore(void);

void copy_function_to_base(void *function_base, unsigned long function_size, unsigned long new_base);

unsigned long kmalloc_page();
unsigned long kmalloc_int(unsigned long sz, int a, unsigned long *pa);
unsigned long kmalloc_a(unsigned long sz);
unsigned long kmalloc(unsigned long sz);
void* kcalloc(size_t num, size_t size);

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
void cprintf(int color, const char *msg, ...);

void *get_ptr(unsigned long offset);
unsigned long get_addr(void *ptr);

void kernelmode_preinit(void);
void kernelmode_init(void);

void presskey(void);

int fschdir(const char *path);
int fsmkdir(const char *path);
int fsrmdir(const char *path);
int fsunlink(const char *path);

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

extern const char *exception_messages[];

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
void sleep(int seconds);

unsigned long getrootdirsector(void);
unsigned char readsector(unsigned long sector, unsigned char *buffer);
unsigned char writesector(unsigned long sector, const unsigned char *buffer);
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
unsigned char getch_stdin(void);

void switchcontext(unsigned long **old_sp, unsigned long *new_sp);
void task_switch(unsigned long *esp);
void task_restore(unsigned long *esp);

int createthread(task_t *handle_instance,
                 const char *name,
                 int attributes,
                 int stack_size,
                 unsigned long *start_address,
                 void *param,
                 int flags,
                 int *new_thread_id);

int pthread_join(int thread_id, void **result);
int pthread_join_handler(int thread_id, void **result);

task_t* findthread(const char *name);
task_t* getthreadbyid(int id);
task_t* getthreadbypid(int pid);
task_t* getthreadbypidnottid(int pid, int not_thread_id);

void task_ready(int task_id);
void task_yield(int task_id);
void task_block(int task_id);
void task_terminate(int task_id);

void set_task_priority(int task_id, int priority);
void set_task_state(int task_id, int state);

int task_is_runnable(const task_t *t);
int valid_task_id(int id);

int execv(const char *path, char *const argv[]);

void cmdnotfound(char *app, char *name);

void dump_hex(const void *data, size_t size);

void delay_us(unsigned long us);
void delay_ms(unsigned long ms);

void sys_exit(unsigned long exitcode);
int sys_fork(void);

extern void *pmm_alloc_page(void);
extern void pmm_free_page(void *page);
extern int vmm_map_page(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags);
extern uint32_t vmm_get_phys(uint32_t virt_addr);
extern void vmm_unmap_page(uint32_t virt_addr);


size_t strlen(const char *s);

char *strcpy(char *s1, const char *s2);

char *strrev(const char *s);

int strcmp(const char *s1, const char *s2);

char *strcat(char *s1, const char *s2);

char *strncpy(char *s1, const char *s2, size_t n);

int strncmp(const char *s1, const char *s2, size_t n);

char *strupr(const char *s);

char *strlwr(const char *s);

char *strchr(const char *s, int c);

void strcatb(char *s1, char *s2);


/* Memory functions */

void *memset(void *s, int c, size_t n);

void *memcpy(void *s1, const void *s2, size_t n);


/* String formatting / conversion */

void str_pad_left(const char *s1, char *s2, int padding);

unsigned int atoh(char *s);

long int atol(char *s);

int atoi(char *s);

char *itob(unsigned long num, unsigned long base);

char *itob64(unsigned long long num, unsigned long long base);

char *itoa_s(unsigned long num, unsigned long base);

char *itoa(int value, char *str, int base);

typedef int32_t time_t;

struct tm
{
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
};

time_t time(time_t *timer);
time_t mktime(struct tm *tm);

struct tm *gmtime(const time_t *timer);
struct tm *localtime(const time_t *timer);

struct tm *gmtime_r(const time_t *timer, struct tm *result);
struct tm *localtime_r(const time_t *timer, struct tm *result);

double difftime(time_t time1, time_t time0);

char *asctime(const struct tm *tm);
char *ctime(const time_t *timer);

unsigned long getfilesize(const char *filename);

int tls_create(task_t *t);
void tls_destroy(task_t *t);

void *tls_get(unsigned long offset);
int tls_set(unsigned long offset, const void *src, unsigned long size);

struct user_desc 
{
    uint32_t entry_number;
    uint32_t base_addr;
    uint32_t limit;
    uint32_t seg_32bit:1;
    uint32_t contents:2;
    uint32_t read_exec_only:1;
    uint32_t limit_in_pages:1;
    uint32_t seg_not_present:1;
    uint32_t useable:1;
    uint32_t lm:1;
};

int set_thread_area(struct user_desc *u_info);
int get_thread_area(struct user_desc *u_info);
int tls_load_task(task_t *t);


#endif // __FSKRNL_H__
