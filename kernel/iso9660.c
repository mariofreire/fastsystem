// Read Sector Using IDE Device

#include <conio.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>

#define ATA_SECTOR_SIZE 512
#define ATAPI_SECTOR_SIZE 2048

#define FLAGBIT(value, index) ((value >> index) & ((1 << 1) - 1))

#define UCHAR8A(value) ((unsigned char)(value))
#define UCHAR8B(value) ((unsigned char)((value)>> 8))
#define UCHAR8C(value) ((unsigned char)((value)>>16))
#define UCHAR8D(value) ((unsigned char)((value)>>24))
#define UINT16(a,b) ((unsigned long)((unsigned char)(a)|((unsigned char)(b)<<8)))
#define UINT32(a,b,c,d) ((unsigned long)((unsigned char)(a)|((unsigned char)(b)<<8)|((unsigned char)(c)<<16)|((unsigned char)(d)<<24)))
#define USHORT16(a,b) ((unsigned long)(((unsigned long)(a)<<16)|((unsigned short)(b))))

#define HIGH16(a) ((unsigned short)(((a)>>16)&0xFFFF))
#define LOW16(a) ((unsigned short)((a)&0xFFFF))

#define MAX_CACHED_DIRS 1024

unsigned int atoh(char *s);

void dump_hex(const void *data, size_t size) {
    const unsigned char *buffer = (const unsigned char *)data;
    size_t i, j;

    for (i = 0; i < size; i += 16) {
        printf("%06x: ", (unsigned int)i);

        for (j = 0; j < 16; j++) {
            if (i + j < size)
                printf("%02x ", buffer[i + j]);
            else
                printf("   ");

            if (j % 16 == 7)
                printf(" ");
        }

        printf("\n");
    }
}

void ata_io_wait(uint16_t base)
{
	inb(base + 0x206);
	inb(base + 0x206);
	inb(base + 0x206);
	inb(base + 0x206);
}

void ata_reset(uint16_t base)
{
	outb(base + 0x206, 4);
	msleep(10);
	outb(base + 0x206, 0);
	msleep(10);
}

unsigned char ata_wait(uint16_t base, int mask, int state)
{
	int s;
	int t=0;
	while(1)
	{
		s = inb(base + 7);
		if((s & mask) == state)
		{
			return 1;
		}
		if ((s & 0x01) || (t >= 300))
		{
			ata_reset(base);
			return 0;
		}
		//msleep(10);
		//for(int i=0;i<1000;i++);
		t++;
	}
}

void ata_read_sector(uint8_t id, uint32_t lba, uint8_t *buffer) {
    // Assuming the use of a specific I/O port for reading from the IDE device
	uint8_t second = 0;
	uint8_t slave = 0;
	uint8_t drive = 0xE0;
	uint16_t base = 0x1F0;
	if (id >= 4) return;
	slave = (id % 2);
	second = ((id >> 1) % 2);
	if (slave == 1) drive |= 0x10;
	if (second == 1) base &= ~0x80;
	outb(base + 0x206, 0);
    outb(base + 6, drive | ((lba >> 24) & 0x0F)); // Select drive
    outb(base + 1, 0x00); // Clear error register
    outb(base + 2, 1); // Number of sectors to read
    outb(base + 3, (uint8_t)(lba & 0xFF)); // LBA low
    outb(base + 4, (uint8_t)((lba >> 8) & 0xFF)); // LBA mid
    outb(base + 5, (uint8_t)((lba >> 16) & 0xFF)); // LBA high
    outb(base + 7, 0x20); // Command to read sectors

    // Wait for the drive to be ready
    while (!(inb(base + 7) & 0x08)) msleep(10);

    // Read the sector data
    for (int i = 0; i < ATA_SECTOR_SIZE / 2; i++) {
        ((uint16_t *)buffer)[i] = inw(base);
    }
}

int atapi_read_sector(uint8_t id, uint32_t lba, uint8_t *buffer, int count) {
    // Assuming the use of a specific I/O port for reading from the IDE device
	uint8_t second = 0;
	uint8_t slave = 0;
	uint8_t flags = 0xA0;
	uint16_t base = 0x1F0;
	if (id >= 4) return 0;
	slave = (id % 2);
	second = ((id >> 1) % 2);
	if (slave == 1) flags |= 0x10;
	if (second == 1) base &= ~0x80;
	ata_reset(base);
	outb(base + 6, flags);
	ata_io_wait(base);
	outb(base + 1, 0);
	outb(base + 4, (2048 & 0xFF));
	outb(base + 5, (2048 >> 8));
	outb(base + 7, 0xA0);
	ata_io_wait(base);
	while (1)
	{
		uint8_t status = inb(base + 7);
		if ((status & 0x01) == 1) return 0;
		if ((status & 0x08) && (!(status & 0x80))) break;
		ata_io_wait(base);		
	}
	outw(base, UINT16(0xA8,0x00));
	outw(base, UINT16(((lba >> 0x18) & 0xFF),((lba >> 0x10) & 0xFF)));
	outw(base, UINT16(((lba >> 0x08) & 0xFF),((lba >> 0x00) & 0xFF)));
	outw(base, UINT16(((count >> 0x18) & 0xFF),((count >> 0x10) & 0xFF)));
	outw(base, UINT16(((count >> 0x08) & 0xFF),((count >> 0x00) & 0xFF)));
	outw(base, 0x00);

    // Read the sector data
	for(int j=0;j<count;j++)
	{
		// Wait for the drive to be ready
		while (1)
		{
			uint8_t status = inb(base + 7);
			if ((status & 0x01) == 1) return 1;
			if ((status & 0x08) && (!(status & 0x80))) break;
		}
		for (int i = 0; i < ATAPI_SECTOR_SIZE / 2; i++) 
		{
			((uint16_t *)buffer)[i+(j*(ATAPI_SECTOR_SIZE/2))] = inw(base);
		}
	}
	return 1;
}

unsigned char get_ata_ident(int id, int command) 
{
	int r;
	uint8_t second = 0;
	uint8_t slave = 0;
	uint16_t base = 0x1F0;
	uint8_t flags = 0xE0;	
	if (id >= 4) return 0;
	slave = (id % 2);
	second = ((id >> 1) % 2);
	if (second == 1) base &= ~0x80;
	if (inb(base + 7) == 0xFF) return 0;
	ata_reset(base);
	if(slave == 1) flags |= 0x10;
	if(!ata_wait(base, 0x80, 0)) return 0;
	outb(base + 6, flags);
	if(command == 0xA1) r = ata_wait(base, 0x80, 0);
	else r = ata_wait(base, 0x80|0x40, 0x40);
	if(!r) return 0;
	outb(base + 0x206, 0);
	outb(base + 1, 0);
	outb(base + 2, 0);
	outb(base + 3, 0);
	outb(base + 4, 0);
	outb(base + 5, 0);
	outb(base + 6, flags);
	outb(base + 7, command);
	if (!ata_wait(base, 8, 8)) return 0;
	return 1;
}

unsigned char get_ata_type(int id)
{
	unsigned char is_ata, is_atapi;
	is_ata = get_ata_ident(id, 0xEC);
	is_atapi = get_ata_ident(id, 0xA1);
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

void read_sector(uint32_t lba, uint8_t *buffer) {
	memset(buffer, 0, ATA_SECTOR_SIZE);
	if (get_ata_type(0) == 1) ata_read_sector(0, lba, buffer);
}

int get_atapi_sector_length(int size) {
	int l = 0;
	for(int i=0;i<size;i+=ATAPI_SECTOR_SIZE) l += ATAPI_SECTOR_SIZE;
	return l;
}

void dump_sector(uint32_t lba, uint16_t length) {
	int size = get_atapi_sector_length(length);
	int id = 0;
	int type = 0;
	uint8_t *buffer = (uint8_t*)malloc(size);
	memset(buffer, 0, size);
	while(id < 4)
	{
		type = get_ata_type(id);
		if (type == 2) break;
		id++;
	}
	if (type == 2) 
	{
		atapi_read_sector(id, lba, buffer, (size / ATAPI_SECTOR_SIZE));
		dump_hex((unsigned char*)buffer, length);
	}
	free(buffer);
}

int atapi_id = -1;

int init_atapi()
{
	int id = 0;
	int type = 0;
	while(id < 4)
	{
		type = get_ata_type(id);
		if (type == 2) break;
		id++;
	}
	if (type == 2) 
	{
		atapi_id = id;
		return 1;
	}
	atapi_id = -1;
	return 0;
}

int atapi_read(uint32_t sector, uint8_t* buffer, uint32_t count)
{
	int r = 0;
	if (atapi_id < 0) return r;
	r = atapi_read_sector(atapi_id, sector, buffer, count);
	return r;
}

#define PVD_OFFSET 16 // Primary Volume Descriptor is usually found after 16 sectors


#pragma pack (push, 1)

// Structure for the Volume Descriptor (VD)
typedef struct
{
    uint8_t type;                      // Type of descriptor
    char identifier[5];                // Always "CD001"
    uint8_t version;                   // Version of the Volume Descriptor, usually 0x01
	uint8_t data[2041];
} volume_descriptor_t;

// Structure for the Primary Volume Descriptor (PVD)
typedef struct 
{
    uint8_t type;                                            // volume type: 1 = standard, 2 = coded
    char identifier[5];                                      // volume structure standard id = CD001
    uint8_t version;                                         // volume structure version number = 1
    uint8_t VolumeFlags;                                     // volume flags
    char system_identifier[32];                              // system identifier
    char volume_identifier[32];                              // volume identifier
    uint8_t unused_2[8];                                     // Reserved
    uint32_t volume_space_size;                              // size of the volume in LBN's (LittleEndian)
    uint32_t volume_space_size_BE;                           // size of the volume in LBN's (BigEndian)
    uint8_t unused_3[32];                                    // Unused Field (all zeroes)
    uint16_t volume_set_size;                                // Volume Set Size (int16_LSB-MSB)
    uint16_t volume_set_size_BE;                             // Volume Set Size (BigEndian)
    uint16_t volume_sequence_number;                         // Volume Sequence Number (int16_LSB-MSB)
    uint16_t volume_sequence_number_BE;                      // Volume Sequence Number (BigEndian)
    uint16_t logical_block_size;                             // Logical Block Size (int16_LSB-MSB)
    uint16_t logical_block_size_BE;                          // Logical Block Size (BigEndian)
    uint32_t path_table_size;                                // Path Table Size (int32_LSB-MSB)
    uint32_t path_table_size_BE;                             // Path Table Size (BigEndian)
    uint32_t location_of_type_l_path_table;                  // PathTabLocation[0]
    uint32_t location_of_optional_type_l_path_table;         // PathTabLocationBE[0]
    uint32_t location_of_type_m_path_table;                  // PathTabLocation[1]
    uint32_t location_of_optional_type_m_path_table;         // PathTabLocationBE[1]
    uint8_t root_directory[34];                              // Directory entry for the root directory (fixed size)
    char volume_set_identifier[128];                         // Volume Set Identifier
    char publisher_identifier[128];                          // Publisher Identifier
    char data_preparer_identifier[128];                      // Data Preparer Identifier
    char application_identifier[128];                        // Application Identifier
    char copyright[37];                                      // file name of copyright notice
    char abstract[37];                                       // file name of abstract
    char bibliograph[37];                                    // file name of bibliography
    char create_date[17];                                    // volume creation date and time
    char mod_date[17];                                       // volume modification date and time
    char expire_date[17];                                    // volume expiration date and time
    char effect_date[17];                                    // volume effective date and time
    uint8_t file_struct_ver;                                  // file structure version number = 1
    uint8_t reserved3;                                       // reserved
    uint8_t res_app[512];                                    // reserved for application
    uint8_t reserved4[653];                                  // remainder of 2048 bytes reserved
} primary_volume_descriptor_t;

// Directory Entry structure
typedef struct {
    uint8_t length;                 // Length of the directory entry
    uint8_t extended_attr;          // Extended attribute (usually not used)
    uint16_t date_time;             // Date and time (may not be used in this basic version)
    uint16_t file_flags;            // File flags (0x02 for files, 0x01 for directories)
    uint16_t file_unit_size;        // File unit size (usually 1 in ISO 9660)
    uint16_t interleave_gap_size;   // Interleave gap size (usually not used)
    uint32_t starting_sector;       // Starting sector (LBA)
    uint32_t file_size;             // File size in bytes
    uint8_t name_length;            // Length of file or directory name
    char name[1];                   // Name of file/directory (variable length)
} DirectoryEntry;

// Cache directory entries to avoid reading from the ISO multiple times
typedef struct {
    char directory_name[256];
    DirectoryEntry *entries;  // Array of DirectoryEntry
    int entry_count;
} DirectoryCache;

// Path Table Entry structure (simplified version)
typedef struct {
    uint8_t length;             // Length of the entry
    uint8_t extended_attr;      // Extended attribute record (unused in this basic implementation)
    uint32_t parent_dir;        // Parent directory
    uint8_t name_len;           // Length of the name
    char name[1];               // Name (variable length)
} PathTableEntry;

// Path Entry structure (simplified version)
typedef struct {
    uint8_t length;             // Length of the entry
    uint8_t extended_attr;      // Extended attribute record (unused in this basic implementation)
    uint32_t location;        // Location
	uint32_t location_be;        // Location
    uint32_t data_length;             // Data Length
    uint32_t data_length_be;             // Data Length
	uint8_t record_time[6];
	uint8_t flags_hsg;
	uint8_t flags_iso;
	uint8_t int_leave_size;
	uint8_t int_leave_skip;
	uint16_t vssn;
	uint16_t vssn_le;
    uint8_t name_len;           // Length of the name
    char name[1];               // Name (variable length)
} PathEntry;

#pragma pack (pop)

DirectoryCache dir_cache[MAX_CACHED_DIRS];
int cache_index = 0;

primary_volume_descriptor_t *pvd;

int has_pvd = 0;
int has_bvd = 0;
int has_svd = 0;
int has_joilet = 0;
int has_vdst = 0;
int has_rockridge = 0;
int has_vpd = 0;
int has_evd = 0;
int has_terminator_vd = 0;

uint32_t pvd_sector = 0;
uint32_t bvd_sector = 0;
uint32_t svd_sector = 0;
uint32_t vpd_sector = 0;
uint32_t vdst_sector = 0;
uint32_t rrvd_sector = 0;
uint32_t evd_sector = 0;
uint32_t root_sector = 0;

uint32_t root_address = 0;

int snprintf (char *s, size_t maxlen, const char *format, ...);

int init_vd()
{
	int find_vd = 1;
	int has_error = 0;
	uint32_t vd_offset = PVD_OFFSET;	
	while (find_vd)
	{
		int has_type = 0;
		int has_vd = 0;
		volume_descriptor_t *vd;
		uint8_t *buffer = (uint8_t*)malloc(ATAPI_SECTOR_SIZE);
		int can_read = atapi_read(vd_offset, buffer, 1);
		if (can_read == 1)
		{
			vd = (volume_descriptor_t*)buffer;
			switch (vd->type)
			{
				case 0x00:
				{
					bvd_sector = vd_offset;
					has_bvd = 1;
					has_type = 1;
				}
				break;
				case 0x01:
				{
					pvd_sector = vd_offset;
					has_pvd = 1;
					has_type = 1;
				}
				break;
				case 0x02:
				{
					svd_sector = vd_offset;
					has_svd = 1;
					has_joilet = 1;
					has_type = 1;
				}
				break;
				case 0x03:
				{
					vpd_sector = vd_offset;
					has_vpd = 1;
					has_type = 1;
				}
				break;
				case 0x05:
				{
					evd_sector = vd_offset;
					has_evd = 1;
					has_type = 1;
				}
				break;
				case 0x06:
				{
					rrvd_sector = vd_offset;
					has_rockridge = 1;
					has_type = 1;
				}
				break;
				case 0xFF:
				{
					vdst_sector = vd_offset;
					has_terminator_vd = 1;
					has_vdst = 1;
					find_vd = 0;
					has_type = 1;
				}
				break;
			};
			if (has_type == 1)
			{
				if (strncmp(vd->identifier, "CD001", 5) == 0) 
				{
					has_vd = 1;
				}
				else
				{
					has_error = 1;
					find_vd = 0;
				}
			}
			vd_offset++;
			if (has_vd == 0)
			{
				has_error = 1;
				find_vd = 0;
			}
			if ((vd_offset > 1024) && (has_terminator_vd == 0))
			{
				has_error = 1;
				find_vd = 0;			
			}
			if (pvd_sector != PVD_OFFSET)
			{
				has_error = 1;
				find_vd = 0;			
			}
		}
		else
		{
			has_error = 1;
			find_vd = 0;
		}
		free(buffer);
	}
	if (has_error == 1) return 0;
	return 1;
}

int read_pvd()
{
	char pvd_id[10];
	char sys_id[64];
	char vol_id[64];
	char root_dir[64];
	int retval = 0;
	if ((has_pvd == 1) && (pvd_sector != 0))
	{
		uint8_t *buffer = (uint8_t*)malloc(ATAPI_SECTOR_SIZE);
		if (buffer == NULL) return retval;
		memset(pvd_id, 0, 10);
		memset(sys_id, 0, 64);
		memset(vol_id, 0, 64);
		memset(root_dir, 0, 64);
		atapi_read(pvd_sector, buffer, 1);
		pvd = (primary_volume_descriptor_t*)buffer;
		// Check if the descriptor is valid
		if (pvd->type == 1 && strncmp(pvd->identifier, "CD001", 5) == 0) {
			strncpy(pvd_id, pvd->identifier, 5);
			strncpy(sys_id, pvd->system_identifier, 32);
			strncpy(vol_id, pvd->volume_identifier, 32);
			memcpy(root_dir, pvd->root_directory, 32);
			root_sector = (pvd->location_of_type_l_path_table);
			root_address = (root_sector * ATAPI_SECTOR_SIZE);
			printf("Primary Volume Descriptor Found\n");
			printf("Standard Identifier: %s\n", pvd_id);
			printf("System Identifier: %s\n", sys_id);
			printf("Volume Identifier: %s\n", vol_id);
			printf("Volume Space Size: %u blocks\n", pvd->volume_space_size);
			printf("Logical Block Size: %u bytes\n", pvd->logical_block_size);
			printf("Path Table Size: %u bytes\n", pvd->path_table_size);
			printf("Location of Type-L Path Table: 0x%X\n", pvd->location_of_type_l_path_table);
			printf("Location of the Optional Type-L Path Table: 0x%X\n", pvd->location_of_optional_type_l_path_table);
			printf("Location of Type-M Path Table: 0x%X\n", pvd->location_of_type_m_path_table);
			printf("Location of Optional Type-M Path Table: 0x%X\n", pvd->location_of_optional_type_m_path_table);
			printf("Root Directory: 0x%X\n", root_dir);
			printf("Root Directory LBA: %d\n", root_sector);
			printf("Root Directory Address: 0x%X\n", root_address);
			printf("Sector Size: %d\n", ATAPI_SECTOR_SIZE);
			retval = 1;
		} else {
			printf("Invalid Primary Volume Descriptor.\n");
			retval = 0;
		}
		free(buffer);
	}
	else
	{
		return retval;
	}
	return retval;
}


int translate_filename_iso(PathEntry *path_entry, char *dest_fn)
{
	char * tmp_fn = path_entry->name;
	int len = path_entry->name_len;
	int i = 0;
	while(i < len)
	{
		unsigned char ch = tmp_fn[i];
		if (!ch)
		{
			break;
		}
		if ((ch >= 'A') && (ch <= 'Z'))
		{
			ch |= 0x20;
		}
		if ((ch == '.') && (i == len - 3) && (tmp_fn[i + 1] == ';') && (tmp_fn[i + 2] == '1'))
		{
			break;
		}
		if ((ch == ';') && (i == len - 2) && (tmp_fn[i + 1] == '1'))
		{
			break;
		}
		if ((ch == ';') || (ch == '/'))
		{
			ch = '.';
		}
		dest_fn[i] = ch;
		i++;
	}
	return i;
}

// Function to read the path table and list directory entries
uint32_t read_path_table(uint32_t path_table_location, uint32_t path_table_size) {
    //fseek(iso_file, path_table_location, SEEK_SET);  // Go to the path table location
	uint32_t path_table_location_dir = 0;
	uint32_t root_path_table_location_dir = 0;
	uint32_t parent_path_table_location_dir = 0;
	char file_name[256];
    uint8_t *path_table_data = (uint8_t *)malloc(path_table_size*ATAPI_SECTOR_SIZE);
    if (path_table_data == NULL) {
        printf("Memory allocation failed for path table");
        return path_table_location_dir;
    }
    memset(file_name, 0, 256);
    // Read the entire path table into memory
	atapi_read(path_table_location, path_table_data, path_table_size); 
    // Iterate through the path table entries
    uint32_t current_offset = 0;
	uint32_t data_length = ATAPI_SECTOR_SIZE;
	//uint32_t data_sector_count = 1;
	//uint32_t sector_count = 0;
	uint32_t bytes_read = 0;
	uint32_t dir_offset = path_table_location;
	uint8_t has_er = 0;
    while (current_offset < path_table_size) {
        PathTableEntry *table_entry = (PathTableEntry *)(path_table_data + current_offset);
        
        while ((table_entry->length == 0) && (current_offset < path_table_size)) {
			current_offset += table_entry->length;
			uint32_t aligned_offset = ((current_offset / ATAPI_SECTOR_SIZE) * ATAPI_SECTOR_SIZE);
			uint32_t padding_size = (ATAPI_SECTOR_SIZE - (current_offset - aligned_offset));
			
			current_offset += padding_size;//entry->length;
			bytes_read += padding_size;//sizeof(PathEntry);
			table_entry = (PathTableEntry *)(path_table_data + current_offset);
        }
		
		/*
		if (table_entry->length == 0)
		{
			break;
		}
		*/
		path_table_location_dir = table_entry->parent_dir;
		parent_path_table_location_dir = path_table_location_dir;
		PathEntry *entry_root = (PathEntry *)(path_table_data + current_offset);		
		if ((path_table_location_dir != 0) && (current_offset == 0) && (table_entry->name[0] == '\0')) {
			if (path_table_location_dir == path_table_location)	{
				while (path_table_location_dir == path_table_location)
				{
					current_offset += table_entry->length;
					table_entry = (PathTableEntry *)(path_table_data + current_offset);
					while ((table_entry->length == 0) && (current_offset < path_table_size)) {
						current_offset += table_entry->length;
						uint32_t aligned_offset = ((current_offset / ATAPI_SECTOR_SIZE) * ATAPI_SECTOR_SIZE);
						uint32_t padding_size = (ATAPI_SECTOR_SIZE - (current_offset - aligned_offset));
						
						current_offset += padding_size;//entry->length;
						bytes_read += padding_size;//sizeof(PathEntry);
						table_entry = (PathTableEntry *)(path_table_data + current_offset);
					}
					/*
					if (table_entry->length == 0) {
						break; // End of the table (length 0 indicates an empty entry)
					}
					*/
					path_table_location_dir = table_entry->parent_dir;
					data_length = entry_root->data_length;
					//data_sector_count = (entry_root->data_length / ATAPI_SECTOR_SIZE);
				}
				while (bytes_read < data_length)
				{
					PathEntry *entry = (PathEntry *)(path_table_data + current_offset);									
					while ((entry->length == 0) && (bytes_read < data_length)) {
						
						uint32_t aligned_offset = ((current_offset / ATAPI_SECTOR_SIZE) * ATAPI_SECTOR_SIZE);
						uint32_t padding_size = (ATAPI_SECTOR_SIZE - (current_offset - aligned_offset));
						if (current_offset > data_length) break;
						current_offset += padding_size;//entry->length;
						bytes_read += padding_size;//sizeof(PathEntry);
						
						entry = (PathEntry *)(path_table_data + current_offset);
						uint8_t* entry_ptr = (uint8_t*)entry;
						if ((entry_ptr[0] == 'E') && (entry_ptr[1] == 'R'))
						{
							bytes_read += entry_ptr[2];
							current_offset += entry_ptr[2];
							has_er = 1;
							break;
						}
						if ((entry->name[0] != 0x00) && (entry->name[0] != 0x01))
						{
							memset(file_name, 0, 256);
							translate_filename_iso(entry, file_name);
						}
						
					}
					
					if (current_offset > data_length) break;
					// Print the directory/file name	
					memset(file_name, 0, 256);
					translate_filename_iso(entry, file_name);
					if (entry->name[0] == 0x01) 
					{
						dir_offset = parent_path_table_location_dir;
						printf("Directory Entry Offset: 0x%X\n", dir_offset);
					}

					uint32_t loc_n = (uint32_t)((void*)entry->name);
					uint32_t loc_l = (uint32_t)((void*)entry);
					uint32_t sys_use_loc_rel = ((loc_n-loc_l)+entry->name_len);
					uint32_t sys_use_loc = (loc_l+sys_use_loc_rel);
					uint32_t sys_use_len = (entry->length-sys_use_loc_rel);
					uint8_t* sys_use = (uint8_t*)((void*)sys_use_loc);
					if (sys_use[0] == 0)
					{
						sys_use_loc_rel++;
						sys_use_loc = (loc_l+sys_use_loc_rel);
						sys_use = (uint8_t*)((void*)sys_use_loc);
					}
					//getch();
					//printf("%d\n", sys_use_loc_rel);
					//printf("0x%X%X\n", sys_use[0], sys_use[1]);
					uint8_t r_nm_l = 0;
					uint8_t name_len = 0;
					uint32_t rr_offset = 0;
					char nm[256];
					memset(nm, 0, 256);
					while (rr_offset < sys_use_len)
					{
						uint16_t rr_signature = UINT16(sys_use[rr_offset+0], sys_use[rr_offset+1]);
						rr_offset += 2;
						switch(rr_signature)
						{
							case UINT16('S','P'):
							{
								// susp signature
								//printf("SUSP detected.\n");								
								rr_offset += 3;
							}
							break;
							case UINT16('R','R'):
							{
								// rock ridge signature
								//printf("Rock Ridge detected.\n");
								rr_offset += 1;
							}
							break;
							case UINT16('N','M'):
							{
								// alternative name signature
								//printf("Name detected.\n");
								uint8_t name_length = sys_use[rr_offset+0];
								uint8_t name_flags = sys_use[rr_offset+1];
								if (name_length < 5) break;
								if (name_flags & 6) break;
								if (name_flags & ~1) break;
								name_len = name_length-5;
								if ((r_nm_l+name_len) >= 254) break;
								r_nm_l += name_len;
								//printf("0x%X\n", name_length);								
								memset(nm, 0, 256);
								memcpy(nm, &sys_use[rr_offset+3], name_len);
								nm[name_len] = '\0';
								//printf("%d: %s\n", name_len, nm);
								rr_offset += 1;
							}
							break;
							case UINT16('P','X'):
							{
								// posix attribute signature
								//printf("Posix detected.\n");
								rr_offset += 32;
							}
							break;
							case UINT16('P','N'):
							{
								// posix device signature
								//printf("Posix Device detected.\n");
								rr_offset += 16;
							}
							break;
							case UINT16('T','F'):
							{
								// timestamp signature
								//printf("Timestamp detected.\n");
								rr_offset += 8;
							}
							break;
							case UINT16('E','R'):
							{
								// iso9660 extension signature
								//printf("Extension detected.\n");
								uint8_t er_cnt = sys_use[rr_offset+0];
								rr_offset += er_cnt;
							}
							break;
							default:
							{
								//rr_offset += 1;
							}
							break;
						};
					}
					
					uint8_t* entry_ptr = (uint8_t*)entry;	
					if ((entry_ptr[0] == 'E') && (entry_ptr[1] == 'R'))
					{
						bytes_read += entry_ptr[2];
						current_offset += entry_ptr[2];
						has_er = 1;
						break;
					}
					if ((entry->name[0] != 0x00) && (entry->name[0] != 0x01) && (has_er == 0))
					{
						char file_type_s[16];
						uint8_t file_type = entry->flags_iso;
						int is_dir = 0;
						if (file_type & 0x02) is_dir = 1;
						if (is_dir == 1) strcpy(file_type_s, "Dir");
						else strcpy(file_type_s, "File");
						translate_filename_iso(entry, file_name);
						if (nm[0] != 0)
						{
							strcpy(file_name, nm);
						}
						printf("%s Name: '%s'", file_type_s, file_name);
						printf(" Length: %d", entry->length);
						printf(" Location: 0x%X", entry->location);
						printf(" Parent: 0x%X\n", parent_path_table_location_dir);						
						if (is_dir == 1) 
						{
							path_table_location_dir = entry->location;
							//read_path_table(path_table_location_dir, path_table_size);
						}
					}
					current_offset += entry->length;
					//if (sector_count < data_sector_count) sector_count++;
					bytes_read += entry->length;//sizeof(PathEntry);
				}	
				break;
			}
			else
			{
				root_path_table_location_dir = table_entry->parent_dir;
				printf("Path Table Entry Offset: 0x%X\n", table_entry->parent_dir);
				printf("Path Table Entry Root: 0x%X\n", path_table_location);
				break;
			}
		}
        // Move to the next entry (skip the current entry's length and name)
        current_offset += table_entry->length;
    }

    free(path_table_data);
	return root_path_table_location_dir;
}



// Function to read a directory entry
void read_directory_entry(uint32_t dir_entry_location) {
    //fseek(iso_file, dir_entry_location, SEEK_SET);
    
    uint8_t entry_header[ATAPI_SECTOR_SIZE];
    //fread(entry_header, 1, ATAPI_SECTOR_SIZE, iso_file);
	atapi_read(dir_entry_location, entry_header, 1); 
    
    // Loop through the directory entries
    uint32_t offset = 0;
	
    while (offset < ATAPI_SECTOR_SIZE) {
        DirectoryEntry *entry = (DirectoryEntry *)(entry_header + offset);

        if (entry->length == 0) {
            break;  // End of directory entries (empty entry)
        }
        
        // Print basic directory entry information
        printf("File/Directory Name: ");
        for (int i = 0; i < entry->name_length; i++) {
            putchar(entry->name[i]);
        }
        printf("\n");

        printf("File Size: %u bytes\n", entry->file_size);
        printf("Starting Sector (LBA): %u\n", entry->starting_sector);
        
        // If the entry represents a file, we can read the file data from its starting sector
        if (entry->file_flags == 0x02) {
            // File entry
            printf("This is a file. Reading file data...\n");

            // Read the file data (just an example, more code is needed for real extraction)
            //fseek(iso_file, entry->starting_sector * ATAPI_SECTOR_SIZE, SEEK_SET);
			uint32_t starting_sector = entry->starting_sector;
            uint8_t *file_data = (uint8_t *)malloc(entry->file_size);
            if (file_data == NULL) {
                printf("Error allocating memory for file data");
                return;
            }
            atapi_read(starting_sector, file_data, 1);
            //fread(file_data, 1, entry->file_size, iso_file);
            printf("File data read successfully (size: %u bytes).\n", entry->file_size);
            free(file_data);
        } else {
            // Directory entry (Not reading data in this case)
            printf("This is a directory. Skipping file data read.\n");
        }

        offset += entry->length;  // Move to the next directory entry
    }
}

// Recursive function to read and list the contents of a directory
void list_directory(uint32_t dir_entry_location, const char *parent_dir) {
    //fseek(iso_file, dir_entry_location, SEEK_SET);
	
    uint8_t entry_header[ATAPI_SECTOR_SIZE];
	atapi_read(dir_entry_location, entry_header, 1); 
    //fread(entry_header, 1, ATAPI_SECTOR_SIZE, iso_file);
    
    uint32_t offset = 0;
    while (offset < ATAPI_SECTOR_SIZE) {
        DirectoryEntry *entry = (DirectoryEntry *)(entry_header + offset);

        if (entry->length == 0) {
            break;  // End of directory entries
        }
        
        char file_name[256];
        strncpy(file_name, entry->name, entry->name_length);
        file_name[entry->name_length] = '\0'; // Null-terminate the file name
        
        // Print directory or file details
        printf("Found: %s\n", file_name);
        printf("File Size: %u bytes\n", entry->file_size);
        printf("Starting Sector (LBA): %u\n", entry->starting_sector);
        
        // Check if it's a file or directory
        if (entry->file_flags == 0x02) {  // It's a file
            printf("File entry found: %s\n", file_name);
            // You could extract the file here or print its contents
        } else if (entry->file_flags == 0x01) {  // It's a directory
            printf("Directory entry found: %s\n", file_name);
            
            // Call recursively to list contents of the subdirectory
            char new_dir_path[512];
            snprintf(new_dir_path, sizeof(new_dir_path), "%s/%s", parent_dir, file_name);
            list_directory(entry->starting_sector, new_dir_path);
        }

        offset += entry->length;  // Move to next entry
    }
}

int main(int argc, char *argv[])
{
	uint8_t is_file_address = 0;
	uint8_t f_argc = 0;
    if (init_atapi())
	{
		if (init_vd())
		{
			if (read_pvd())
			{
				uint32_t root_sector_dir;
				if (argc > 1)
				{
					if (argc > 2)
					{
						for(int i=1;i<argc;i++)
						{
							//root_sector_dir
							if (strcmp(argv[i], "-f") == 0)
							{
								is_file_address = 1;
								f_argc = i+1;
							}
						}
						if (is_file_address == 1)
						{
							root_sector_dir = atoh(argv[f_argc]);
						}
						else
						{
							return 0;
						}
					}
					else
					{
						root_sector_dir = atoh(argv[1]);
					}
				}
				else 
				{
					root_sector_dir = read_path_table(root_sector, pvd->path_table_size);
				}
				if (is_file_address == 1)
				{
					uint8_t *sector_buffer = (uint8_t *)malloc(ATAPI_SECTOR_SIZE);
					if (sector_buffer == NULL) {
						printf("Memory allocation failed for read sector");
						return 0;
					}
					memset(sector_buffer, 0, ATAPI_SECTOR_SIZE);
					atapi_read(root_sector_dir, sector_buffer, 1); 					
					dump_hex(sector_buffer, 256);
					free(sector_buffer);
				}
				else
				{
					read_path_table(root_sector_dir, pvd->path_table_size);
				}
			}
		} else {
			printf("Invalid Volume Descriptor.\n");
		}
	}
    return 0;
}
