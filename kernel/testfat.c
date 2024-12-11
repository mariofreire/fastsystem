#include <stdio.h>
#include <stdint.h>



int main(int argc, char *argv[])
{
	uint32_t root_dir = 466;
	uint32_t cluster = 32949;
	uint32_t root_entries = 512;
	uint32_t bytes_per_sector = 512;
	uint32_t sector_per_cluster = 16;	
	uint32_t data_start;
	uint32_t root_count;	
	uint32_t sector;
	
	root_count = (32 * root_entries + bytes_per_sector - 1) / bytes_per_sector;	
	data_start = (root_dir + root_count);
	sector = (data_start + (cluster-2) * sector_per_cluster);	
	
	printf("Root Start: %08X\n", root_dir);
	printf("Cluster: %08X\n", cluster);
	printf("Bytes per sector: %d\n", bytes_per_sector);
	printf("Root entries count: %d\n", root_entries);
	printf("Root count: %d\n", root_count);
	printf("Data Start: %d\n", data_start);
	printf("Sector per cluster: %d\n", sector_per_cluster);
	printf("Data Sector: %08X\n", sector);	
	return 0;
}
