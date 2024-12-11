#include <iostream>
#include <cstdint>
#include <unistd.h> // For sleep() if needed

// PCI configuration space access functions (implement these based on your system)
uint32_t read_pci_config(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
void write_pci_config(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t value);

// Define PCI class code for EHCI
const uint8_t PCI_CLASS_SERIAL_BUS = 0x0C;
const uint8_t PCI_SUBCLASS_USB_EHCI = 0x03;

// Define EHCI register offsets and bitmasks (example values; adjust as needed)
const uint32_t EHCI_USBCMD = 0x00;
const uint32_t EHCI_USBCMD_RUN = 0x00000001;

void init_ehci(uint8_t bus, uint8_t device, uint8_t function) {
    uint16_t vendor_id = read_pci_config(bus, device, function, 0x00);
    uint16_t device_id = read_pci_config(bus, device, function, 0x02);
    uint8_t class_code = read_pci_config(bus, device, function, 0x0B);
    uint8_t subclass = read_pci_config(bus, device, function, 0x0A);

    if (class_code == PCI_CLASS_SERIAL_BUS && subclass == PCI_SUBCLASS_USB_EHCI) {
        // Read BAR0
        uint32_t bar0 = read_pci_config(bus, device, function, 0x10) & 0xFFFFFFF0;

        // Map BAR0 to your address space (this is highly system-dependent)
        volatile uint32_t* ehci_base = reinterpret_cast<volatile uint32_t*>(bar0);

        // Initialize EHCI controller
        ehci_base[EHCI_USBCMD / sizeof(uint32_t)] |= EHCI_USBCMD_RUN;

        std::cout << "EHCI Controller initialized at BAR0: " << std::hex << bar0 << std::endl;
    } else {
        std::cerr << "No EHCI Controller found." << std::endl;
    }
}

int main() {
    // Example bus, device, and function numbers; adjust as needed
    const uint8_t bus = 0x00;
    const uint8_t device = 0x1D;
    const uint8_t function = 0x00;

    init_ehci(bus, device, function);
    return 0;
}
