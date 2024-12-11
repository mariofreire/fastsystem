#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <dos.h>

#define IDE_DATA_PORT 0x1F0
#define IDE_ERROR_PORT 0x1F1
#define IDE_SECTOR_COUNT_PORT 0x1F2
#define IDE_SECTOR_NUMBER_PORT 0x1F3
#define IDE_CYLINDER_LOW_PORT 0x1F4
#define IDE_CYLINDER_HIGH_PORT 0x1F5
#define IDE_DRIVE_HEAD_PORT 0x1F6
#define IDE_STATUS_PORT 0x1F7

void main() {
    unsigned char data;
    unsigned char error;
    unsigned char sectorCount;
    unsigned char sectorNumber;
    unsigned char cylinderLow;
    unsigned char cylinderHigh;
    unsigned char driveHead;
    unsigned char status;

    // Select the drive
    driveHead = 0xA0; // Master drive
    outportb(IDE_DRIVE_HEAD_PORT, driveHead);

    // Set the sector count
    sectorCount = 1;
    outportb(IDE_SECTOR_COUNT_PORT, sectorCount);

    // Set the sector number
    sectorNumber = 1;
    outportb(IDE_SECTOR_NUMBER_PORT, sectorNumber);

    // Set the cylinder low
    cylinderLow = 0;
    outportb(IDE_CYLINDER_LOW_PORT, cylinderLow);

    // Set the cylinder high
    cylinderHigh = 0;
    outportb(IDE_CYLINDER_HIGH_PORT, cylinderHigh);

    // Send the identify command
    outportb(IDE_STATUS_PORT, 0xEC);

    // Wait for the drive to be ready
    do {
        status = inportb(IDE_STATUS_PORT);
    } while ((status & 0x80) != 0);

    // Read the data
    for (int i = 0; i < 256; i++) {
        data = inportb(IDE_DATA_PORT);
        printf("%c", data);
    }
}
