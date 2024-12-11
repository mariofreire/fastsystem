#ifndef fat16info_h_
#define fat16info_h_

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
uint8_t head;
uint8_t sector;
uint8_t cylinder;
} chs_t;

// partition table entries
typedef struct
{
uint8_t status;
chs_t chs_start;
uint8_t type;
chs_t chs_end;
uint32_t lba_start;
uint32_t sector_count;
} partition_entry_t;

// master boot record
typedef struct
{
uint8_t bootcode[0x1BE];
partition_entry_t partition[4];
uint16_t signature;
} mbr_t;

// fat16 boot sector
typedef struct
{
uint16_t jumpcode;
uint8_t nop;
uint8_t oem_name[8];
uint16_t bytes_per_sector;
uint8_t sector_per_cluster;
uint16_t reserved_sector_count;
uint8_t table_count;
uint16_t root_entry_count;
uint16_t total_sectors_16;
uint8_t media_descriptor;
uint16_t table_size_16;
uint16_t sectors_per_track;
uint16_t head_side_count;
uint32_t hidden_sector_count;
uint32_t total_sectors_32;
uint8_t drive_number;
uint8_t reserved_1;
uint8_t boot_signature;
uint32_t volume_id;
uint8_t volume_label[11];
uint8_t fat_type_label[8];
uint8_t bootcode[0x1C0];
uint16_t signature;
} fat16_boot_sector_t;

typedef struct
{
  char name[11];
  uint8_t attribute;
  uint8_t reserved;
  uint8_t creation_time_sec_thents;
  uint16_t creation_time_2_seconds;
  uint16_t creation_date;
  uint16_t last_access_date;
  uint16_t first_cluster_hi_word;
  uint16_t last_write_time;
  uint16_t last_write_date;
  uint16_t first_cluster_lo_word;
  uint32_t size;
} fat16_file_entry_t;


typedef struct
{
  char name1[11];
  uint8_t attribute;
  char name2[20];
} fat16_file_lfn_entry_t;

#pragma pack (pop)

extern uint32_t imagedisk_size;
extern FILE * imagedisk_file;
extern mbr_t *mbr;
extern fat16_boot_sector_t *bootsector;
extern partition_entry_t *partition;     
extern partition_entry_t *main_partition;
extern uint8_t active_partition;
extern uint8_t MBR[SECTORSIZE];
extern uint8_t BOOTSECTOR[SECTORSIZE];
extern char volume_id[11];

uint8_t fileexists(char *filename);
int filesize(char *filename);

uint8_t initimagedisk(char *filename);
uint8_t uninitimagedisk(void);
uint8_t readsector(uint32_t sector, uint8_t *buffer);
uint8_t loadmbr(void);
uint8_t loadbootsector(void);
uint8_t loadrootaddress(void);
uint8_t loadrootentries(void);
uint8_t isfat16type(void);
uint8_t getpartitiontype(void);
uint32_t getpartitionsize(void);
uint32_t getpartitionstart(void);
uint32_t getpartitionlbastart(void);
uint32_t getclusterstart(void);
uint32_t getrootlbaaddress(void);
uint32_t getrootaddress(void);
void getoemname(char *oem_name);
void getvolumelabel(char *volume_label);
void getfattypelabel(char *fat_type_label);

void strfilenamedot8e3s11(char source_filename[11], char destination_filename[12]);
void strtrm(char *s1, char *s2);

#endif
