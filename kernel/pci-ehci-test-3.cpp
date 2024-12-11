#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "pci.h"

// Function to find and initialize EHCI controller
void initialize_ehci_controller() {
    // Iterate over PCI devices
    for (unsigned char bus = 0; bus < 256; ++bus) {
        for (unsigned char slot = 0; slot < 32; ++slot) {
            for (unsigned char func = 0; func < 8; ++func) {
                uint16_t vendor_id = pci_read_word(bus, slot, func, 0x00);
                uint16_t device_id = pci_read_word(bus, slot, func, 0x02);
                uint8_t class_code = pci_read_byte(bus, slot, func, 0x0A);
                uint8_t subclass_code = pci_read_byte(bus, slot, func, 0x0B);

                // Check if device is EHCI controller
                if (class_code == 0x0C && subclass_code == 0x03) {
                    // Read BAR0 for base address
                    uint32_t bar0 = pci_read_long(bus, slot, func, 0x10) & ~0xF; // Mask off the flag bits

                    printf("EHCI Controller found: Vendor ID = 0x%04X, Device ID = 0x%04X\n", vendor_id, device_id);
                    printf("EHCI Base Address Register 0: 0x%08X\n", bar0);

                    // Map BAR0 address
                    usb_ehci_op_regs_address = bar0;
                    usb_ehci_op_regs = (usb_ehci_op_regs_t *)usb_ehci_op_regs_address;

                    // Initialize EHCI Controller
                    if (usb_ehci_op_regs) {
                        usb_ehci_op_regs->usb_cmd = 0x00000001; // Example command to reset controller
                        printf("EHCI Controller initialized.\n");

                        // Example: Set up and read sectors from a mass storage device
                        // (Implementation depends on the specific USB mass storage protocol)
                    }
                    return;
                }
            }
        }
    }
    printf("EHCI Controller not found.\n");
}

int main() {
    if (init_pci()) {
        initialize_ehci_controller();
    } else {
        printf("PCI Initialization failed.\n");
    }
    return 0;
}
