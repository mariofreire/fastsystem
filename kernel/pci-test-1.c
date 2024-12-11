#include <stdio.h>
#include <stdint.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

unsigned char inb(unsigned short port)
{
    unsigned char r;
    __asm__ volatile   ( "inb %1, %0"
                   : "=a"(r)
                   : "Nd"(port)
                   : "memory");
    return r;
}

void outb(unsigned short port, unsigned char value)
{
    __asm__ volatile   ( "outb %0, %1"
				   : 
				   : "a"(value), 
				     "Nd"(port) 
				   : "memory");
}

unsigned short inw(unsigned short port)
{
    unsigned short r;
    __asm__ volatile   ( "inw %1, %0"
                   : "=a"(r)
                   : "Nd"(port)
                   : "memory");
    return r;
}

void outw(unsigned short port, unsigned short value)
{
    __asm__ volatile   ( "outw %0, %1"
				   : 
				   : "a"(value), 
				     "Nd"(port) 
				   : "memory");
}


unsigned long inl( unsigned short port )
{
  unsigned long r;
  __asm__ volatile    ("inl %1, %0\n"
				   : "=a"( r )
				   : "dN"( port ));
  return r;
}

void outl(unsigned short port, unsigned long value)
{
  __asm__ volatile ("outl %1, %0\n"
					:
					: "dN"(port),
					  "a"(value));
}


uint32_t read_pci_config(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset) {
    uint32_t address;
    uint32_t lbus = (uint32_t)bus;
    uint32_t lslot = (uint32_t)device;
    uint32_t lfunc = (uint32_t)func;
    uint32_t tmp = 0;

    address = (uint32_t)((lbus << 16) | (lslot << 11) |
                         (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));

    outl(PCI_CONFIG_ADDRESS, address);
    tmp = inl(PCI_CONFIG_DATA);

    return (tmp >> ((offset & 3) * 8));// & 0xFF;
}

int main() {
    uint8_t bus = 0;
    //uint8_t device = 0;
    uint8_t func = 0;
    uint8_t offset = 0x10; // BAR0 offset for EHCI

	for(int i=0;i<32;i++)
	{
		uint32_t bar0_value = read_pci_config(bus, i, func, offset);
		if ((bar0_value != 0) && (bar0_value != 0xFFFFFFFF))
		{
			printf("PCI Device %d: BAR0 Value: 0x%X\n", i, bar0_value);
		}
	}

    return 0;
}
