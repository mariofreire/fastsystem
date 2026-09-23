asm (".code16gcc");
asm ("jmp main");

extern void putc(char c);
extern void print_hex(unsigned short v);
extern unsigned char getdrivenumber();
extern unsigned char diskread(void *buffer, unsigned long seek);
extern void oserror();

unsigned char *mbr_sector = (unsigned char *)0x7C00;
unsigned char *boot_sector = (unsigned char *)0x7E00;


#define SECTORSIZE      512

#define PARTITION_INACTIVE                    0x00
#define PARTITION_ACTIVE                      0x80

#define PARTITION_UNKNOWN                     0x00
#define PARTITION_FAT12                       0x01
#define PARTITION_FAT16_UNDER_FAT32           0x04
#define PARTITION_EXTENDED                    0x05
#define PARTITION_FAT16_OVER_FAT32            0x06
#define PARTITION_FAT32                       0x0B
#define PARTITION_FAT32_LBA                   0x0C
#define PARTITION_FAT16_OVER_32MB_LBA         0x0E
#define PARTITION_EXTENDED_LBA                0x0F

#pragma pack (push, 1)

// chs address position
typedef struct
{
unsigned char head;
unsigned char sector;
unsigned char cylinder;
} chs_t;

// partition table entries
typedef struct
{
unsigned char status;
chs_t chs_start;
unsigned char type;
chs_t chs_end;
unsigned long int lba_start;
unsigned long int sector_count;
} partition_entry_t;

// master boot record
typedef struct
{
unsigned char bootcode[0x1BE];
partition_entry_t partition[4];
unsigned short int signature;
} mbr_t;

#pragma pack (pop)


mbr_t* mbr;

void puts(const char *s) 
{
    while(*s) 
	{
        putc(*s++);
    }
}

void main()
{
  unsigned short lba_start = 0;
  int i = 0;
  unsigned char drive = getdrivenumber();
  unsigned char status = 0;
  mbr = (mbr_t*)mbr_sector;
  
 // puts("Hello World!");
  //print_hex(mbr->partition[0].status);
  
  while(i < 4)
  {
	if (mbr->partition[i].status & PARTITION_ACTIVE)
	{
		lba_start = mbr->partition[i].lba_start;
	}	
	i++;
  }
  if (lba_start == 0)
  {
	  return;
  }
  status = diskread(boot_sector, lba_start);
  
  if (status == 1) 
  {
	  //print_hex(status);
	  /*
	  asm ("mov $0x7e00, %ax");
	  while(1);
	  asm ("push %ax");
	  asm ("ret");
	  */
	  
	  //asm ("mov $0x7c00, %ax");
	  //asm ("mov %ax, %ds");
	  //asm ("mov %ax, %es");
	  //asm ("mov %ax, %ss");
	  asm ("mov $0x7e00, %ax");
	  asm ("push %ax");
	  asm ("ret");
  }
  else 
  {
	  oserror();
  }
  /*
	  asm ("mov $0x7e00, %ax");
//	  asm ("mov %ax, %ds");
	  asm ("mov %ax, %es");
	  asm ("mov %ax, %ss");
	  asm ("push %ax");
	  asm ("ret");
  */
}

