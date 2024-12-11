#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <libusb-1.0/libusb.h>

#define USB_REQUEST_TYPE_OUT 0x00
#define USB_REQUEST_TYPE_IN  0x80

// USB Command Definitions
#define USB_CMD_GET_MAX_LUN   0xFE
#define USB_CMD_BULK_RESET    0xFF

// USB Storage Command Definitions
#define USB_STORAGE_CMD_READ_10 0x28
#define USB_STORAGE_CMD_WRITE_10 0x2A

// USB Endpoint Definitions
#define USB_EP_IN  0x80
#define USB_EP_OUT 0x00

#define MASS_STORAGE_MAX_LUN 1

typedef struct {
    uint32_t base_address;
} ehci_controller_t;

typedef struct {
    uint8_t  opcode;
    uint8_t  flags;
    uint8_t  lba[4];  // Logical Block Addressing
    uint8_t  reserved;
    uint8_t  length[2];  // Transfer length
    uint8_t  control;
} scsi_command_t;

int usb_bulk_transfer(libusb_device_handle *dev_handle, unsigned char endpoint, unsigned char *data, int length) {
    int transferred;
    int result;

    // Perform a bulk transfer
    result = libusb_bulk_transfer(dev_handle, endpoint, data, length, &transferred, 5000); // 5000 ms timeout

    if (result < 0) {
        // Handle error
        fprintf(stderr, "Bulk transfer failed: %s\n", libusb_error_name(result));
        return -1;
    }

    return transferred;
}

void prepare_mass_storage(ehci_controller_t *ehci) {
    // Send a Get Max LUN command to the USB mass storage device
    uint8_t max_lun;
    if (usb_control_transfer(ehci, USB_REQUEST_TYPE_IN, USB_CMD_GET_MAX_LUN, 0, 0, &max_lun, sizeof(max_lun)) != 0) {
        printf("Failed to get max LUN.\n");
        return;
    }
    printf("Max LUN: %d\n", max_lun);
    
    // Assuming LUN 0 for simplicity
    // More initialization steps may be needed based on your device's requirements
}

int usb_control_transfer(libusb_device_handle *dev_handle,
                         uint8_t bmRequestType, uint8_t bRequest,
                         uint16_t wValue, uint16_t wIndex,
                         unsigned char *data, uint16_t wLength,
                         unsigned int timeout) {
    int transferred;
    int result;

    // Perform a control transfer
    result = libusb_control_transfer(dev_handle,
                                     bmRequestType,
                                     bRequest,
                                     wValue,
                                     wIndex,
                                     data,
                                     wLength,
                                     timeout);

    if (result < 0) {
        // Handle error
        fprintf(stderr, "Control transfer failed: %s\n", libusb_error_name(result));
        return -1;
    }

    return result; // Number of bytes transferred or error code
}

int usb_control_transfer(ehci_controller_t *ehci, uint8_t request_type, uint8_t request, uint16_t value, uint16_t index, uint8_t *data, uint16_t length) {
    // Placeholder for USB control transfer implementation
    // This should set up and perform the control transfer
    // This is a simplified version and should be replaced with actual USB control transfer handling code
    printf("Control Transfer: Request Type: 0x%02X, Request: 0x%02X, Value: 0x%04X, Index: 0x%04X\n", request_type, request, value, index);
    return 0;
}

#define READ_10_COMMAND 0x28

void send_scsi_read_command(int device_handle, uint32_t lba, uint16_t sector_count) {
    unsigned char cbw[31]; // Command Block Wrapper (CBW) size for Bulk-Only Transport

    // Fill CBW structure
    memset(cbw, 0, sizeof(cbw));
    cbw[0] = 0x55; // Signature for CBW
    cbw[1] = 0x53; // Signature for CBW
    cbw[2] = 0x54; // Signature for CBW
    cbw[3] = 0x42; // Signature for CBW
    cbw[4] = 0;    // Tag
    cbw[8] = 0;    // Data Transfer Length
    cbw[12] = 0;   // Flags
    cbw[13] = 0;   // LUN
    cbw[14] = READ_10_COMMAND;
    cbw[15] = (lba >> 24) & 0xFF;
    cbw[16] = (lba >> 16) & 0xFF;
    cbw[17] = (lba >> 8) & 0xFF;
    cbw[18] = lba & 0xFF;
    cbw[19] = (sector_count >> 8) & 0xFF;
    cbw[20] = sector_count & 0xFF;
    
    // Send CBW to device
    usb_bulk_transfer(device_handle, cbw, sizeof(cbw));
}


void read_sector(int device_handle, uint32_t lba, unsigned char *buffer, size_t buffer_size) {
    send_scsi_read_command(device_handle, lba, 1);

    // Receive data from the device
    usb_bulk_transfer(device_handle, buffer, buffer_size);
}


void close_device(int device_handle) {
    // Close the USB device handle
}


libusb_device_handle *dev_handle = /* Assume this is already initialized */;
unsigned char buffer[256];
int length = sizeof(buffer);

// Example: Get Device Descriptor
int result = usb_control_transfer(dev_handle,
                                   LIBUSB_REQUEST_TYPE_STANDARD | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_IN,
                                   LIBUSB_REQUEST_GET_DESCRIPTOR,
                                   (LIBUSB_DT_DEVICE << 8), // Descriptor type and index
                                   0,
                                   buffer,
                                   length,
                                   5000); // 5000 ms timeout

if (result >= 0) {
    // Process the data in buffer
} else {
    // Handle the error
}

libusb_device_handle* open_device(libusb_context *ctx, uint16_t vendor_id, uint16_t product_id) {
    libusb_device_handle *handle = libusb_open_device_with_vid_pid(ctx, vendor_id, product_id);
    if (!handle) {
        fprintf(stderr, "Error: Unable to open USB device.\n");
        return NULL;
    }
    return handle;
}

void send_scsi_read_command(libusb_device_handle *handle, uint32_t lba, uint16_t sector_count) {
    unsigned char cbw[31] = {0}; // Command Block Wrapper (CBW) for Bulk-Only Transport

    // Set up CBW
    cbw[0] = 0x55; // Signature for CBW
    cbw[1] = 0x53;
    cbw[2] = 0x54;
    cbw[3] = 0x42;
    cbw[4] = 0;    // Tag (set to zero for simplicity)
    cbw[8] = 0;    // Data Transfer Length
    cbw[12] = 0;   // Flags (0 for data-in, 0x80 for data-out)
    cbw[13] = 0;   // Logical Unit Number (LUN)
    cbw[14] = 0x28; // SCSI READ (10) Command
    cbw[15] = (lba >> 24) & 0xFF; // LBA
    cbw[16] = (lba >> 16) & 0xFF;
    cbw[17] = (lba >> 8) & 0xFF;
    cbw[18] = lba & 0xFF;
    cbw[19] = (sector_count >> 8) & 0xFF; // Number of sectors to read
    cbw[20] = sector_count & 0xFF;

    int transferred;
    int result = libusb_bulk_transfer(handle, /* OUT endpoint */, cbw, sizeof(cbw), &transferred, 5000); // 5000 ms timeout
    if (result < 0) {
        fprintf(stderr, "Error sending CBW: %s\n", libusb_error_name(result));
    }
}

int read_sector(libusb_device_handle *handle, uint32_t lba, unsigned char *buffer, size_t buffer_size) {
    // Send SCSI READ (10) command
    send_scsi_read_command(handle, lba, 1);

    // Read data (assuming IN endpoint is 0x81, adjust if necessary)
    int transferred;
    int result = libusb_bulk_transfer(handle, /* IN endpoint */, buffer, buffer_size, &transferred, 5000); // 5000 ms timeout
    if (result < 0) {
        fprintf(stderr, "Error receiving data: %s\n", libusb_error_name(result));
        return -1;
    }

    return transferred;
}

void close_device(libusb_device_handle *handle) {
    libusb_close(handle);
}

void cleanup(libusb_context *ctx) {
    libusb_exit(ctx);
}

int main() {
    libusb_context *ctx;
    libusb_init(&ctx);

    libusb_device_handle *handle = open_device(ctx, /* Vendor ID */, /* Product ID */);
    if (!handle) {
        libusb_exit(ctx);
        return -1;
    }

    unsigned char buffer[512]; // Buffer to read data into
    int bytes_read = read_sector(handle, /* LBA */, buffer, sizeof(buffer));
    if (bytes_read > 0) {
        // Process data in buffer
        printf("Read %d bytes\n", bytes_read);
    }

    close_device(handle);
    cleanup(ctx);
    return 0;
}

