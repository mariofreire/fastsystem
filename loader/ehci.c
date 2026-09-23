// Fast System Kernel Loader - Enhanced Host Controller Interface
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2024 DSP Interactive.

#include <stdarg.h>
#include "enum.h"
#include "funcbase.h"
#include "ecs.h"
#include "pci.h"
#include "ehci.h"

void enable_interrupt(void);
void disable_interrupt(void);

ehci_qh_t ehci_async_head __attribute__((aligned(32))) ENHANCED_DATA_SECTION;
ehci_qh_t ehci_control_qh __attribute__((aligned(32))) ENHANCED_DATA_SECTION;
ehci_qtd_t ehci_qtd_setup __attribute__((aligned(32))) ENHANCED_DATA_SECTION;
ehci_qtd_t ehci_qtd_data __attribute__((aligned(32))) ENHANCED_DATA_SECTION;
ehci_qtd_t ehci_qtd_status __attribute__((aligned(32))) ENHANCED_DATA_SECTION;
usb_setup_t ehci_setup_packet __attribute__((aligned(16))) ENHANCED_DATA_SECTION;

unsigned char *usb_ehci_ptr  = (unsigned char *)USB_EHCI_ADDRESS;
usb_ehci_t *usb_ehci ENHANCED_DATA_SECTION;
volatile unsigned long ehci_base ENHANCED_DATA_SECTION = 0;
volatile unsigned long ehci_op ENHANCED_DATA_SECTION = 0;
unsigned char ehci_ports ENHANCED_DATA_SECTION = 0;
unsigned char ehci_available_ports ENHANCED_DATA_SECTION = 0;
unsigned char ehci_available_port[15] ENHANCED_DATA_SECTION;

volatile usb_ehci_cap_t *ehci_cap ENHANCED_DATA_SECTION;
volatile usb_ehci_op_regs_t *ehci_op_regs ENHANCED_DATA_SECTION;

int ehci_initialized ENHANCED_DATA_SECTION = 0;

usb_device_t usb_device[MAX_USB_DEVICES] ENHANCED_DATA_SECTION;
int usb_device_count ENHANCED_DATA_SECTION = 0;

/*
typedef struct 
{
    int class_code;
    int subclass_code;
    int protocol_code;
    const char *name;
} usb_device_type_t;

// USB Device and Interface Specification Mapping Table
static const usb_device_type_t usb_type_table[] = 
{
    {0x00, 0x00, 0x00, "Defined at Interface Level"},
    {0x01, USB_ANY, USB_ANY, "USB Audio Device"},
    {0x02, 0x02, USB_ANY, "USB CDC ACM Communication Device"},
    {0x02, USB_ANY, USB_ANY, "USB Communications Device"},
    {0x03, 0x01, 0x01, "USB HID Keyboard"},
    {0x03, 0x01, 0x02, "USB HID Mouse"},
    {0x03, USB_ANY, USB_ANY, "USB Human Interface Device (HID)"},
    {0x05, USB_ANY, USB_ANY, "USB Physical Interface Device"},
    {0x06, 0x01, 0x01, "USB Still Image Capture Device (PTP/MTP)"},
    {0x06, USB_ANY, USB_ANY, "USB Imaging Device"},
    {0x07, USB_ANY, USB_ANY, "USB Printer Device"},
    {0x08, 0x06, 0x50, "USB Mass Storage Class"}, // SCSI transparent command set, Bulk-Only Transport
    {0x08, 0x06, USB_ANY, "USB Mass Storage (SCSI)"},
    {0x08, USB_ANY, USB_ANY, "USB Mass Storage Device"},
    {0x09, USB_ANY, USB_ANY, "USB Hub"},
    {0x0A, USB_ANY, USB_ANY, "USB CDC-Data Device"},
    {0x0B, USB_ANY, USB_ANY, "USB Smart Card Reader"},
    {0x0D, USB_ANY, USB_ANY, "USB Content Security Device"},
    {0x0E, USB_ANY, USB_ANY, "USB Video Device (UVC)"},
    {0x0F, USB_ANY, USB_ANY, "USB Personal Healthcare Device"},
    {0xDC, USB_ANY, USB_ANY, "USB Diagnostic Device"},
    {0xE0, 0x01, 0x01, "USB Bluetooth Adapter"},
    {0xE0, USB_ANY, USB_ANY, "USB Wireless Controller"},
    {0xEF, 0x02, 0x01, "USB RNDIS Network Device"},
    {0xEF, USB_ANY, USB_ANY, "USB Miscellaneous Device"},
    {0xFE, USB_ANY, USB_ANY, "USB Application Specific Device"},
    {0xFF, USB_ANY, USB_ANY, "USB Vendor Specific Device"}
};

#define USB_TYPE_TABLE_SIZE (sizeof(usb_type_table) / sizeof(usb_type_table[0]))

ENHANCED_CODE_SECTION const char* ehci_get_usb_device_type(unsigned char class_code, unsigned char subclass_code, unsigned char protocol_code)
{
    // 1. Hierarchical Match: Check for exact class, subclass, and protocol match
    for (size_t i = 0; i < USB_TYPE_TABLE_SIZE; i++) 
    {
        if (usb_type_table[i].class_code == class_code &&
            usb_type_table[i].subclass_code == subclass_code &&
            usb_type_table[i].protocol_code == protocol_code) 
        {
            return usb_type_table[i].name;
        }
    }

    // 2. Fallback Match: Check for matching class and subclass, ignoring protocol
    for (size_t i = 0; i < USB_TYPE_TABLE_SIZE; i++) 
    {
        if (usb_type_table[i].class_code == class_code &&
            usb_type_table[i].subclass_code == subclass_code &&
            usb_type_table[i].protocol_code == USB_ANY) 
        {
            return usb_type_table[i].name;
        }
    }

    // 3. Fallback Match: Check for matching class only
    for (size_t i = 0; i < USB_TYPE_TABLE_SIZE; i++) 
    {
        if (usb_type_table[i].class_code == class_code &&
            usb_type_table[i].subclass_code == USB_ANY &&
            usb_type_table[i].protocol_code == USB_ANY) 
        {
            return usb_type_table[i].name;
        }
    }

    return "Unknown USB Device";
}
*/

ENHANCED_CODE_SECTION void irq_restore(unsigned long flags)
{
    if (flags & 0x00000200U)
        enable_interrupt();
    else
        disable_interrupt();
}

ENHANCED_CODE_SECTION void delay_us(unsigned long us)
{
    volatile unsigned long i;

    while (us--)
    {
        for (i = 0; i < 100; ++i)
            __asm__ volatile ("nop");
    }
}

ENHANCED_CODE_SECTION void delay_ms(unsigned long ms)
{
    while (ms--)
        delay_us(1000);
}

ENHANCED_CODE_SECTION void flush_cpu_cache(void)
{
    __asm__ volatile (
        "wbinvd"
        :
        :
        : "memory"
    );
}

ENHANCED_CODE_SECTION int ehci_reset(void)
{
	unsigned long cmd;
	unsigned long sts;
    unsigned long j;
	ehci_op_regs->usb_intr = 0;
	cmd = ehci_op_regs->usb_cmd;
	cmd &= ~CMD_RUN;
	cmd &= ~CMD_ASE;
	cmd &= ~CMD_PSE;
	ehci_op_regs->usb_cmd = cmd;
	for (j=0;j<1000;++j)
	{
    	sts = ehci_op_regs->usb_sts;
	
    	if (sts & STS_HCHALTED)
        	break;
	
    	delay_ms(1);
	}
	if (!(sts & STS_HCHALTED)) return -1;
	cmd = ehci_op_regs->usb_cmd;
	cmd |= CMD_RESET;
	ehci_op_regs->usb_cmd = cmd;
	for (j=0;j<1000;++j)
	{
    	cmd = ehci_op_regs->usb_cmd;
	
    	if (!(cmd & CMD_RESET))
        	break;
	
    	delay_ms(1);
	}
	if (cmd & CMD_RESET) return -2;								
	ehci_op_regs->asynclistaddr = 0;
	ehci_op_regs->periodiclistbase = 0;
	ehci_op_regs->usb_sts = 0x3F;
	return 0;
}

ENHANCED_CODE_SECTION int ehci_start(void)
{
	unsigned long j;
	unsigned long cmd;
	unsigned long sts;
	cmd = ehci_op_regs->usb_cmd;
	cmd &= ~(CMD_ASE | CMD_PSE);
	cmd |= CMD_RUN;
	ehci_op_regs->usb_cmd = cmd;
	for (j=0;j<1000;++j)
	{
    	sts = ehci_op_regs->usb_sts;
    	if (!(sts & STS_HCHALTED)) return 0;
    	delay_ms(1);
	}
	return -1;
}

ENHANCED_CODE_SECTION int ehci_take_ownership(void)
{
    unsigned long i;
    unsigned long v;

    v = ehci_op_regs->configflag;
	ehci_op_regs->configflag = 1;

    for (i = 0; i < 1000; ++i)
    {
        v = ehci_op_regs->configflag;

        if (v & 1)
            break;

        delay_ms(1);
    }

    return (v & 1) ? 0 : -1;
}

ENHANCED_CODE_SECTION unsigned long ehci_port_read(unsigned char p)
{
	return ehci_op_regs->portsc[p];
}

ENHANCED_CODE_SECTION void ehci_port_write(unsigned char p, unsigned long v)
{
	ehci_op_regs->portsc[p] = v;
}

ENHANCED_CODE_SECTION int ehci_reset_port(unsigned char p)
{
    unsigned long v;
    unsigned long i;

    v =
        ehci_port_read(p);

    if (!(v & PORT_CCS))
        return -1;

    v &= ~PORTSC_W1C;
    v |= PORT_PR;

    ehci_port_write(p, v);

    delay_ms(60);

    v = ehci_port_read(p);
    v &= ~PORT_PR;
    v &= ~PORTSC_W1C;

    ehci_port_write(p, v);

    for (i = 0; i < 1000; ++i)
    {
        v = ehci_port_read(p);

        if (!(v & PORT_PR))
            break;

        delay_ms(1);
    }

    delay_ms(10);

    v = ehci_port_read(p);

    if (v & PORT_OWNER) return -2;
    if (!(v & PORT_CCS)) return -3;
    if (!(v & PORT_PED)) return -4;

    return 0;
}


ENHANCED_CODE_SECTION int init_usb_ehci(void)
{
	if (init_pci())
	{
		if (pci_count > 0)
		{	
			for(int i=0;i<pci_count;i++) 
			{
				if (pci_device[i].pci.vendor != 0xFFFF)
				{
					if ((pci_device[i].pci.class == 0x0C) && (pci_device[i].pci.subclass == 0x03))
					{
						if (pci_device[i].pci.progif == 0x20)
						{
							if (pci_device[i].pci.bar[0] != 0)
							{
                    			unsigned long bar;
                    			unsigned short cmd;
								bar = pci_device[i].pci.bar[0];
								if (bar & 1) continue;
                    			bar &= 0xFFFFFFF0U;
                    			if (!bar) continue;
                    			ehci_base = bar;
                    			ehci_cap = (usb_ehci_cap_t*)ehci_base;
								if (ehci_cap->version == 0x100)
								{
									usb_ehci_ptr = (unsigned char *)USB_EHCI_ADDRESS;
									usb_ehci = (usb_ehci_t*)usb_ehci_ptr;
									strcpy(usb_ehci->signature, "EHCI");
									usb_ehci->version = 1;
									usb_ehci->address = ehci_base;
									cmd = pci_read_word(pci_device[i].bus, pci_device[i].slot, pci_device[i].function, 0x04);
									cmd |= 0x0002;
									cmd |= 0x0004;
									pci_write_word(pci_device[i].bus, pci_device[i].slot, pci_device[i].function, 0x04, cmd);
									unsigned char cap_length = ehci_cap->cap_length;
									ehci_op = ehci_base + cap_length;
									unsigned long hc_sparams = ehci_cap->sparams;
									ehci_ports = (hc_sparams & 0x0F);
									usb_ehci->ports = ehci_ports;
									if (ehci_ports == 0) return 0;
									ehci_op_regs = (usb_ehci_op_regs_t*)(void*)ehci_op;						
									if (ehci_reset() != 0) return 0;  
									if (ehci_start() != 0) return 0;  
									if (ehci_take_ownership() != 0) return 0;    								
									return 1;
								}
							}
						}
					}
				}
			}
		}
	}
	return 0;
}

ENHANCED_CODE_SECTION void qtd_set_buffer(
    ehci_qtd_t *qtd,
    unsigned long address)
{
    unsigned long page;
    int i;

    qtd->buffer[0] = address;
    page = address & 0xFFFFF000U;

    for (i = 1; i < 5; ++i)
    {
        page += 0x1000;
        qtd->buffer[i] = page;
    }
}

ENHANCED_CODE_SECTION int qtd_error(unsigned long token)
{
    return token &
        (QTD_HALTED |
         QTD_DBE |
         QTD_BABBLE |
         QTD_XACTERR |
         QTD_MISSED);
}

ENHANCED_CODE_SECTION void ehci_build_async(void)
{
    memset(&ehci_async_head, 0, sizeof(ehci_async_head));
    memset(&ehci_control_qh, 0, sizeof(ehci_control_qh));

    ehci_async_head.horizontal =
        ((unsigned long)&ehci_control_qh &
         EHCI_PTR_MASK) |
        EHCI_LINK_QH;
    ehci_async_head.ep_char = QH_HEAD;
    ehci_async_head.ep_cap = 0;
    ehci_async_head.current = 0;
    ehci_async_head.next = EHCI_LINK_TERMINATE;
    ehci_async_head.alt_next = EHCI_LINK_TERMINATE;
    ehci_async_head.token = 0;

    ehci_control_qh.horizontal =
        ((unsigned long)&ehci_async_head &
         EHCI_PTR_MASK) |
        EHCI_LINK_QH;
    ehci_control_qh.ep_char =
        QH_DTC |
        QH_SPEED_HIGH |
        (64U << 16);
    ehci_control_qh.ep_cap = 0;
    ehci_control_qh.current = 0;
    ehci_control_qh.next = EHCI_LINK_TERMINATE;
    ehci_control_qh.alt_next = EHCI_LINK_TERMINATE;
    ehci_control_qh.token = 0;
}

ENHANCED_CODE_SECTION int ehci_start_async(void)
{
    unsigned long cmd;
    unsigned long sts;
    unsigned long i;

    flush_cpu_cache();
    
    ehci_op_regs->asynclistaddr = (unsigned long)&ehci_async_head;
	
    cmd = ehci_op_regs->usb_cmd;
    cmd |= CMD_RUN;
    cmd |= CMD_ASE;
    ehci_op_regs->usb_cmd = cmd;

    for (i = 0; i < 2000; ++i)
    {
        sts = ehci_op_regs->usb_sts;

        if (sts & STS_HCHALTED) return -1;
        if (sts & STS_ASS) return 0;

        delay_ms(1);
    }

    return -2;
}

ENHANCED_CODE_SECTION void setup_ehci_control_qh(unsigned char address, unsigned short max_packet)
{
    ehci_control_qh.ep_char =
        ((unsigned long)address & 0x7F) |
        QH_DTC |
        QH_SPEED_HIGH |
        ((unsigned long)max_packet << 16);
    ehci_control_qh.ep_cap = 0;
    ehci_control_qh.current = 0;
    ehci_control_qh.next = EHCI_LINK_TERMINATE;
    ehci_control_qh.alt_next = EHCI_LINK_TERMINATE;
    ehci_control_qh.token = 0;
    ehci_control_qh.buffer[0] = 0;
    ehci_control_qh.buffer[1] = 0;
    ehci_control_qh.buffer[2] = 0;
    ehci_control_qh.buffer[3] = 0;
    ehci_control_qh.buffer[4] = 0;
}

ENHANCED_CODE_SECTION void build_control_transfer(unsigned long data_address, unsigned short data_length, int data_in)
{
    memset(&ehci_qtd_setup, 0, sizeof(ehci_qtd_setup));

    ehci_qtd_setup.next =
        ((unsigned long)&ehci_qtd_data &
         EHCI_PTR_MASK);
    ehci_qtd_setup.alt_next = EHCI_LINK_TERMINATE;
    ehci_qtd_setup.token =
        QTD_ACTIVE |
        QTD_CERR_3 |
        QTD_PID_SETUP |
        QTD_BYTES(8);
    qtd_set_buffer(&ehci_qtd_setup, (unsigned long)&ehci_setup_packet);

    memset(&ehci_qtd_data, 0, sizeof(ehci_qtd_data));
    ehci_qtd_data.next =
        ((unsigned long)&ehci_qtd_status &
         EHCI_PTR_MASK);
    ehci_qtd_data.alt_next = EHCI_LINK_TERMINATE;
    ehci_qtd_data.token =
        QTD_ACTIVE |
        QTD_CERR_3 |
        (data_in ?
            QTD_PID_IN :
            QTD_PID_OUT) |
        QTD_TOGGLE |
        QTD_BYTES(data_length);
    qtd_set_buffer(&ehci_qtd_data, data_address);

    memset(&ehci_qtd_status, 0, sizeof(ehci_qtd_status));
    ehci_qtd_status.next = EHCI_LINK_TERMINATE;
    ehci_qtd_status.alt_next = EHCI_LINK_TERMINATE;
    ehci_qtd_status.token =
        QTD_ACTIVE |
        QTD_CERR_3 |
        (data_in ?
            QTD_PID_OUT :
            QTD_PID_IN) |
        QTD_TOGGLE |
        QTD_IOC;

    ehci_control_qh.current = 0;
    ehci_control_qh.next =
        ((unsigned long)&ehci_qtd_setup &
         EHCI_PTR_MASK);
    ehci_control_qh.alt_next = EHCI_LINK_TERMINATE;
    ehci_control_qh.token = 0;

    flush_cpu_cache();
}

ENHANCED_CODE_SECTION void build_no_data_control(void)
{
    memset(&ehci_qtd_setup, 0, sizeof(ehci_qtd_setup));

    memset(&ehci_qtd_status, 0, sizeof(ehci_qtd_status));

    ehci_qtd_setup.next =
        ((unsigned long)&ehci_qtd_status &
         EHCI_PTR_MASK);
    ehci_qtd_setup.alt_next = EHCI_LINK_TERMINATE;
    ehci_qtd_setup.token =
        QTD_ACTIVE |
        QTD_CERR_3 |
        QTD_PID_SETUP |
        QTD_BYTES(8);
    qtd_set_buffer(&ehci_qtd_setup, (unsigned long)&ehci_setup_packet);

    ehci_qtd_status.next = EHCI_LINK_TERMINATE;
    ehci_qtd_status.alt_next = EHCI_LINK_TERMINATE;
    ehci_qtd_status.token =
        QTD_ACTIVE |
        QTD_CERR_3 |
        QTD_PID_IN |
        QTD_TOGGLE |
        QTD_IOC;

    ehci_control_qh.current = 0;
    ehci_control_qh.next =
        ((unsigned long)&ehci_qtd_setup &
         EHCI_PTR_MASK);
    ehci_control_qh.alt_next = EHCI_LINK_TERMINATE;
    ehci_control_qh.token = 0;

    flush_cpu_cache();
}

ENHANCED_CODE_SECTION int wait_control_transfer(void)
{
    unsigned long i;
    unsigned long sts;

    for (i = 0; i < 5000; ++i)
    {
        sts = ehci_op_regs->usb_sts;

        if (sts & STS_HCHALTED) return -100;
        if (ehci_qtd_setup.token & QTD_HALTED) return -1;
        if (ehci_qtd_data.token & QTD_HALTED) return -2;
        if (ehci_qtd_status.token & QTD_HALTED) return -3;
        if (ehci_qtd_setup.token & QTD_XACTERR) return -4;
        if (ehci_qtd_data.token & QTD_XACTERR) return -5;
        if (ehci_qtd_status.token & QTD_XACTERR) return -6;
        if (ehci_qtd_setup.token & QTD_BABBLE) return -7;
        if (ehci_qtd_data.token & QTD_BABBLE) return -8;
        if (ehci_qtd_status.token & QTD_BABBLE) return -9;
        if (ehci_qtd_setup.token & QTD_DBE) return -10;
        if (ehci_qtd_data.token & QTD_DBE) return -11;
        if (ehci_qtd_status.token & QTD_DBE) return -12;
        if (!(ehci_qtd_status.token & QTD_ACTIVE)) return 0;

        delay_ms(1);
    }

    return -50;
}

ENHANCED_CODE_SECTION int ehci_control_transfer(
    unsigned char address,
    unsigned short max_packet,
    unsigned char bmRequestType,
    unsigned char bRequest,
    unsigned short wValue,
    unsigned short wIndex,
    unsigned short wLength,
    void *buffer,
    int data_in)
{
    int rc;

    ehci_setup_packet.type = bmRequestType;
    ehci_setup_packet.request = bRequest;
    ehci_setup_packet.value = wValue;
    ehci_setup_packet.index = wIndex;
    ehci_setup_packet.length = wLength;
    setup_ehci_control_qh(address, max_packet);

    if (wLength)
    {
        build_control_transfer((unsigned long)buffer, wLength, data_in);
    }
    else
    {
        build_no_data_control();
    }

    rc = wait_control_transfer();
    if (rc) return rc;

    flush_cpu_cache();

    return 0;
}

ENHANCED_CODE_SECTION int ehci_bulk_transfer(unsigned char address, unsigned char endpoint, int data_in, unsigned short max_packet, void *buffer, unsigned long length, unsigned char toggle)
{
    unsigned long i, sts;
    
    ehci_control_qh.ep_char =
        ((unsigned long)address & 0x7F) |
        (((unsigned long)endpoint & 0x0F) << 8) |
        QH_SPEED_HIGH |
        ((unsigned long)max_packet << 16);
    
    ehci_control_qh.ep_cap = 0;
    ehci_control_qh.current = 0;
    ehci_control_qh.next = EHCI_LINK_TERMINATE;
    ehci_control_qh.alt_next = EHCI_LINK_TERMINATE;
    ehci_control_qh.token = 0;
    
    memset(&ehci_qtd_data, 0, sizeof(ehci_qtd_data));
    ehci_qtd_data.next = EHCI_LINK_TERMINATE;
    ehci_qtd_data.alt_next = EHCI_LINK_TERMINATE;
    
    ehci_qtd_data.token =
        QTD_ACTIVE |
        QTD_CERR_3 |
        (data_in ? QTD_PID_IN : QTD_PID_OUT) |
        (toggle ? QTD_TOGGLE : 0) |
        QTD_IOC | 
        QTD_BYTES(length);
        
    qtd_set_buffer(&ehci_qtd_data, (unsigned long)buffer);

    ehci_control_qh.next = ((unsigned long)&ehci_qtd_data & EHCI_PTR_MASK);
    flush_cpu_cache();

    for (i = 0; i < 5000; ++i)
    {
        sts = ehci_op_regs->usb_sts;
        if (sts & STS_HCHALTED) return -100;
        if (ehci_qtd_data.token & QTD_HALTED) return -1;
        if (ehci_qtd_data.token & QTD_XACTERR) return -5;
        if (ehci_qtd_data.token & QTD_BABBLE) return -8;
        if (ehci_qtd_data.token & QTD_DBE) return -11;
        if (!(ehci_qtd_data.token & QTD_ACTIVE)) return 0;
        delay_ms(1);
    }
    return -50;
}

ENHANCED_CODE_SECTION int usb_get_device_header(usb_device_descriptor_t *device_descriptor, unsigned short *max_packet)
{
    memset(device_descriptor, 0, sizeof(device_descriptor));
    if (ehci_control_transfer(
            0,
            64,
            0x80,
            USB_REQ_GET_DESC,
            ((unsigned short)USB_DESC_DEVICE << 8),
            0,
            8,
            device_descriptor,
            1) != 0)
    {

        return -1;
    }

    if (device_descriptor->type != USB_DESC_DEVICE)
        return -2;

    if (device_descriptor->max_packet != 8 &&
        device_descriptor->max_packet != 16 &&
        device_descriptor->max_packet != 32 &&
        device_descriptor->max_packet != 64)
    {

        return -3;
    }

    *max_packet = device_descriptor->max_packet;

    return 0;
}

ENHANCED_CODE_SECTION int usb_get_full_device(usb_device_descriptor_t *device_descriptor, unsigned short max_packet)
{
    memset(device_descriptor, 0, sizeof(device_descriptor));

    if (ehci_control_transfer(
            0,
            max_packet,
            0x80,
            USB_REQ_GET_DESC,
            ((unsigned short)USB_DESC_DEVICE << 8),
            0,
            18,
            device_descriptor,
            1) != 0)
    {
        return -1;
    }

    if (device_descriptor->length != 18 ||
        device_descriptor->type != USB_DESC_DEVICE) return -2;

    return 0;
}

ENHANCED_CODE_SECTION int usb_set_address(unsigned char address, unsigned short max_packet)
{
    int rc;

    ehci_setup_packet.type = 0x00;
    ehci_setup_packet.request = USB_REQ_SET_ADDR;
    ehci_setup_packet.value = address;
    ehci_setup_packet.index = 0;
    ehci_setup_packet.length = 0;
    setup_ehci_control_qh(0, max_packet);

    build_no_data_control();

    rc = wait_control_transfer();
    if (rc) return rc;

    delay_ms(2);

    setup_ehci_control_qh(address, max_packet);

    return 0;
}

ENHANCED_CODE_SECTION int usb_get_string(unsigned char address, unsigned short max_packet, unsigned short langid, unsigned char index, char *str)
{
	unsigned char string_descriptor[256] __attribute__((aligned(64)));
    unsigned int length;
    unsigned long i, sl = 0;
    char *s = str;

    if (!index)
        return 0;

    memset(string_descriptor, 0, sizeof(string_descriptor));

    if (ehci_control_transfer(
            address,
            max_packet,
            0x80,
            USB_REQ_GET_DESC,
            ((unsigned short)USB_DESC_STRING << 8) |
            index,
            langid,
            255,
            string_descriptor,
            1) != 0)
    {
        return -1;
    }

    length = string_descriptor[0];

    if (length < 2)
        return -2;

    if (length > 255)
        length = 255;

    for (i=2;i+1<length;i+=2)
    {
        unsigned short c;

        c = (unsigned short)string_descriptor[i] |
            ((unsigned short)string_descriptor[i + 1] << 8);

        if (c >= 32 && c < 127)
        {
        	s[sl] = (char)c;
        	sl++;
        }
    }

    s[sl] = '\0';

    return 0;
}

ENHANCED_CODE_SECTION int usb_get_configuration(unsigned char address, unsigned short max_packet)
{
	unsigned char configuration_descriptor[512] __attribute__((aligned(64)));
    unsigned short total_length;
    unsigned char *b;
	
    usb_device[usb_device_count].hci_type = HCI_TYPE_EHCI;
	usb_device[usb_device_count].port = address-1;

    memset(configuration_descriptor, 0, sizeof(configuration_descriptor));

    if (ehci_control_transfer(
            address,
            max_packet,
            0x80,
            USB_REQ_GET_DESC,
            ((unsigned short)USB_DESC_CONFIG << 8),
            0,
            9,
            configuration_descriptor,
            1) != 0)
    {
        return -1;
    }

    b = configuration_descriptor;

    if (b[0] < 9 || b[1] != USB_DESC_CONFIG) return -2;

    total_length = (unsigned short)b[2] | ((unsigned short)b[3] << 8);
    if (total_length > sizeof(configuration_descriptor)) return -3;

    if (ehci_control_transfer(
            address,
            max_packet,
            0x80,
            USB_REQ_GET_DESC,
            ((unsigned short)USB_DESC_CONFIG << 8),
            0,
            total_length,
            configuration_descriptor,
            1) != 0)
    {
        return -4;
    }

    unsigned short off = 0;

    while (off+2 <= total_length)
    {
        unsigned char len;
        unsigned char type;

        len = configuration_descriptor[off];
        type = configuration_descriptor[off+1];

        if (len < 2)
            break;

        if (off + len > total_length)
            break;
		
		/*
        printf(
            "  Descriptor %u: type=0x%02X, length=%u\n",
            off,
            type,
            len);
        */

        if (type == 4 && len >= 9)
        {
        	usb_interface_descriptor_t *d = (usb_interface_descriptor_t*)&configuration_descriptor[off];
        	unsigned char conf_interface = d->interface_number;
        	unsigned char conf_class = d->interface_class;
        	unsigned char conf_subclass = d->interface_subclass;
        	unsigned char conf_protocol = d->interface_protocol;
        	usb_device[usb_device_count].interface_number = conf_interface;
        	usb_device[usb_device_count].class_code = conf_class;
        	usb_device[usb_device_count].subclass_code = conf_subclass;
        	usb_device[usb_device_count].protocol_code = conf_protocol;
        }

        if (type == 5 && len >= 7)
        {
        	usb_endpoint_descriptor_t *d = (usb_endpoint_descriptor_t*)&configuration_descriptor[off];
            unsigned char ep;
            unsigned short mps;            
            //unsigned char attr = d->attributes;
            ep = d->ep_address;
            mps = d->max_packet_size;
            if (ep & 0x80) usb_device[usb_device_count].endpoint_in = ep;
            else usb_device[usb_device_count].endpoint_out = ep;
            usb_device[usb_device_count].max_packet = mps;
            /*
            printf(
                "    Endpoint 0x%02X: attributes=0x%02X, max_packet_size=%u\n",
                ep,
                attr,
                mps);
            */
        }

        off += len;
    }

    return 0;
}

    
ENHANCED_CODE_SECTION int usb_enumerate(unsigned char port)
{
    usb_device_descriptor_t d  __attribute__((aligned(64)));
    unsigned short max_packet=64;
    unsigned char usb_address = port+1;    

    if (usb_get_device_header(&d, &max_packet) != 0) return -1;
    if (usb_get_full_device(&d, max_packet) != 0) return -2;
    if (usb_set_address(usb_address, max_packet) != 0) return -3;
    if (usb_get_configuration(usb_address, max_packet) != 0) return -4;    
    
    if (d.manufacturer)
    {
    	char mstr[256];
    	memset(mstr, 0, 256);
        usb_get_string(usb_address, max_packet, 0x0409, d.manufacturer, mstr);
        strcpy(usb_device[usb_device_count].manufacturer, mstr);
    }
    
    if (d.product_string)
    {
    	char prodstr[256];
    	memset(prodstr, 0, 256);
        usb_get_string(usb_address, max_packet, 0x0409, d.product_string, prodstr);
        strcpy(usb_device[usb_device_count].name, prodstr);
    }

    if (d.serial)
    {
    	char serialstr[256];
    	memset(serialstr, 0, 256);
        usb_get_string(usb_address, max_packet, 0x0409, d.serial, serialstr);
        strcpy(usb_device[usb_device_count].serial, serialstr);
    }
    
    usb_device_count++;
    
    return 0;
}

ENHANCED_CODE_SECTION usb_device_t* find_usb_ehci_device_by_port(unsigned char port)
{
	for(int i=0;i<usb_device_count;i++)
	{
		if (usb_device[i].port == port)
		{
			usb_device_t* dev = (usb_device_t*)&usb_device[i];
			return dev;
		}
	}
	return NULL;
}

ENHANCED_CODE_SECTION int init_ehci(void)
{
    unsigned char p;
	
    if (init_usb_ehci())
    {	
    	ehci_available_ports = 0;
    	memset(&ehci_available_port[0], 0, 15);
	
    	for (p=0;p<ehci_ports;++p)
    	{
        	unsigned long v;
	
        	v = ehci_port_read(p);
        	if (!(v & PORT_CCS))
            	continue;
            	
        	if (ehci_reset_port(p) == 0)
        	{
            	ehci_available_port[ehci_available_ports] = p;
            	ehci_available_ports++;
        	}
    	}
    	if (ehci_available_ports == 0) return -2;
    	ehci_build_async();
    	if (ehci_start_async() != 0) return -3;
    	for (int i=0;i<ehci_available_ports;i++)
    	{
    		if (usb_enumerate(ehci_available_port[i]) != 0)
			{
				printk("USB Error\n");
			}
		}
		ehci_initialized = 1;
    }
    else
    {
    	ehci_available_ports = 0;
    	memset(&ehci_available_port[0], 0, 15);
    	return -1;
    }
    
    return 0;
}

ENHANCED_CODE_SECTION int loadusb(void)
{
    usb_device_count = 0;    
    memset(&usb_device[0], 0, (MAX_USB_DEVICES*sizeof(usb_device_t)));
	int result = init_ehci();
	if (result != 0) return 0;
	return 1;
}



ehci_msc_t ehci_msc_list[MAX_USB_DEVICES] ENHANCED_DATA_SECTION;
int ehci_msc_list_count ENHANCED_DATA_SECTION=0;
int has_ehci_msc ENHANCED_DATA_SECTION = 0;

ENHANCED_CODE_SECTION int init_ehci_msc(void)
{
	if (ehci_initialized == 0)
	{
		if (!loadusb())
		{
			ehci_msc_list_count = 0;
			has_ehci_msc = 0;
			return 0;
		}
	}
	ehci_msc_list_count = 0;
	memset(ehci_msc_list, 0, (MAX_USB_DEVICES*sizeof(ehci_msc_t)));
	if (ehci_initialized == 1)
	{
		for(int i=0;i<ehci_available_ports;i++)
		{
			usb_device_t *dev = find_usb_ehci_device_by_port(ehci_available_port[i]);
			if (dev != NULL)
			{
				if ((dev->class_code == 0x08) && 
				(dev->subclass_code == 0x06) && 
				(dev->protocol_code == 0x50))
				{
					ehci_msc_list[ehci_msc_list_count].port = dev->port;
					strcpy(ehci_msc_list[ehci_msc_list_count].name, dev->name);
					ehci_msc_list_count++;
				}
			}
		}
		if (ehci_msc_list_count != 0)
		{
			has_ehci_msc = 1;
			return 1;
		}
	}
	has_ehci_msc = 0;
	return 0;
}

ENHANCED_CODE_SECTION int get_ehci_msc_name(int id, char *buffer)
{
	memset(&buffer[0], 0, 256);
	if (ehci_initialized == 1)
	{
		if (has_ehci_msc == 1)
		{
			if (id >= MAX_USB_DEVICES) return 0;
			if (id < 0) return 0;
			if (id >= ehci_msc_list_count) return 0;
			memcpy(&buffer[0], &ehci_msc_list[id].name[0], strlen(ehci_msc_list[id].name));
			return 1;
		}
	}
	return 0;
}

ENHANCED_CODE_SECTION int ehci_msc_read_sector(int drive_id, unsigned long lba, unsigned short num_sectors, unsigned short sector_size, void *buffer)
{
    if (drive_id < 0 || drive_id >= ehci_msc_list_count) return -1;
    
    unsigned char address = ehci_msc_list[drive_id].port + 1;
    unsigned long transfer_length = num_sectors * sector_size;    
    ehci_msc_cbw_t cbw __attribute__((aligned(32)));
    ehci_msc_csw_t csw __attribute__((aligned(32)));
    usb_device_t *dev = find_usb_ehci_device_by_port(ehci_msc_list[drive_id].port);   
    unsigned char ep_in = dev->endpoint_in;
    unsigned char ep_out = dev->endpoint_out;
    unsigned short max_packet = dev->max_packet;
    int rc;

    memset(&cbw, 0, sizeof(ehci_msc_cbw_t));
    cbw.signature = MSC_CBW_SIGNATURE;
    cbw.tag = 0x12345678;
    cbw.data_transfer_length = transfer_length;
    cbw.flags = 0x80;
    cbw.lun = 0;
    cbw.cb_length = 10;
    
    cbw.cb[0] = SCSI_CMD_READ_10;
    cbw.cb[1] = 0;
    cbw.cb[2] = (unsigned char)(lba >> 24);
    cbw.cb[3] = (unsigned char)(lba >> 16);
    cbw.cb[4] = (unsigned char)(lba >> 8);
    cbw.cb[5] = (unsigned char)(lba & 0xFF);
    cbw.cb[6] = 0;
    cbw.cb[7] = (unsigned char)(num_sectors >> 8);
    cbw.cb[8] = (unsigned char)(num_sectors & 0xFF);
    cbw.cb[9] = 0;

    rc = ehci_bulk_transfer(address, ep_out, 0, max_packet, &cbw, sizeof(ehci_msc_cbw_t), 0);
    if (rc != 0) return -2;

    rc = ehci_bulk_transfer(address, ep_in, 1, max_packet, buffer, transfer_length, 1);
    if (rc != 0) return -3;

    rc = ehci_bulk_transfer(address, ep_in, 1, max_packet, &csw, sizeof(ehci_msc_csw_t), 0);
    if (rc != 0) return -4;

    if (csw.signature != MSC_CSW_SIGNATURE || csw.status != 0) {
        return -5;
    }

    return 0;
}

#if 0
void detectehci(void)
{	
	int i;
	char buffer[256];
	if (ehci_initialized == 1)
	{
		if (has_ehci_msc == 1)
		{
			if (ehci_msc_list_count == 0) return;
			for(i=0;i<ehci_msc_list_count;i++)
			{
				if (get_ehci_msc_name(i, buffer))
				{
					if (strlen(buffer) == 0)
					{
						strcpy(buffer, "Virtual HD");
					}
					printk("EHCI USB Drive %d: %s\n", i, buffer);
				}
			}
		}
	}
}
#endif

