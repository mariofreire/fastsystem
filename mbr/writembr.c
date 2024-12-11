#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int fileexists(char *filename)
{
	FILE *fp;
	fp = fopen(filename, "rb");
	if (!fp) return 0;
	fclose(fp);
	return 1;
}

int filesize(char *filename)
{
	FILE *fp;
	int sz,oldpos;
	fp = fopen(filename, "rb");
	if (!fp) return 0;
	oldpos = ftell(fp);
	fseek(fp, 0, SEEK_END);
	sz = ftell(fp);
	fseek(fp, oldpos, SEEK_SET);
	fclose(fp);
	return sz;
}

int updatembr(char *imagefilename, char *bootfilename)
{
	FILE *boot_fp, *image_fp;
	int boot_sz = filesize(bootfilename);
	int image_sz = filesize(imagefilename);
	unsigned char mbr[512];
	unsigned char bootstrap[436];
	int i;
	if (boot_sz != 512)
	{
		return 0;
	}
	else
	{
		if (image_sz > 512)
		{
			boot_fp = fopen(bootfilename, "rb");
			if (!boot_fp) return 0;
			fread(mbr, 512, 1, boot_fp);
			fclose(boot_fp);
			
			for(i=0;i<436;i++)
			{
				bootstrap[i] = mbr[i];
			}
			
			image_fp = fopen(imagefilename, "r+b");
			if (!image_fp) return 0;
			fseek(image_fp, 0, SEEK_SET);
			fwrite(bootstrap, 436, 1, image_fp);
			fseek(image_fp, 0, SEEK_SET);
			fclose(image_fp);
		}
		else
		{
			return 0;
		}
	}
	return 1;
}

int main(int argc, char *argv[])
{
	char imagefilename[256];
	char bootfilename[256];
	if (argc > 2)
	{
		strcpy(imagefilename, argv[1]);
		strcpy(bootfilename, argv[2]);
		if (fileexists(imagefilename))
		{
			if (fileexists(bootfilename))
			{
				if (updatembr(imagefilename, bootfilename))
				{
					printf("Writted %d bytes in MBR of the image file.\n", filesize(bootfilename));
					printf("Recorded Sector 1 successfully.\n");
				}
			}
			else
			{
				printf("Cannot find boot file '%s'.\n", bootfilename);
			}
		}
		else
		{
			printf("Cannot find image file '%s'.\n", imagefilename);			
		}
	}
	else
	{
		printf("Write Boot Sector To MBR\n");
		printf("   Created by Mario Freire\n");
		printf("\n");
		printf("Usage: writembr [image-file] [boot-sector-file]\n");
		printf("Example: writembr harddisk.img boot.bin\n");
		printf("\n");
	}
	return 0;
}