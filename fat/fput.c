#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <limits.h>
#include "fatlib.h"


int main(int argc, char *argv[])
{
    const char *image_name;
    const char *source_name;
    const char *destination_name;
    const char *final_name;
    FILE *source = NULL;
    uint64_t file_size;
    uint32_t required_clusters;
    uint32_t first_cluster;
    path_sub_t path;
    uint32_t directory_sector;
    found_file_t existing;
    uint8_t dos_name[11];
    bool file_exists_on_disk = false;
    uint16_t utf16_name[LFN_MAX_CHARS];
    size_t utf16_length;
    size_t lfn_count;
    size_t directory_entries_needed;

    if (argc != 4)
    {
        printf("FAT16/FAT32 File Put\n");
        printf("   Created by Mario Freire\n");
        printf("\n");
        printf("Usage: fput [image-file] [source-file-name] [destination-file-name]\n");
        printf("Example: fput harddisk.img loader loader\n");
        printf("\n");
        return 1;
    }

    image_name = argv[1];
    source_name = argv[2];
    destination_name = argv[3];

    source = fopen(source_name, "rb");

    if (!source)
    {
        fprintf(stderr, "Error: Cannot open source file: %s\n", source_name);
        return 1;
    }

    if (fseeko(source, 0, SEEK_END) != 0)
    {
        fprintf(stderr, "Error: Cannot seek source file.\n");
        fclose(source);
        return 1;
    }

    {
        off_t size = ftello(source);

        if (size < 0)
        {
            fprintf(stderr, "Error: Cannot determine source size.\n");
            fclose(source);
            return 1;
        }

        file_size = (uint64_t)size;
    }

    rewind(source);

    if (file_size > UINT32_MAX)
    {
        fprintf(stderr, "Error: FAT directory entries support files up to 4 GiB.\n");
        fclose(source);
        return 1;
    }

    if (!initimagedisk(image_name))
    {
        fprintf(stderr, "Error: Cannot initialize disk image.\n");
        fclose(source);
        return 1;
    }

    if (!hasactive())
    {
        fprintf(stderr, "Error: No active FAT partition found.\n");
        uninitimagedisk();
        fclose(source);
        return 1;
    }

    if (!isfattype())
    {
        fprintf(stderr, "Error: Partition is not FAT16/FAT32.\n");
        uninitimagedisk();
        fclose(source);
        return 1;
    }

    if (!loadfat())
    {
        fprintf(stderr, "Error: Invalid FAT boot sector.\n");
        uninitimagedisk();
        fclose(source);
        return 1;
    }

    path = getpath(destination_name);

    if (path.pathcount == 0)
    {
        fprintf(stderr, "Error: Invalid destination filename.\n");
        uninitimagedisk();
        fclose(source);
        return 1;
    }

    final_name = path.path[path.pathcount - 1].path;

    utf16_length = utf8_to_utf16(final_name, utf16_name, LFN_MAX_CHARS);

    if (utf16_length == 0)
    {
        fprintf(stderr, "Error: Invalid UTF-8 filename.\n");
        uninitimagedisk();
        fclose(source);
        return 1;
    }

    if (utf16_length > LFN_MAX_CHARS)
    {
        fprintf(stderr, "Error: Filename is longer than 255 UTF-16 characters.\n");
        uninitimagedisk();
        fclose(source);
        return 1;
    }

    lfn_count = lfn_entry_count(utf16_length);

    if (lfn_count == 0 || lfn_count > LFN_MAX_ENTRIES)
    {
        fprintf(stderr, "Error: Invalid LFN length.\n");
        uninitimagedisk();
        fclose(source);
        return 1;
    }

    directory_entries_needed = lfn_count + 1;

    strfilenamedos(final_name, dos_name);

    if (path.pathcount > 1)
    {
        if (!find_path_directory(&path, path.pathcount - 1, &directory_sector))
        {
            fprintf(stderr, "Error: Destination directory not found.\n");
            uninitimagedisk();
            fclose(source);
            return 1;
        }
    }
    else
    {
        directory_sector = getrootdirsectorstart();
    }

    if (directory_sector == 0)
    {
        fprintf(stderr, "Error: Invalid destination directory sector.\n");
        uninitimagedisk();
        fclose(source);
        return 1;
    }

    if (findfileinsectorfilenumber(directory_sector, final_name, &existing))
        file_exists_on_disk = true;

    required_clusters = (uint32_t)neededcluster(file_size);

    if (file_size != 0 && required_clusters == 0)
    {
        fprintf(stderr, "Error: Cannot determine required clusters.\n");
        uninitimagedisk();
        fclose(source);
        return 1;
    }

    first_cluster = 0;

    if (file_exists_on_disk)
    {
        first_cluster = getfilefirstcluster(&existing.entry);

        if (existing.entry.size != 0 && first_cluster < 2)
        {
            fprintf(stderr, "Error: Existing file has an invalid cluster chain.\n");
            fclose(source);
            uninitimagedisk();
            return 1;
        }

        if (existing.entry.size == 0)
            first_cluster = 0;

        if (!resizechain(&first_cluster, required_clusters))
        {
            fprintf(stderr, "Error: Cannot resize existing file chain.\n");
            fclose(source);
            uninitimagedisk();
            return 1;
        }

        rewind(source);

        if (!write_file_data(source, file_size, first_cluster))
        {
            fprintf(stderr, "Error: Failed to write file data.\n");
            fclose(source);
            uninitimagedisk();
            return 1;
        }

        {
            uint8_t buffer[SECTORSIZE];
            file_entry_t entry;

            if (!readsector(existing.sector, buffer))
            {
                fprintf(stderr, "Error: Cannot read existing directory entry.\n");
                fclose(source);
                uninitimagedisk();
                return 1;
            }

            memcpy(&entry, buffer + existing.offset, sizeof(entry));
            entry.size = (uint32_t)file_size;

            if (isfat32type())
                entry.first_cluster_hi = (uint16_t)(first_cluster >> 16);
            else
                entry.first_cluster_hi = 0;

            entry.first_cluster_lo = (uint16_t)(first_cluster & 0xFFFF);
            set_current_datetime(&entry);

            memcpy(buffer + existing.offset, &entry, sizeof(entry));

            if (!writesector(existing.sector, buffer))
            {
                fprintf(stderr, "Error: Failed to update directory entry.\n");
                fclose(source);
                uninitimagedisk();
                return 1;
            }
        }
    }
    else
    {
        if (required_clusters != 0)
        {
            first_cluster = (uint32_t)allocchain(required_clusters);

            if (first_cluster == 0)
            {
                fprintf(stderr, "Error: Not enough free clusters.\n");
                uninitimagedisk();
                fclose(source);
                return 1;
            }
        }

        rewind(source);

        if (!write_file_data(source, file_size, first_cluster))
        {
            if (first_cluster != 0)
                freechain(first_cluster);

            fprintf(stderr, "Error: Failed to write file data.\n");
            fclose(source);
            uninitimagedisk();
            return 1;
        }

        {
            uint32_t free_sector = 0;
            uint32_t free_offset = 0;

            if (isfat16type() && directory_sector == getrootdirsector())
            {
                if (!findfreeslots_root_fat16((uint32_t)directory_entries_needed, &free_sector, &free_offset))
                {
                    if (first_cluster != 0)
                        freechain(first_cluster);

                    fprintf(stderr, "Error: Root directory does not have enough contiguous entries.\n");
                    fclose(source);
                    uninitimagedisk();
                    return 1;
                }
            }
            else
            {
                uint32_t directory_cluster = getclusterfromsector(directory_sector);

                if (isfat32type() && directory_sector == getrootdirsectorstart())
                    directory_cluster = getrootdircluster();

                if (directory_cluster < 2)
                {
                    if (first_cluster != 0)
                        freechain(first_cluster);

                    fprintf(stderr, "Error: Invalid destination directory cluster.\n");
                    fclose(source);
                    uninitimagedisk();
                    return 1;
                }

                if (!findfreeslots(directory_cluster, (uint32_t)directory_entries_needed, &free_sector, &free_offset))
                {
                    if (first_cluster != 0)
                        freechain(first_cluster);

                    fprintf(stderr, "Error: No contiguous directory space for LFN.\n");
                    fclose(source);
                    uninitimagedisk();
                    return 1;
                }
            }

            if (!write_file_directory_entries(free_sector, free_offset, final_name, dos_name, first_cluster, (uint32_t)file_size))
            {
                if (first_cluster != 0)
                    freechain(first_cluster);

                fprintf(stderr, "Error: Cannot write LFN directory entries.\n");
                fclose(source);
                uninitimagedisk();
                return 1;
            }
        }
    }

    printf("Put file successfully.\n");

    fclose(source);
    uninitimagedisk();

    return 0;
}
