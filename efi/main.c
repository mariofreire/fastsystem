#include <stdarg.h>
#include <stdbool.h>

#include "efi.h"

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

#define PARTITION_GPT_PROTECTIVE 0xEE


#define MBR_BOOTSTRAP_SIZE 0x1BE
#define FAT32_BOOTSTRAP_SIZE 0x1A4
//#define FAT16_BOOTSTRAP_SIZE 0x1C0
#define FAT16_BOOTSTRAP_SIZE 0x1C

#define FAT_ENTRY_SIZE 32

#define FAT12_EOC_MARK 0x0FF8
#define FAT16_EOC_MARK 0xFFF8
#define FAT32_EOC_MARK 0x0FFFFFF8U

#define FAT16_CHAIN_MARK 0xFFF0
#define FAT16_CHAIN_MASK 0xFFFF
#define FAT32_CHAIN_MARK 0x0FFFFFF0U
#define FAT32_CHAIN_MASK 0x0FFFFFFFU

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

/*
#ifdef	WIN32
#define PATHSEPARATOR(c) ((c) == '\\' || (c) == '/')
#else	//WIN32
#define PATHSEPARATOR(c) ((c) == '/')
#endif	//WIN32
*/

#define PATHSEPARATOR(c) (((c) == '/') || ((c) == '\\'))

#define UCHAR8A(value) ((BYTE)(value))
#define UCHAR8B(value) ((BYTE)((value)>> 8))
#define UCHAR8C(value) ((BYTE)((value)>>16))
#define UCHAR8D(value) ((BYTE)((value)>>24))
#define UINT16(a,b) ((DWORD)((BYTE)(a)|((BYTE)(b)<<8)))
#define UINT32(a,b,c,d) ((DWORD)((BYTE)(a)|((BYTE)(b)<<8)|((BYTE)(c)<<16)|((BYTE)(d)<<24)))
#define USHORT16(a,b) ((DWORD)(((DWORD)(a)<<16)|((WORD)(b))))


#define TEXT_COLS                             80
#define TEXT_ROWS                             25


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

#define xyoffset(_x,_y,_w) ((_w*_y) + _x)
#define xyoffset16(_x,_y,_w)  ((_w*_y) + (_x * 2))
#define xyoffset24(_x,_y,_w)  ((_w*_y) + (_x * 3))
#define xyoffset32(_x,_y,_w)  ((_w*_y) + (_x * 4))

#define textoffset(_x,_y) (2 * xyoffset(_x,_y,TEXT_COLS))
#define textoffsety(_offset) (_offset/(2*TEXT_COLS))
#define textoffsetx(_offset) ((_offset-(textoffsety(_offset)*2*TEXT_COLS))/2)


#define isspace(c)                      (c == ' ')
#define isnumber(c)                      ((c >= '0') && (c <= '9'))
#define isalpha(c)                      (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')))
#define isalnum(c)                      (isalpha(c) || isnumber(c))
#define issign(c)                      ((c == '-') || (c == '+') || (c == '*') || (c == '/') || (c == '='))
#define isspecial(c)                      ((c == '\\') || (c == ';') || (c == '\'') || (c == '[') || (c == ']') || (c == ',') || (c == '.'))


EFI_HANDLE Handle;
EFI_SYSTEM_TABLE *ST;


#ifndef __SIZE_TYPE__
#define __SIZE_TYPE__ UINTN
#endif

#pragma pack (push, 1)

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

#pragma pack (pop)

extern pci_device_t pci_device[32];
extern unsigned char pci_count;

extern unsigned long pci_config_address(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset);
extern unsigned char pci_read_byte(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset);
extern unsigned short pci_read_word(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset);
extern unsigned long pci_read_long(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset);
extern unsigned char pci_scan_device(unsigned char bus, unsigned char slot, unsigned char func, unsigned char index);
extern void pci_scan_bus(unsigned char bus);
extern void pci_scan(void);
extern void loadpci(void);
extern int init_pci(void);

#define AHCI_ADDRESS 0x841500

#pragma pack (push, 1)

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

#pragma pack (pop)

extern int init_ahci(void);
extern int init_ahci_ports();
extern void detectahci(void);
extern int check_ahci_ports(void);
extern int check_ahci_type(ahci_hba_port_t *port);
extern void ahci_start_cmd(ahci_hba_port_t *port);
extern void ahci_stop_cmd(ahci_hba_port_t *port);
extern unsigned long sata_read(int id, void *buffer, unsigned long sector, unsigned long count);
extern unsigned char get_sata_ident(ahci_hba_port_t *port, void *buffer);

unsigned long ahci_hba_address = 0;
ahci_hba_port_t *ahci_hba_port;
ahci_hba_memory_t *ahci_hba;
ahci_port_t ahci_port[32];
unsigned char ahci_list_count = 0;
unsigned char ahci_list[32];


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

unsigned short storage_drive_controller = STORAGE_CONTROLLER_NONE;

char *bootfilename = "bootia32.efi";

#pragma pack (push, 1)

typedef struct
{
	char signature[8];
	unsigned long revision;
	unsigned long header_size;
	unsigned long crc32_header;
	unsigned long reserved;
	unsigned long long my_lba;
	unsigned long long alternate_lba;
	unsigned long long first_usable_lba;
	unsigned long long last_usable_lba;
	unsigned char disk_guid[16];
	unsigned long long partition_entry_lba;
	unsigned long num_partition_entries;
	unsigned long partition_entry_size;
	unsigned long crc32_partitions;
} gpt_header_t;

typedef struct
{
	unsigned char type_guid[16];
	unsigned char unique_guid[16];
	unsigned long long lba_start;
	unsigned long long lba_end;
	unsigned long long attributes;
	unsigned short name[36];
} gpt_entry_t;

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
  char name1[11];
  unsigned char attribute;
  char name2[20];
} file_lfn_entry_t;

typedef struct
{
    char path[256];
} path_t;

typedef struct
{
path_t path[32];
int pathcount;
} path_sub_t;

#pragma pack (pop)

mbr_t* mbr;

int active_partition=-1;
partition_entry_t* partition;
partition_entry_t *main_partition;

fat_t *fat;

file_entry_t file_dir_sector[16];

int mbr_loaded=0;
int esp_loaded=0;

int esp_found = 0;

gpt_entry_t* gpt_main_partition;

void remap_mbr(void);
void remap_boot(void);

int has_gpt=0;
int has_efi_support=1;

int fat_loaded=0;

gpt_header_t *gpt_header;
gpt_entry_t *gpt_partition;

unsigned long gpt_partition_count = 0;

unsigned char bootstrap[FAT32_BOOTSTRAP_SIZE];

unsigned char *root_sector = (unsigned char *)0x9000;
//unsigned char *root_dir_sector = (unsigned char*)0xa000;

unsigned char *mbr_sector = (unsigned char *)0x8400;
unsigned char *boot_sector = (unsigned char *)0x8600;
unsigned char *disk_address_packet = (unsigned char *)0x8200;
unsigned char *gpt_header_ptr = (unsigned char *)0x870000;
unsigned char *gpt_entry_ptr = (unsigned char *)0x870200;

const uint8_t efi_system_guid[16] = {
    0x28, 0x73, 0x2A, 0xC1, 0x1F, 0xF8, 0xD2, 0x11,
    0xBA, 0x4B, 0x00, 0xA0, 0xC9, 0x3E, 0xC9, 0x3B
};

unsigned char loadmbr(void);
unsigned char loadgpt(void);
bool uninitimagedisk(void);
bool has_partition_active(void);
char *getshortfilename(char *filename11);
void strfilenamedot8e3s11(char *source_filename, char *destination_filename);
uint32_t getfilefirstcluster(file_entry_t entry);
uint32_t getrootlbaaddress(void);

void guidtostring(const unsigned char *guid, char *s, unsigned char stdfmt);

unsigned long root_sector_start;

int guid_validate(const unsigned char *guid);
int uuidv4_validate(const char *str);

unsigned long int rand_next = 1;

void printk(const char *msg, ...);
void printf(const char *msg, ...);
int sprintf(char *s, const char *fmt, ...);

char *basename(char *path);

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

void halt(void)
{
	__asm__ ("hlt");
	while(1);
}

void enable_interrupt(void)
{
	__asm__ ("sti");
}

void disable_interrupt(void)
{
	__asm__ ("cli");
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

void msleep(unsigned int milliseconds)
{
	unsigned long ms = milliseconds * 1000;
	unsigned long timertick = 0;
	while (timertick < ms)
	{
		timertick++;
	}
}

int getcursor(void)
{
	unsigned short offset;
	int r;
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
	offset = textoffset(x,y) / 2;
    outb(0x3d4, 14);
    outb(0x3d5, (offset >> 8));
    outb(0x3d4, 15);
    outb(0x3d5, offset);
}

void setcursoroffset(unsigned short offset)
{
    outb(0x3d4, 14);
    outb(0x3d5, (offset >> 8));
    outb(0x3d4, 15);
    outb(0x3d5, offset);
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
	restartcode = 0x02;
	printk("System Halted.\n");
	disable_interrupt();
	while (restartcode & 0x02)
		restartcode = inb(0x64);
	outb(0x64, 0xFE);
	halt();
}

void shutdown(void)
{
	printk("System Halted.\n");
	disable_interrupt();
	outw(0xB004, 0x2000);
	outw(0x604,  0x2000);
	outw(0x4004, 0x3400);
	outw(0x600,  0x34);
	halt();
}

char toupper(char c) 
{
    if (c >= 'a' && c <= 'z') 
    {
        return c - 32;
    }
    return c;
}

int tolower(int c) 
{
    if (c >= 'A' && c <= 'Z') 
    {
        return c + 32;
    }
    return c;
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

int memcmp(const void *ptr1, const void *ptr2, size_t num) 
{
    const unsigned char *p1 = (const unsigned char *)ptr1;
    const unsigned char *p2 = (const unsigned char *)ptr2;
    while (num-- > 0) 
	{
        if (*p1 != *p2) 
		{
            return (*p1 < *p2) ? -1 : 1;
        }
        p1++;
        p2++;
    }
    return 0;
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

/*
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
*/

char *strupr(char *str)
{
  unsigned char *p = (unsigned char *)str;
  while (*p) {
     *p = toupper((unsigned char)*p);
      p++;
  }
  return str;
}

char *strlwr(char *str)
{
  unsigned char *p = (unsigned char *)str;
  while (*p) {
     *p = tolower((unsigned char)*p);
      p++;
  }
  return str;
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

char *strrchr(const char *s, int c)
{
    char *last = NULL;
    
    while (*s)
    {
        if (*s == (char)c)
            last = (char *)s;
        s++;
    }
    
    if (c == '\0')
    {
    	return (char *)s;
    }
    
    return last;
}


float itof(int i)
{
 float f=0;
 char c[4];

 c[0] = i >> 24;
 c[1] = i >> 16;
 c[2] = i >> 8;
 c[3] = i;

 f = ((float*)c)[0];

 return f;
}

int ftoi(float x)
{
    int i=0;
    unsigned char c[4];
    *(float*)c = x;

    i = c[3];
    i |= c[2] << 8;
    i |= c[1] << 16;
    i |= c[0] << 24;

    return i;
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

int guid_reverse(unsigned char *guid)
{
    unsigned char tmp;
    unsigned char i;
    if (guid == NULL) return 0;
    for (i = 0; i < 16 / 2; i++)
    {
        tmp = guid[i];
        guid[i] = guid[16 - 1 - i];
        guid[16 - 1 - i] = tmp;
    }
    return 1;
}

int guid_swap_endianness(unsigned char *guid)
{
    unsigned char tmp;
    if (guid == NULL) return 0;
    tmp = guid[0]; guid[0] = guid[3]; guid[3] = tmp;
    tmp = guid[1]; guid[1] = guid[2]; guid[2] = tmp;
    tmp = guid[4]; guid[4] = guid[5]; guid[5] = tmp;
    tmp = guid[6]; guid[6] = guid[7]; guid[7] = tmp;
    return 1;
}

int guid_validate(const unsigned char *guid)
{
	unsigned char i;
	unsigned char z = 1;
	unsigned char ver;
	unsigned char var;
	if (guid == NULL) return 0;
	for (i = 0; i < 16; i++)
	{
    	if (guid[i] != 0)
    	{
        	z = 0;
        	break;
    	}
	}
	if (z) return 0;
	ver = (guid[6] >> 4) & 0x0F;
	if (ver < 1 || ver > 5) return 0;
	var = (guid[8] >> 6) & 0x03;
	if (var != 2) return 0;
	return 1;	
}

void guidtostring(const unsigned char *guid, char *s, unsigned char stdfmt)
{
	char buffer[64];
	memset(buffer, 0, 64);	
	if (guid == NULL) return;
	if (stdfmt == 0)
	{
		sprintf(buffer, 
		"%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X",
		guid[0],guid[1],guid[2],guid[3],guid[4],guid[5],guid[6],guid[7],guid[8],guid[9],
		guid[10],guid[11],guid[12],guid[13],guid[14],guid[15]);
	}
	else
	{
		sprintf(buffer, 
		"%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
		guid[0],guid[1],guid[2],guid[3],guid[4],guid[5],guid[6],guid[7],guid[8],guid[9],
		guid[10],guid[11],guid[12],guid[13],guid[14],guid[15]);		
	}
	strcpy(s, buffer);
}

const char *getfilename(const char *path)
{
    const char *slash1 = strrchr(path, '/');
    const char *slash2 = strrchr(path, '\\');

    if (slash1 && slash2)
        return (slash1 > slash2) ? slash1 + 1 : slash2 + 1;
    else if (slash1)
        return slash1 + 1;
    else if (slash2)
        return slash2 + 1;
    else
        return path;
}

char *basename(char *path)
{
	if (path == NULL) return NULL;
	char *s = (char*)getfilename((char*)path);
	return s;
}

void getfilebase(const char *path, char *base)
{
    char temp[256];
    strcpy(temp, path);
    char *name = basename(temp);
    char *dot = strrchr(name, '.');
    if (dot)
        *dot = '\0';
    strcpy(base, name);
}

char *extractfilebase(char *path)
{
	if (path == NULL) return NULL;
	char *base;
    char temp[256];
    strcpy(temp, path);
	getfilebase(path, temp);
	base = temp;
	return base;
}

char *dirname(char *path)
{
    char *last_sep;
    char *end;
    if (path == NULL || *path == '\0')
        return (char*)".";
    if (strcmp(path, ".") == 0 || strcmp(path, "..") == 0)
        return (char*)".";
    end = path + strlen(path) - 1;
    while (end > path && PATHSEPARATOR(*end))
        *end-- = '\0';
    last_sep = strrchr(path, '/');
    char *last_sep2 = strrchr(path, '\\');
    if (last_sep2 && (!last_sep || last_sep2 > last_sep))
        last_sep = last_sep2;
    if (last_sep == NULL)
        return (char*)".";
    if (last_sep == path)
    {
        path[1] = '\0';
        return path;
    }
    *last_sep = '\0';
    return path;
}

char *strdup(const char *s)
{
    if (s == NULL)
        return NULL;
    size_t len = strlen(s) + 1;
    char *copy = (char *)malloc(len);
    if (copy == NULL)
        return NULL;
    for (size_t i = 0; i < len; i++)
        copy[i] = s[i];
    return copy;
}

VOID InitializeLib (IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
   Handle = ImageHandle;
   ST = SystemTable;
   (void)Handle;
}

EFI_STATUS WaitForKeyStroke(OUT EFI_INPUT_KEY* Key) 
{
    EFI_STATUS Status;
    UINTN Index;
    if (!Handle) return 0;
    if (!ST) return 0;
    if (Key == NULL) 
    {
        return EFI_INVALID_PARAMETER;
    }
    while (1) 
    {
        Status = ST->ConIn->ReadKeyStroke(ST->ConIn, Key);
        if (!EFI_ERROR(Status)) 
        {
            break;
        }
        if (Status == EFI_NOT_READY) 
        {
            ST->BootServices->WaitForEvent(1, &ST->ConIn->WaitForKey, &Index);
        } else {
            return Status;
        }
    }
    return EFI_SUCCESS;
}

UINTN Print (IN CHAR16 *s)
{
    if (!Handle) return 0;
    if (!ST) return 0;
    EFI_STATUS back = ST->ConOut->OutputString (ST->ConOut, s);
    return back;
}


VOID print(IN CHAR8 *s)
{
    CHAR16 c[2];
    while (*s != 0)
    {
         if (*s == '\n')
         {
           c[0] = '\r';
           c[1] = 0;
           Print(c);
         }
         c[0] = *s++;
         c[1] = 0;
         if (((c[0] >= 0x20) && (c[0] < 0x7F)) || 
		    ((c[0] == '\n') || (c[0] == '\t'))) Print(c);
    }
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



/*
int pvsnprintf(char* str, const char* format, va_list ap)
{
	char buf[64];
	char *s;
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
  return pvsnprintf (str, format, arglist);
}
*/

void printk(const char *msg, ...)
{
	int slen = strlen(msg)*4+1;
	char msgBuf[1024];
	va_list va_alist;

	if (!msg) return;

	msgBuf[slen - 1] = '\0';
	msgBuf[1024 - 1] = '\0';
	va_start(va_alist, msg);
	vsprintf(msgBuf, msg, va_alist);
	va_end(va_alist);

	print(msgBuf);
}

void printf(const char *msg, ...)
{
	int slen = strlen(msg)*4+1;
	char msgBuf[1024];
	va_list va_alist;

	if (!msg) return;

	msgBuf[slen - 1] = '\0';
	msgBuf[1024 - 1] = '\0';
	va_start(va_alist, msg);
	vsprintf(msgBuf, msg, va_alist);
	va_end(va_alist);

	print(msgBuf);
}

int sprintf(char *s, const char *fmt, ...)
{
	int slen = strlen(fmt)*4+1;
	char sBuf[1024];
	va_list va_alist;

	if (!fmt) return 0;

	sBuf[slen - 1] = '\0';
	sBuf[1024 - 1] = '\0';
	va_start(va_alist, fmt);
	vsprintf(sBuf, fmt, va_alist);
	va_end(va_alist);

	strcpy(s, sBuf);
	return slen;
}

void panic(unsigned long exception_code)
{
	printk("System Halted: Panic at %s: KernelException(0x%x).\n", bootfilename, exception_code);
	// printk("Error: Cann't open a operating system.\n");
	halt();
}

pci_device_t pci_device[32];
unsigned char pci_count = 0;

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
	pci_count = 0;
	for(i=0;i<32;i++)
	{
		memset(&pci_device[i].pci, 0, sizeof(pci_t));
		pci_device[i].pci.vendor = 0xFFFF;
	}
	pci_scan();
}

int init_ahci(void)
{
	int i;
	unsigned long portcnt = 0, port_ok = 0;
	if (pci_count > 0)
	{
		for(i=0;i<pci_count;i++) 
		{
			if (pci_device[i].pci.vendor != 0xFFFF)
			{
				if ((pci_device[i].pci.class == 0x01) && (pci_device[i].pci.subclass == 0x06))
				{
					if (pci_device[i].pci.bar[5] != 0)
					{						
						ahci_hba_address = pci_device[i].pci.bar[5];
						ahci_hba = (ahci_hba_memory_t*)ahci_hba_address;						
						portcnt = check_ahci_ports();						
						ahci_list_count = portcnt;	
						port_ok = init_ahci_ports();
						return port_ok;
					}						
				}
			}
		}
	}
	return 0;
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

int check_ahci_ports(void)
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
				ahci_list[portcount] = i;
				portcount++;
				ahci_list_count = i;
				ahci_port[i].type = dt;
			}
			else if (dt == 4)
			{
				ahci_list[portcount] = i;
				portcount++;
				ahci_list_count = i;
				ahci_port[i].type = dt;
			}
			else if (dt == 2)
			{
				ahci_list[portcount] = i;
				portcount++;
				ahci_list_count = i;
				ahci_port[i].type = dt;
			}
			else if (dt == 3)
			{
				ahci_list[portcount] = i;
				portcount++;
				ahci_list_count = i;
				ahci_port[i].type = dt;
			}
			else
			{
				ahci_port[i].type = dt;				
			}
		}
		pi >>= 1;
		i++;
	}
	return portcount;
}

extern void msleep(unsigned int milliseconds);

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
	int i;
	ahci_stop_cmd(port);
	port->clb = 0x400000 + (portno<<10);
	port->clbu = 0;
	memset((void*)(port->clb), 0, 1024);
	port->fb = 0x400000 + (32<<10) + (portno<<8);
	port->fbu = 0;
	memset((void*)(port->fb), 0, 256);
	ahci_hba_cmd_header_t *cmdheader = (ahci_hba_cmd_header_t*)(port->clb);
	for (i=0; i<32; i++)
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
			int dt = check_ahci_type(&ahci_hba->ports[i]);
			if (dt == 1)
			{
				ahci_port[i].type = dt;
				ahci_port_rebase(&ahci_hba->ports[i], i);
				if (f == 0)
				{
					ahci_hba_port = &ahci_hba->ports[i];
					f = 1;
				}
				r = 1;
			}
			else if (dt == 4)
			{
				ahci_port[i].type = dt;
				ahci_port_rebase(&ahci_hba->ports[i], i);
				r = 1;
			}
			else
			{
				ahci_port[i].type = dt;				
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
	if (ahci_port == NULL) return 0;
	if (ahci_list == NULL) return 0;
	int index = ahci_list[id];
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
			int dt = ahci_port[i].type;
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

unsigned long sectortobytes(unsigned long sector)
{
	return (sector * SECTORSIZE);
}

unsigned long bytestosector(unsigned long bytes)
{
	return (bytes / SECTORSIZE);
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

unsigned char readsectors(unsigned long sector, unsigned long sectors, unsigned char *buffer)
{
	int i=0;
	int result = 0;
	int r = 0;
	while (i < sectors)
	{
		r = readsector(sector+i, buffer+sectortobytes(i));
		if (r == 0)
		{
			result = 0;
			return result;
		}
		else
		{			
			result = 1;
		}
		i++;
	}
	return result;
}

int is_print(int c)
{
    return c >= 0x20 && c <= 0x7E;
}

void dump_hex_address(unsigned long offset, const void *data, size_t size) {
    const unsigned char *buffer = (const unsigned char *)data;
    int i, j;

    for (i = 0; i < size; i += 16) {
        printk("%08X: ", (unsigned int)offset+i);
        for (j = 0; j < 16; j++) {
            if (i + j < size)
                printk("%02X ", buffer[i + j]);
            else
                printk("   ");	
        }		
		printk(" ");			
		for (int j = 0; j < 16; j++) {
			if (i + j < size)
				printk("%c", is_print(buffer[i + j]) ? buffer[i + j] : '.');
			else
				printk(" ");
		}
        printk("\n");
    }
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
		if (has_efi_support)
		{
			if (mbr->partition[0].type == PARTITION_GPT_PROTECTIVE)
			{
				has_gpt = 1;
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

bool has_esp_partition(void)
{
	if (has_efi_support == 0) return false;
	if (has_gpt == 0)
	{
		return false;
	}
	else
	{
		if (has_gpt == 1)
		{
			if (esp_found == 1)
			{
				if (esp_loaded == 1)
				{
					return true;
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
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
		if (has_gpt == 0)
		{
			panic((unsigned long)partition);
		}
	}
}

unsigned char* getgptpartitiontypeguid(int index)
{
	if (index < 0) return NULL;
	if (index >= gpt_partition_count) return NULL;
	return gpt_partition[index].type_guid;
		
}
unsigned char is_null_guid(unsigned char *guid)
{
	int result = 1;
	int i;
	for(i=0;i<16;i++)
	{
		if (guid[i] != 0) result = 0;
	}	
	return result;
}


unsigned char has_gpt_partition(int index)
{
	int result = 0;
	if (!is_null_guid(gpt_partition[index].type_guid)) result = 1;
	return result;
		
}

unsigned char loadgpt(void)
{
	int i, j;
	if (mbr_loaded == 0) return 0;
	if (has_efi_support == 0) return 0;
	if (has_gpt == 0) return 0;	
	if (!readsector(1, gpt_header_ptr)) return 0;
	gpt_header = (gpt_header_t*)gpt_header_ptr;	
	if (memcmp(gpt_header->signature, "EFI PART", 8) != 0) return 0;
	unsigned long long entry_lba = gpt_header->partition_entry_lba;	
	unsigned long entry_count = gpt_header->num_partition_entries;
	unsigned long entries_size = entry_count*gpt_header->partition_entry_size;
	unsigned long entries_sectors = entries_size/SECTORSIZE;
	for(i=0;i<entries_sectors;i++)
	{
		if (!readsector(entry_lba + i, gpt_entry_ptr+(i * SECTORSIZE))) return 0;
	}
	gpt_partition = (gpt_entry_t*)&gpt_entry_ptr[0];
    for(j=0;j<entry_count;j++)
	{
		if (memcmp(gpt_partition[j].type_guid, efi_system_guid, 16) == 0) 
		{
			gpt_main_partition = (gpt_entry_t*)(&gpt_partition[j]);
			esp_found = 1;
			esp_loaded = 1;
			break;
		}
	}
	gpt_partition_count = entry_count;
	if (esp_found == 0) return 0;
	return 1;
}

unsigned long getgptpartitioncount(void)
{
	return gpt_partition_count;
}

unsigned char isfat16type(void)
{
	unsigned char has_fat16;
	if (has_esp_partition())
	{
		if (fat_loaded == 1)
		{
			if (strncmp(fat->bpb.bpb2.fat16.type, "FAT16   ", 8) == 0)
			{
				has_fat16 = 1;
			}
		}
		else
		{
			has_fat16 = 0;
		}
	}
	else
	{
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
	}
	return has_fat16;
}

unsigned char isfat32type(void)
{
	unsigned char has_fat32;
	if (has_esp_partition())
	{
		if (fat_loaded == 1)
		{
			if (strncmp(fat->bpb.bpb2.fat32.type, "FAT32   ", 8) == 0)
			{
				has_fat32 = 1;
			}			
		}
		else
		{
			has_fat32 = 0;
		}		
	}
	else
	{
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
	}
	return has_fat32;
}

unsigned char isfattype(void)
{
	unsigned char has_fat;
	if (has_esp_partition())
	{
		if (fat_loaded == 1)
		{
			if (strncmp(fat->bpb.bpb2.fat16.type, "FAT16   ", 8) == 0)
			{
				has_fat = 1;
			}
			if (strncmp(fat->bpb.bpb2.fat32.type, "FAT32   ", 8) == 0)
			{
				has_fat = 1;
			}			
		}
		else
		{
			has_fat = 1;
		}
	}
	else
	{
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
	}
	return has_fat;
}

unsigned char hasfatlba(void)
{
	unsigned char has_lba;
	if (has_esp_partition())
	{
		if (fat_loaded == 1)
		{
			if (strncmp(fat->bpb.bpb2.fat16.type, "FAT16   ", 8) == 0)
			{
				has_lba = 1;
			}
			if (strncmp(fat->bpb.bpb2.fat32.type, "FAT32   ", 8) == 0)
			{
				has_lba = 1;
			}			
		}
		else
		{
			has_lba = 0;
		}
	}
	else
	{
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
	}
	return has_lba;
}

unsigned char loadfat(void)
{
	unsigned long sector;
	if (has_esp_partition())
	{
		if (!isfattype()) return 0;
		sector = gpt_main_partition->lba_start;
	}
	else
	{
		if (!has_partition_active()) return 0;
		if (!isfattype()) return 0;
		sector = main_partition->lba_start;
	}
	readsector(sector, boot_sector);
	fat = (fat_t*)boot_sector;
	if (fat->signature != BOOT_SIGNATURE) return 0;
	memcpy(bootstrap, fat->bootstrap, FAT32_BOOTSTRAP_SIZE);
	remap_boot();
	fat_loaded = 1;
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
	if (has_esp_partition())
	{
		if (!isfattype()) return 0;		
		start = gpt_main_partition->lba_start + fat->bpb.bpb1.reserved_sectors_count;
	}
	else
	{
		if (!has_partition_active()) return 0;
		if (!isfattype()) return 0;
		start = main_partition->lba_start + fat->bpb.bpb1.reserved_sectors_count;		
	}
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
	if (has_esp_partition())
	{
		return getrootlbaaddress();
	}
	return root_dir;
}

unsigned long getrootdirsectorscount(void)
{
	unsigned long sectors;
	unsigned long entries;
	unsigned long bytes;
	if (has_esp_partition())
	{
		if (!isfattype()) return 0;		
	}
	else
	{
		if (!has_partition_active()) return 0;
		if (!isfattype()) return 0;
	}
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
	if (has_esp_partition())
	{
		if (!isfattype()) return 0;		
	}
	else
	{
		if (!has_partition_active()) return 0;
		if (!isfattype()) return 0;
	}
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
	if (has_esp_partition())
	{
		if (!isfattype()) return 0;		
	}
	else
	{
		if (!has_partition_active()) return 0;
		if (!isfattype()) return 0;
	}
	sector_per_cluster = fat->bpb.bpb1.sector_per_cluster;
	data_sector = getdatasector();
	first_sector = (data_sector + (cluster-2) * sector_per_cluster);
	return first_sector;
}

unsigned long getclusterstart_fat16(void)
{
        unsigned long lba_cluster;
		uint32_t lba_start;
		if (has_esp_partition())
		{
			if (!isfattype()) return 0;		
			lba_start = gpt_main_partition->lba_start;
		}
		else
		{
			if (!main_partition) return 0;
			if (!fat) return 0;
			lba_start = main_partition->lba_start;
		}
		lba_cluster = (lba_start + fat->bpb.bpb1.reserved_sectors_count) + 
					  (fat->bpb.bpb1.number_fats * fat->bpb.bpb1.fat_size_16);
		return lba_cluster;
}

unsigned long getclusterstart_fat32(void)
{
        unsigned long lba_cluster;
		uint32_t lba_start;
		if (has_esp_partition())
		{
			if (!isfattype()) return 0;		
			lba_start = gpt_main_partition->lba_start;
		}
		else
		{
			if (!main_partition) return 0;
			if (!fat) return 0;
			lba_start = main_partition->lba_start;
		}
		lba_cluster = (lba_start + fat->bpb.bpb1.reserved_sectors_count) + 
					  (fat->bpb.bpb1.number_fats * fat->bpb.bpb2.fat32.fat_size_32);
		return lba_cluster;
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

unsigned long getrootdircluster(void)
{
	unsigned long root_cluster;
	if (!has_esp_partition())
	{
		if (!main_partition) return 0;
	}
	if (!isfat32type()) return 0;
	root_cluster = fat->bpb.bpb2.fat32.root_cluster;
	return root_cluster;
}

uint32_t getrootlbaaddress(void)
{
		uint32_t root_lba_address;
		uint32_t lba_start;
		uint32_t root_cluster;
		uint32_t cluster_start;
		remap_boot();
		if (has_esp_partition())
		{
			if (!isfattype()) return 0;		
			lba_start = gpt_main_partition->lba_start;
		}
		else
		{
			if (!main_partition) return 0;
			if (!fat) return 0;
			lba_start = main_partition->lba_start;
		}
		cluster_start = getclusterstart();
		root_cluster = getrootdircluster();
		root_lba_address = cluster_start;
		if (isfat32type()) root_lba_address += ((root_cluster-2) * fat->bpb.bpb1.sector_per_cluster);
		return root_lba_address;
}

uint32_t getrootaddress(void)
{
		uint32_t root_lba_address;
		uint32_t root_address;
		remap_boot();
		if (has_esp_partition())
		{
			if (!isfattype()) return 0;		
		}
		else
		{
			if (!main_partition) return 0;
			if (!fat) return 0;
		}
		root_lba_address = getrootlbaaddress();
		root_address = (root_lba_address * SECTORSIZE);
		return root_address;
}

uint32_t getfilefirstcluster(file_entry_t entry)
{
	uint16_t cluster_hi;
	uint16_t cluster_lo;
	uint32_t cluster;
	remap_boot();
	if (has_esp_partition())
	{
		if (!isfattype()) return 0;		
	}
	else
	{
		if (!main_partition) return 0;
		if (!fat) return 0;
	}
	cluster_hi = isfat32type() ? entry.first_cluster_hi : 0;
	cluster_lo = entry.first_cluster_lo;
	cluster = USHORT16(cluster_hi, cluster_lo);
	return cluster;
}

uint32_t getfirstdatasector()
{
	uint32_t fat_size;
	remap_boot();
	if (has_esp_partition())
	{
		if (!isfattype()) return 0;		
	}
	else
	{
		if (!main_partition) return 0;
		if (!fat) return 0;
	}
	if (isfat16type())
	{
		fat_size = fat->bpb.bpb1.fat_size_16;
	}
	else if (isfat32type())
	{
		fat_size = fat->bpb.bpb2.fat32.fat_size_32;
	}
	uint32_t firstdatasector = fat->bpb.bpb1.reserved_sectors_count+(fat->bpb.bpb1.number_fats * fat_size);
	return firstdatasector;
}

uint32_t getclusterfromsector(uint32_t sector)
{
	uint32_t first_sector = getfirstdatasector();
	uint32_t relative_sector = sector - first_sector;
	uint32_t sector_per_cluster = fat->bpb.bpb1.sector_per_cluster;
	uint32_t cluster_number = (relative_sector / sector_per_cluster) + 2;
	return cluster_number;
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
        fn[11] = '\0';
        strcpy(destination_filename,fn);        
		if (strcmp(destination_filename, "...") == 0) destination_filename[2] = '\0';
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

char *getshortfilename(char *filename11)
{
	char filename[12];
	char filename_s[12];
	char *fn;	
	memcpy(filename_s, filename11, 12);
	filename_s[11] = '\0';
	strfilenamedot8e3s11(filename_s, filename);
	filename[11] = '\0';	
	fn = (char*)filename;	
	return fn;
}

file_entry_t* getfileentryofsector(uint32_t sector)
{
	uint32_t entryoffset;
	uint32_t filecount;
	uint8_t first_sector[SECTORSIZE];
	uint8_t dir_entry_data[FAT_ENTRY_SIZE];
	file_entry_t *file;
	file_entry_t *file_p;
	file_entry_t* entry;
	remap_boot();
	if (has_esp_partition())
	{
		if (!isfattype()) return 0;		
	}
	else
	{
		if (!main_partition) return 0;
		if (!fat) return 0;
	}
	//memset(file, 0, sizeof(file_entry_t)*16);
	//memset(first_sector, 0, SECTORSIZE);
	if (!readsector(sector, first_sector)) return NULL;
	//memset(dir_entry_data, 0, FAT_ENTRY_SIZE);
	//if (first_sector[0] == 0) return NULL;
	filecount = 0;
	file = (file_entry_t*)malloc(sizeof(file_entry_t)*16);
	while (filecount < 16)
	{
		entryoffset = (filecount*FAT_ENTRY_SIZE);
		//if (first_sector[entryoffset] == 0) break;
		memcpy(&dir_entry_data[0], &first_sector[entryoffset], FAT_ENTRY_SIZE);
		entry = (file_entry_t*)&dir_entry_data[0];
		file[filecount] = *entry;
		filecount++;
	}
	file_p = &file[0];
	return file_p;
}

unsigned long readcluster(unsigned long cluster)
{
	unsigned long chain_mask, lba_start;
	remap_boot();
	if (has_esp_partition())
	{
		if (!isfattype()) return 0;		
		lba_start = gpt_main_partition->lba_start;
	}
	else
	{
		if (!main_partition) return 0;
		if (!fat) return 0;
		lba_start = main_partition->lba_start;
	}
	if (isfat16type())
	{
		chain_mask = FAT16_CHAIN_MASK;
	}
	else if (isfat32type())
	{
		chain_mask = FAT32_CHAIN_MASK;
	}
	unsigned char buffer[SECTORSIZE];
    long offset = (long)(lba_start + fat->bpb.bpb1.reserved_sectors_count) + (long)cluster * 4;
    unsigned long val = 0;
	readsector(offset, buffer);
	val = UINT32(buffer[0], buffer[1], buffer[2], buffer[3]);
    return val & chain_mask;
}

uint32_t listrootdir(void)
{
	uint32_t root_sector;
	//uint32_t root_cluster;
	uint32_t filecluster;
	uint32_t filecount;
	uint32_t entrycount;
	uint32_t totalfiles;
	int q;
	file_entry_t* entries;
	char filename[13];
	char fn[12];
	remap_boot();
	if (has_esp_partition())
	{
		if (!isfattype()) return 0;		
	}
	else
	{
		if (!main_partition) return 0;
		if (!fat) return 0;
	}
	q = 0;
	filecount = 0;
	entrycount = 0;
	totalfiles = 0;
	//root_cluster = getrootdircluster();
	//root_sector = getfirstsectorofcluster(root_cluster);
	root_sector = root_sector_start; //getrootdirsector();
	while(q == 0)
	{
		entries = getfileentryofsector(root_sector+entrycount);
		if (entries == NULL)
		{
			q = 1;
			return totalfiles;
		}
		filecount = 0;
		while (filecount < 16)
		{
			if (entries[filecount].name[0] == 0)
			{
				q = 1;
				break;
			}
			if (((uint8_t)entries[filecount].name[0] != FILE_NAME_DELETED) && 
			    (entries[filecount].attribute != F_ATTR_LNGFNM) && 
			    (entries[filecount].attribute != F_ATTR_VOLMID))
			{
				totalfiles++;
				strcpy(filename, getshortfilename(entries[filecount].name));
				printk("%s\n", filename);
			}
			unsigned long eocmark = (isfat32type() ? FAT32_EOC_MARK : FAT16_EOC_MARK);
			unsigned long chainmark = (isfat32type() ? FAT32_CHAIN_MARK : FAT16_CHAIN_MARK);
			unsigned long chainmask = (isfat32type() ? FAT32_CHAIN_MASK : FAT16_CHAIN_MASK);
			if (filecount < 15)
			{
				filecluster = getfilefirstcluster(entries[filecount+1]) & chainmask;
				if (filecluster >= chainmark)
				{
					unsigned long nextcluster = readcluster(filecluster);
					if (nextcluster >= eocmark) 
					{
						filecount++;
						return totalfiles;
					}
				}
			}
			filecount++;
		}
		entrycount++;
		free(entries);
	}
	return totalfiles;
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

uint8_t getlongfilename(char *filename, uint32_t sector)
{
	uint32_t filecount;
	uint32_t entrycount;
	uint32_t totalfiles;
	uint32_t fsize;
	uint32_t fcluster;
	uint32_t fsector;
	uint32_t fwhere;
	char ftype[8];
	int q;
	int d;
	int j;
	int has_lfn = 0;
	char shortfilename[12];
	char longfilename[1024];
	path_sub_t path_sub;
	file_entry_t* entries;
	file_entry_t *find_file;
	if (has_esp_partition())
	{
		if (!isfattype()) return 0;		
	}
	else
	{
		if (!main_partition) return 0;
		if (!fat) return 0;
	}
	d = 0;
	q = 0;
	filecount = 0;
	entrycount = 0;
	path_sub = getpath(strupr(filename));
	strcpy(longfilename, "");
	while(q == 0)
	{
		entries = getfileentryofsector(sector+entrycount);
		if (entries == NULL)
		{
			q = 1;
			break;
		}
		filecount = 0;
		while (filecount < 16)
		{			
			if (entries[filecount].name[0] == 0)
			{
				q = 1;
				break;
			}
			if (((uint8_t)entries[filecount].name[0] != FILE_NAME_DELETED) && 
			    (entries[filecount].attribute != F_ATTR_VOLMID))
			{
				if (entries[filecount].attribute != F_ATTR_LNGFNM)
				{
					if ((entries[filecount].name[0] == '.') && (entries[filecount].name[1] == ' ') && (entries[filecount].attribute & F_ATTR_DIRECT))
					{
						d++;
					}
					if (d > 1)
					{
						q = 1;
						break;
					}
					strcpy(shortfilename, getshortfilename(entries[filecount].name));
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
					char lfn_s[1024];
					file_lfn_entry_t *file_entry_lfn = (file_lfn_entry_t*)&entries[filecount];
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
		free(entries);
	}
	return 0;	
}

file_entry_t* findfileinsector(uint32_t sector, char *filename)
{
	uint32_t filecount;
	uint32_t entrycount;
	uint32_t totalfiles;
	uint32_t fsize;
	uint32_t fcluster;
	uint32_t fsector;
	uint32_t fwhere;
	char ftype[8];
	int q;
	int d;
	int j;
	int has_lfn = 0;
	path_sub_t path_sub;
	file_entry_t* entries;
	file_entry_t* active_file;
	file_entry_t *find_file;
	char shortfilename[13];
	char longfilename[1024];
	char filename_s[1024];
	if (has_esp_partition())
	{
		if (!isfattype()) return 0;		
	}
	else
	{
		if (!main_partition) return 0;
		if (!fat) return 0;
	}
	d = 0;
	q = 0;
	filecount = 0;
	entrycount = 0;
	totalfiles = 0;
	while(q == 0)
	{
		entries = getfileentryofsector(sector+entrycount);
		if (entries == NULL)
		{
			q = 1;
			break;
		}
		filecount = 0;
		while (filecount < 16)
		{			
			if (entries[filecount].name[0] == 0)
			{
				q = 1;
				break;
			}
			if (((uint8_t)entries[filecount].name[0] != FILE_NAME_DELETED) && 
			    (entries[filecount].attribute != F_ATTR_VOLMID))
			{
				if (entries[filecount].attribute != F_ATTR_LNGFNM)
				{
					if ((entries[filecount].name[0] == '.') && (entries[filecount].name[1] == ' ') && (entries[filecount].attribute & F_ATTR_DIRECT))
					{
						d++;
					}
					if (d > 1)
					{
						q = 1;
						break;
					}
					totalfiles++;
					strcpy(shortfilename, getshortfilename(entries[filecount].name));
					strcpy(longfilename, shortfilename);
					if (has_lfn)
					{
						if (getlongfilename(longfilename, sector))
						{
							strcpy(filename_s, longfilename);
						}
						else
						{
							strcpy(filename_s, shortfilename);
						}
						has_lfn = 0;
					}
					else
					{
						strcpy(filename_s, shortfilename);
					}
					fsize = entries[filecount].size;
					fcluster = getfilefirstcluster(entries[filecount]);
					fsector = getfirstsectorofcluster(fcluster);
					fwhere = sectortobytes(fsector);
					if (strcmp(strlwr(filename), strlwr(filename_s)) == 0)
					{
						q = 1;
						active_file = (file_entry_t*)&entries[filecount];
						return active_file;
					}
				}
				else
				{
					has_lfn = 1;
				}
			}
			filecount++;
		}
		entrycount++;
		free(entries);
	}
	return NULL;
}

uint32_t listdir(uint32_t sector)
{
	uint32_t filecount;
	uint32_t entrycount;
	uint32_t filecluster;
	uint32_t totalfiles;
	uint32_t fsize;
	uint32_t fcluster;
	uint32_t fsector;
	uint32_t fwhere;
	char ftype[8];
	int has_lfn = 0;
	int q;
	file_entry_t* entries;
	char shortfilename[13];
	char longfilename[1024];
	char filename[1024];
	remap_boot();
	if (has_esp_partition())
	{
		if (!isfattype()) return 0;		
	}
	else
	{
		if (!main_partition) return 0;
		if (!fat) return 0;
	}
	q = 0;
	filecount = 0;
	entrycount = 0;
	totalfiles = 0;
	/*
	char* column[6] = {"Name", "Type/Size", "Cluster", "Sector", "Position"};
	printk("Root Address 0x%08X\n", sectortobytes(getrootdirsector()));
	printk("---------------------------------------------------------------------------------------------------\n");
	printk("%-32s\t%-10s\t%-10s\t%-10s\t%-10s\n", column[0], column[1], column[2], column[3], column[4]);
	printk("---------------------------------------------------------------------------------------------------\n");
	*/
	while(q == 0)
	{
		entries = getfileentryofsector(sector+entrycount);		
		if (entries == NULL)
		{
			q = 1;
			break;
		}
		filecount = 0;		
		while (filecount < 16)
		{			
			if (entries[filecount].name[0] == 0)
			{
				q = 1;
				break;
			}
			
			if (((uint8_t)entries[filecount].name[0] != FILE_NAME_DELETED) && 
			    (entries[filecount].attribute != F_ATTR_VOLMID))
			{
				if (entries[filecount].attribute != F_ATTR_LNGFNM)
				{
					totalfiles++;			
					strcpy(shortfilename, getshortfilename(entries[filecount].name));
					strcpy(longfilename, shortfilename);
					if (has_lfn)
					{
						if (getlongfilename(longfilename, sector))
						{
							strcpy(filename, longfilename);
						}
						else
						{
							strcpy(filename, shortfilename);
						}
						has_lfn = 0;
					}
					else
					{
						strcpy(filename, shortfilename);
					}
					
					if (strncmp(filename, "...", 3) == 0) 
					{
						filename[2] = '\0';
					}
					
					fsize = entries[filecount].size;
					fcluster = getfilefirstcluster(entries[filecount]);
					fsector = getfirstsectorofcluster(fcluster);
					fwhere = sectortobytes(fsector);
					if (strlen(filename) > 0)
					{
						printk("%s\n", filename);
					}
					
					/*
					if (entries[filecount].attribute & F_ATTR_DIRECT)
					{
						strcpy(ftype, "<DIR>");
						printk("%-32s\t%-12s\t%-10u\t%-10u\t0x%08X\n", filename, ftype, fcluster, fsector, fwhere);
					}
					else
					{
						strcpy(ftype, "");
						printk("%-32s\t%-10u\t%-10u\t%-10u\t0x%08X\n", filename, fsize, fcluster, fsector, fwhere);
					}
					*/
				}
				else
				{
					has_lfn = 1;
					memset(shortfilename, 0, 13);
					memset(longfilename, 0, 1024);
					memset(filename, 0, 1024);
				}				
			}
			unsigned long eocmark = (isfat32type() ? FAT32_EOC_MARK : FAT16_EOC_MARK);
			unsigned long chainmark = (isfat32type() ? FAT32_CHAIN_MARK : FAT16_CHAIN_MARK);
			unsigned long chainmask = (isfat32type() ? FAT32_CHAIN_MASK : FAT16_CHAIN_MASK);
			if (filecount < 15)
			{
				filecluster = getfilefirstcluster(entries[filecount+1]) & chainmask;
				if (filecluster >= chainmark)
				{
					unsigned long nextcluster = readcluster(filecluster);
					if (nextcluster >= eocmark) 
					{
						filecount++;
						return totalfiles;
					}
				}
			}
			filecount++;
		}
		entrycount++;
		free(entries);
	}
	return totalfiles;
}

unsigned long getcwd_sector(const char *path, unsigned long *psector, char *filename, file_entry_t *file)
{
	int j;
	int result = 0;
	char f_path[1024];
	file_entry_t *find_file;
	unsigned long fsector = getrootdirsector();		
	unsigned long lsector = fsector;
	char shortfilename[13];
	char longfilename[1024];
	char file_name[1024];	
	memset(shortfilename, 0, 13);
	memset(longfilename, 0, 1024);
	memset(file_name, 0, 1024);
	memset(f_path, 0, 1024);
	strcpy(f_path, path);
	path_sub_t path_sub = getpath(strupr(f_path));	
	for(j=0;j<path_sub.pathcount;j++)
	{
		if (path_sub.path[j].path[0] != 0)
		{
			find_file = findfileinsector(fsector, path_sub.path[j].path);
			if (find_file != NULL)
			{
				if (find_file->attribute & F_ATTR_DIRECT)
				{
					memcpy(file, find_file, sizeof(file_entry_t));
					fsector = getfirstsectorofcluster(getfilefirstcluster(*find_file));
					lsector = fsector;
					*psector = fsector;
					result = 1;
				}
				else
				{
					if (filename != NULL)
					{
						memcpy(file, find_file, sizeof(file_entry_t));
						fsector = getfirstsectorofcluster(getfilefirstcluster(*find_file));
						lsector = fsector;
						strfilenamedot8e3s11(find_file->name, shortfilename);					
						strcpy(longfilename, shortfilename);												
						if (getlongfilename(longfilename, lsector))
						{
							strcpy(file_name, longfilename);
						}
						else
						{
							strcpy(file_name, shortfilename);
						}
						strcpy(filename, file_name);
						lsector = fsector;
						result = 1;
					}
				}					
			}
		}
	}
	return result;
}

unsigned char findfile(const char *filename, file_entry_t *file, unsigned long *sector, char *find_filename)
{
	int i;
	unsigned char result = 0;
	unsigned char has_file = 0;
	unsigned long fcluster=0;
	unsigned long lsector=0;
	unsigned long file_sector=0;
	char shortfilename[13];
	char longfilename[1024];
	char current_file_name[1024];
	char file_name[1024];
	char find_filename_1[1024];
	remap_boot();
	if (has_esp_partition())
	{
		if (!isfattype()) return 0;		
	}
	else
	{
		if (!main_partition) return 0;
		if (!fat) return 0;
	}
	memset(shortfilename, 0, 13);
	memset(longfilename, 0, 1024);
	memset(current_file_name, 0, 1024);
	memset(file_name, 0, 1024);
	memset(find_filename_1, 0, 1024);
	if (strlen(filename) > 0)
	{
		strcpy(file_name, filename);
		if (strcmp(file_name, "/") == 0)
		{
			file_sector = getrootdirsector();
			has_file = 1;
			fcluster = getclusterfromsector(file_sector);
			memset(file, 0, sizeof(file_entry_t));
			file->attribute = F_ATTR_DIRECT;
			file->first_cluster_lo = UCHAR8A(fcluster);
			file->first_cluster_hi = UCHAR8B(fcluster);
		}
		else
		{
			file_sector = getrootdirsector();
			strcpy(find_filename_1, file_name);
			file_entry_t f, f2;
			memset(&f, 0, sizeof(file_entry_t));
			memset(&f2, 0, sizeof(file_entry_t));
			if (getcwd_sector(file_name, &file_sector, find_filename_1, &f))
			{
				if (f.attribute & F_ATTR_DIRECT)
				{
					memcpy(file, &f, sizeof(file_entry_t));
					has_file = 1;
				}
				else
				{					
					file_entry_t *find_file = findfileinsector(file_sector, find_filename_1);
					if (find_file != NULL)
					{
						file_sector = getfirstsectorofcluster(getfilefirstcluster(*find_file));
						memcpy(file, find_file, sizeof(file_entry_t));
						has_file = 1;
					}
					else
					{
						file_entry_t f2;
						memset(&f2, 0, sizeof(file_entry_t));
						unsigned long file_sector_2 = 0;
						char *file_path_2 = dirname(strdup(file_name));
						char *file_name_2 = basename(strdup(file_name));
						if (getcwd_sector(file_path_2, &file_sector_2, NULL, &f2))
						{
							file_entry_t *find_file_2 = findfileinsector(file_sector_2, file_name_2);
							if (find_file_2 != NULL)
							{							
								file_sector_2 = getfirstsectorofcluster(getfilefirstcluster(*find_file_2));
								file_sector = file_sector_2;
								memcpy(file, find_file_2, sizeof(file_entry_t));
								has_file = 1;
							}
						}
						if (file_path_2) free(file_path_2);
						if (file_name_2) free(file_name_2);
					}
				}		
			}
		}
	}
	if (has_file)
	{
		if (file_sector != 0)
		{
			*sector = file_sector;
			result = 1;
		}
		else
		{
			result = 0;
		}
	}
	else
	{
		result = 0;
	}
	return result;
}

unsigned char readfile(const char *filename, unsigned char *buffer, unsigned long *size)
{
	int result = 0;
	int i;
	remap_boot();
	if (has_esp_partition())
	{
		if (!isfattype()) return 0;		
	}
	else
	{
		if (!main_partition) return 0;
		if (!fat) return 0;
	}
	file_entry_t e;
	unsigned long sector=0;
	memset(&e, 0, sizeof(file_entry_t));
	if (findfile(filename, &e, &sector, NULL))
	{
		if (e.attribute & F_ATTR_DIRECT)
		{
			result = 0;
			return result;
		}
		else
		{
			unsigned long fsz = e.size;
			unsigned long szd = filesizeondisk(fsz);
			unsigned long sectorcount = bytestosector(szd);
			unsigned char *buf = (unsigned char*)malloc(szd);
			if (buf == NULL)
			{
				result = 0;
				return result;
			}
			else
			{
				unsigned char r = readsectors(sector, sectorcount, buf);
				if (r != 0)
				{
					memcpy(buffer, buf, szd);
					*size = szd;
					result = 1;
				}			
				free(buf);
			}
			
		}		
	}
	return result;
}

unsigned long getfilesize(const char *filename)
{
	int result = 0;
	int i;
	remap_boot();
	if (has_esp_partition())
	{
		if (!isfattype()) return 0;		
	}
	else
	{
		if (!main_partition) return 0;
		if (!fat) return 0;
	}
	file_entry_t e;
	unsigned long fsz = 0;
	unsigned long sector=0;
	memset(&e, 0, sizeof(file_entry_t));
	if (findfile(filename, &e, &sector, NULL))
	{
		if (e.attribute & F_ATTR_DIRECT)
		{
			result = 0;
			return result;
		}
		else
		{
			fsz = e.size;
		}
	}
	return fsz;
}

unsigned long fileexists(const char *filename)
{
	int result = 0;
	int i;
	remap_boot();
	if (has_esp_partition())
	{
		if (!isfattype()) return 0;		
	}
	else
	{
		if (!main_partition) return 0;
		if (!fat) return 0;
	}
	file_entry_t e;
	unsigned long sector=0;
	memset(&e, 0, sizeof(file_entry_t));
	if (findfile(filename, &e, &sector, NULL))
	{
		if (e.attribute & F_ATTR_DIRECT)
		{
			result = 0;
		}
		else
		{
			result = 1;
		}
	}
	return result;
}

unsigned long direxists(const char *path)
{
	int result = 0;
	int i;
	remap_boot();
	if (has_esp_partition())
	{
		if (!isfattype()) return 0;		
	}
	else
	{
		if (!main_partition) return 0;
		if (!fat) return 0;
	}
	file_entry_t e;
	unsigned long sector=0;
	memset(&e, 0, sizeof(file_entry_t));
	if (findfile(path, &e, &sector, NULL))
	{
		if (e.attribute & F_ATTR_DIRECT)
		{
			result = 1;
		}
		else
		{
			result = 0;
		}
	}
	return result;
}



EFI_STATUS load_efi_image(
    VOID* ImageBuffer,
    UINTN ImageSize,
    EFI_BOOT_SERVICES* BS,
    EFI_HANDLE ParentImageHandle
) 
{
    EFI_STATUS Status;
    EFI_HANDLE LoadedImageHandle = NULL;
    Status = BS->LoadImage(
        FALSE,                 
        ParentImageHandle,
        NULL,                  
        ImageBuffer,           
        ImageSize,             
        &LoadedImageHandle     
    );
    if (EFI_ERROR(Status)) 
    {
        return Status;
    }
    UINTN ExitDataSize = 0;
    CHAR16* ExitData = NULL;
    Status = BS->StartImage(
        LoadedImageHandle, 
        &ExitDataSize,     
        &ExitData          
    );
    if (EFI_ERROR(Status)) 
    {
        BS->UnloadImage(LoadedImageHandle); 
    }
    return Status;
}

unsigned char loadefi(unsigned char *imagebuffer, unsigned long imagesize)
{
	unsigned char result = 0;
    if (!Handle) return 0;
    if (!ST) return 0;
	result = load_efi_image(imagebuffer, imagesize, ST->BootServices, Handle);
	return result;
}

char char16_to_char(char16_t c)
{
    if (c <= 0x7F) 
    {
        return (char)c;
    } else 
    {
        return '?';
    }
}

void __chkstk_ms(void)
{
}

EFI_STATUS EFIAPI efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) 
{	
	InitializeLib(ImageHandle, SystemTable);

	printk("Starting FastSystem...\n");
	printk("\n");
	printk("EFI loaded.\n");
	printk("Loading base configuration.\n");
	loadpci();
	
	init_heap();
	
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
	
	printk("\n");
	
	if (loadmbr())
	{
		partition = (partition_entry_t*)mbr->partition;
		if ((has_efi_support == 1) && (has_gpt == 1))
		{
			if (loadgpt() == 0)
			{
				panic((unsigned long)gpt_header);
			}
		}
		else
		{
			if (has_partition_active())
			{
				main_partition = (partition_entry_t*)&partition[active_partition];
			}
			else
			{
				panic((unsigned long)partition);
			}
		}
	}
	else
	{
		printk("Disk Error\n");
		panic((unsigned long)mbr_sector);
	}
	
	if (has_esp_partition())
	{
		printk("EFI System Partition found:\n");
		printk("  Starting LBA: %d\n", (unsigned long long)gpt_main_partition->lba_start);
		printk("  Ending LBA: %d\n", (unsigned long long)gpt_main_partition->lba_end);
		printk("  Attributes: 0x%X\n", (unsigned long long)gpt_main_partition->attributes);
		if (isfattype())
		{
			if (loadfat())
			{
				root_sector_start = getrootlbaaddress();
			}
			else
			{
				panic((unsigned long)boot_sector);
			}
		}
		else
		{
			panic((unsigned long)gpt_entry_ptr);
		}
	}
	else
	{
		if (has_partition_active())
		{
			if (isfattype())
			{
				if (loadfat())
				{
					root_sector_start = getrootdirsector();
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
	}
	
	printk("\n");
	
	if (has_esp_partition())
	{
		if (isfat32type()) 
		{
			printk("FAT32 filesystem detected in EFI partition.\n");
			printk("  Root Sector: %d\n", root_sector_start);
			printk("\n");
			printk("--------------------------------------------\n");
			printk("ROOT DIRECTORY: /\n");
			printk("--------------------------------------------\n");
			listdir(root_sector_start);
			printk("--------------------------------------------\n");
			file_entry_t f;
			unsigned long dirsector=0;
			memset(&f, 0, sizeof(file_entry_t));
			
			char *current_path = "/efi/FastSystem";
			if (direxists(current_path))
			{
				if (getcwd_sector(current_path, &dirsector, NULL, &f))
				{
					printk("List Directory: %s\n", current_path);
					listdir(dirsector);
					printk("--------------------------------------------\n");
					#if defined(__i386__)
					char *file_name = "/efi/FastSystem/loader.efi";
					printk("Arch: i386\n");
					#else
					char *file_name = "/efi/FastSystem/loader64.efi";
					printk("Arch: amd64\n");
					#endif
					printk("--------------------------------------------\n");
					if (fileexists(file_name))
					{
						printk("File Name: %s\n", file_name);
						unsigned long file_size = getfilesize(file_name);
						printk("File Size: %d\n", file_size);
						printk("--------------------------------------------\n");
						unsigned char *file_buffer = (unsigned char*)malloc(file_size);
						if (file_buffer != NULL)
						{
							if (readfile(file_name, file_buffer, &file_size))
							{
								dump_hex_address(0, file_buffer, 256);
								printk("--------------------------------------------\n");
								if (loadefi(file_buffer, file_size) != EFI_SUCCESS)
								{
									panic((unsigned long)file_buffer);
								}
							}
							free(file_buffer);
						}
					}
					else
					{
						printk("%s not found.\n", strupr(basename(file_name)));
					}
					printk("--------------------------------------------\n");
					printk("\n");
					int i;
					int gpt_part_count = getgptpartitioncount();
					for(i=0;i<gpt_part_count;i++)
					{
						if (has_gpt_partition(i))
						{
							unsigned char tmp_guid[16];
							memcpy(tmp_guid, getgptpartitiontypeguid(i), 16);
							guid_swap_endianness(tmp_guid);
							if (guid_validate(tmp_guid))
							{
								unsigned long gpt_lba_start = (unsigned long)gpt_partition[i].lba_start;
								unsigned long gpt_lba_end = (unsigned long)gpt_partition[i].lba_end;
								char gpt_type_guid[64];
								memset(gpt_type_guid, 0, 64);
								guidtostring(tmp_guid, gpt_type_guid, 1);
								printk("GPT Partition Found %d:\n", i);
								printk("  Type GUID: %s\n", gpt_type_guid);
								printk("  LBA Start: 0x%08X\n", gpt_lba_start);
								printk("  LBA End: 0x%08X\n", gpt_lba_end);
								printk("\n");
							}
						}
					}
					printk("--------------------------------------------\n");
				}
			}
			else
			{
				printk("%s not found.\n", current_path);
			}
		}
		else 
		{
			printk("Unknown filesystem in EFI partition.\n");
		}
	}
	
	EFI_INPUT_KEY Key;
    EFI_STATUS Status;
    char cKey;
    char cKeyS[2];
    memset(&Key, 0, sizeof(EFI_INPUT_KEY));
    memset(cKeyS, 0, 2);
	while (1)
	{
		Status = WaitForKeyStroke(&Key);
        if (EFI_ERROR(Status)) 
        {
            printk("Failed to obtain the key: %d\n", Status);
            break;
        }
        if (Key.ScanCode == SCAN_ESC) 
        {
            break;
        }
        cKey = char16_to_char(Key.UnicodeChar);
        if (cKey != 0)
        {
        	cKeyS[0] = cKey;
        	cKeyS[1] = 0;
        	if (cKey == CHAR_BACKSPACE)
        	{
        		//printk("\b \b");
        		/*
        		unsigned short cursor_pos = getcursor();
        		if (cursor_pos > 1) cursor_pos-2;
        		setcursoroffset(cursor_pos);
        		printk(" ");
        		cursor_pos++;
        		setcursoroffset(cursor_pos);
        		*/
        	}
        	else
        	if (cKey == CHAR_TAB)
        	{
        		printk("\t");
        	}
        	else
        	if (cKey == CHAR_CARRIAGE_RETURN)
        	{
        		printk("\n");
        	}
        	else
        	{
        		printk("%s", cKeyS);
        	}
        	//printk("Key pressed: 0x%02X\n", cKey);
        }
	}
	while(1);
	return EFI_SUCCESS;
}

