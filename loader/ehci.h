// Fast System Kernel Loader - Enhanced Host Controller Interface
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2024 DSP Interactive.

#ifndef __EHCI_H__
#define __EHCI_H__

#define USB_EHCI_ADDRESS 0x842B00

#define EHCI_PCI_CLASS_SERIAL  0x0C
#define EHCI_PCI_SUB_USB       0x03
#define EHCI_PCI_PROG_EHCI     0x20

#define EHCI_CAP_CAPLENGTH     0x00
#define EHCI_CAP_HCSPARAMS     0x04
#define EHCI_CAP_HCCPARAMS     0x08

#define OP_USBCMD         0x00
#define OP_USBSTS         0x04
#define OP_USBINTR        0x08
#define OP_FRINDEX        0x0C
#define OP_CTRLDSSEG      0x10
#define OP_PERIODICLIST   0x14
#define OP_ASYNCLIST      0x18
#define OP_CONFIGFLAG     0x40
#define OP_PORTSC         0x44

#define CMD_RUN           (1U << 0)
#define CMD_RESET         (1U << 1)
#define CMD_PSE           (1U << 4)
#define CMD_ASE           (1U << 5)

#define STS_USBINT        (1U << 0)
#define STS_USBERRINT     (1U << 1)
#define STS_PORT_CHANGE   (1U << 2)
#define STS_HCHALTED      (1U << 12)
#define STS_RECLAMATION   (1U << 13)
#define STS_PSS           (1U << 14)
#define STS_ASS           (1U << 15)

#define PORT_CCS          (1U << 0)
#define PORT_CSC          (1U << 1)
#define PORT_PED          (1U << 2)
#define PORT_PEC          (1U << 3)
#define PORT_OCC          (1U << 5)
#define PORT_PR           (1U << 8)
#define PORT_PP           (1U << 12)
#define PORT_OWNER        (1U << 13)

#define PORTSC_W1C        \
    (PORT_CSC | PORT_PEC | PORT_OCC)

#define EHCI_LINK_TERMINATE   0x00000001U
#define EHCI_LINK_QH          0x00000002U
#define EHCI_PTR_MASK         0xFFFFFFE0U

#define QH_DTC             (1U << 14)
#define QH_HEAD            (1U << 15)

#define QH_SPEED_FULL      (0U << 12)
#define QH_SPEED_LOW       (1U << 12)
#define QH_SPEED_HIGH      (2U << 12)

#define QTD_ACTIVE         (1U << 7)
#define QTD_HALTED         (1U << 6)
#define QTD_DBE            (1U << 5)
#define QTD_BABBLE         (1U << 4)
#define QTD_XACTERR        (1U << 3)
#define QTD_MISSED         (1U << 2)

#define QTD_PID_OUT        (0U << 8)
#define QTD_PID_IN         (1U << 8)
#define QTD_PID_SETUP      (2U << 8)

#define QTD_CERR_3         (3U << 10)

#define QTD_IOC            (1U << 15)

#define QTD_BYTES(n)       \
    (((unsigned long)(n) & 0x7FFFU) << 16)

#define QTD_TOGGLE         (1U << 31)

#define MSC_CBW_SIGNATURE 0x43425355 // 'USBC'
#define MSC_CSW_SIGNATURE 0x53425355 // 'USBS'

#define SCSI_CMD_READ_10  0x28

#define USB_ANY -1

#define USB_DESC_DEVICE    1
#define USB_DESC_CONFIG    2
#define USB_DESC_STRING    3

#define USB_REQ_GET_DESC   6
#define USB_REQ_SET_ADDR   5
#define USB_REQ_SET_CONFIG 9

#define MAX_USB_DEVICES    15

#ifndef SECTORSIZE
#define SECTORSIZE 512
#endif

#define HCI_TYPE_UHCI    0x01
#define HCI_TYPE_OHCI    0x02
#define HCI_TYPE_EHCI    0x03
#define HCI_TYPE_XHCI    0x04

#define HCI_TYPE_TO_STR(type) \
    ((type) == HCI_TYPE_UHCI ? "UHCI" : \
     (type) == HCI_TYPE_OHCI ? "OHCI" : \
     (type) == HCI_TYPE_EHCI ? "EHCI" : \
     (type) == HCI_TYPE_XHCI ? "XHCI" : "Unknown HCI")

#define HCI_TYPE_TO_STR_EXT(type) \
    ((type) == HCI_TYPE_UHCI ? "UHCI (USB 1.x Intel)" : \
     (type) == HCI_TYPE_OHCI ? "OHCI (USB 1.x Open)"  : \
     (type) == HCI_TYPE_EHCI ? "EHCI (USB 2.0)"       : \
     (type) == HCI_TYPE_XHCI ? "XHCI (USB 3.x+)"      : "Unknown HCI")


#pragma pack (push, 1)

typedef struct
{
	char signature[4];
	unsigned char version;
	unsigned char ports;
	unsigned long address;
} usb_ehci_t;

typedef struct
{
    volatile unsigned char cap_length;
	volatile unsigned char reserved;
    volatile unsigned short version;
    volatile unsigned long sparams;
    volatile unsigned long cparams;
    volatile unsigned long sp_portroute_h;
    volatile unsigned long sp_portroute_l;
} usb_ehci_cap_t;

typedef struct
{
    volatile unsigned long usb_cmd;
    volatile unsigned long usb_sts;
    volatile unsigned long usb_intr;
    volatile unsigned long frameindex;
    volatile unsigned long ctrl_ds_segment;
    volatile unsigned long periodiclistbase;
    volatile unsigned long asynclistaddr;
    volatile unsigned long reserved[9];
    volatile unsigned long configflag;
    volatile unsigned long portsc[15];
} usb_ehci_op_regs_t;

typedef struct
{
    volatile unsigned long next;
    volatile unsigned long alt_next;
    volatile unsigned long token;
    volatile unsigned long buffer[5];
} ehci_qtd_t;

typedef struct 
{
    volatile unsigned long horizontal;
    volatile unsigned long ep_char;
    volatile unsigned long ep_cap;
    volatile unsigned long current;
    volatile unsigned long next;
    volatile unsigned long alt_next;
    volatile unsigned long token;
    volatile unsigned long buffer[5];
} ehci_qh_t;

typedef struct
{
    unsigned char  type;
    unsigned char  request;
    unsigned short value;
    unsigned short index;
    unsigned short length;
} usb_setup_t;

typedef struct
{
    unsigned char  length;
    unsigned char  type;
    unsigned short usb_version;
    unsigned char  device_class;
    unsigned char  device_subclass;
    unsigned char  device_protocol;
    unsigned char  max_packet;
    unsigned short vendor;
    unsigned short product;
    unsigned short device_version;
    unsigned char  manufacturer;
    unsigned char  product_string;
    unsigned char  serial;
    unsigned char  configurations;
} usb_device_descriptor_t;

typedef struct
{
    unsigned char length;
    unsigned char type;
    unsigned char interface_number;
    unsigned char alternate_setting;
    unsigned char endpoints;
    unsigned char interface_class;
    unsigned char interface_subclass;
    unsigned char interface_protocol;
    unsigned char index;
} usb_interface_descriptor_t;

typedef struct
{
    unsigned char  length;
    unsigned char  type;
    unsigned char  ep_address;
    unsigned char  attributes;
    unsigned short max_packet_size;
    unsigned char  interval;
} usb_endpoint_descriptor_t;

typedef struct
{
    unsigned long signature;
    unsigned long tag;
    unsigned long data_transfer_length;
    unsigned char flags;
    unsigned char lun;
    unsigned char cb_length;
    unsigned char cb[16];
} ehci_msc_cbw_t;

typedef struct
{
    unsigned long signature;
    unsigned long tag;
    unsigned long data_residue;
    unsigned char status;
} ehci_msc_csw_t;

typedef struct 
{
    char name[256];
	char manufacturer[256];
    char serial[256];
    unsigned char port;
	unsigned char hci_type; 
	unsigned char interface_number;
    unsigned char class_code;
    unsigned char subclass_code;
    unsigned char protocol_code;   
    unsigned char endpoint_in;
    unsigned char endpoint_out;
    unsigned short max_packet;
} usb_device_t;

typedef struct 
{
    int port;
    char name[256];
} ehci_msc_t;

#pragma pack (pop)

extern ehci_qh_t ehci_async_head
    ENHANCED_DATA_SECTION;

extern ehci_qh_t ehci_control_qh
    ENHANCED_DATA_SECTION;

extern ehci_qtd_t ehci_qtd_setup
    ENHANCED_DATA_SECTION;

extern ehci_qtd_t ehci_qtd_data
    ENHANCED_DATA_SECTION;

extern ehci_qtd_t ehci_qtd_status
    ENHANCED_DATA_SECTION;

extern usb_setup_t ehci_setup_packet
    ENHANCED_DATA_SECTION;

extern unsigned char *usb_ehci_ptr
    ENHANCED_DATA_SECTION;

extern usb_ehci_t *usb_ehci
    ENHANCED_DATA_SECTION;

extern volatile unsigned long ehci_base
    ENHANCED_DATA_SECTION;

extern volatile unsigned long ehci_op
    ENHANCED_DATA_SECTION;

extern unsigned char ehci_ports
    ENHANCED_DATA_SECTION;

extern unsigned char ehci_available_ports
    ENHANCED_DATA_SECTION;

extern unsigned char ehci_available_port[15]
    ENHANCED_DATA_SECTION;

extern volatile usb_ehci_cap_t *ehci_cap
    ENHANCED_DATA_SECTION;

extern volatile usb_ehci_op_regs_t *ehci_op_regs
    ENHANCED_DATA_SECTION;

extern int ehci_initialized
    ENHANCED_DATA_SECTION;

extern usb_device_t usb_device[MAX_USB_DEVICES]
    ENHANCED_DATA_SECTION;

extern int usb_device_count
    ENHANCED_DATA_SECTION;


/* --------------------------------------------------------------------------
 * EHCI Mass Storage Global Data
 * -------------------------------------------------------------------------- */

extern ehci_msc_t ehci_msc_list[MAX_USB_DEVICES]
    ENHANCED_DATA_SECTION;

extern int ehci_msc_list_count
    ENHANCED_DATA_SECTION;

extern int has_ehci_msc
    ENHANCED_DATA_SECTION;

extern unsigned char sector_buffer[SECTORSIZE]
    ENHANCED_DATA_SECTION;


/* --------------------------------------------------------------------------
 * USB / EHCI Utility Functions
 * -------------------------------------------------------------------------- */

extern const char *ehci_get_usb_device_type(
    unsigned char class_code,
    unsigned char subclass_code,
    unsigned char protocol_code)
    ENHANCED_CODE_SECTION;

extern void irq_restore(
    unsigned long flags)
    ENHANCED_CODE_SECTION;

extern void delay_us(
    unsigned long us)
    ENHANCED_CODE_SECTION;

extern void delay_ms(
    unsigned long ms)
    ENHANCED_CODE_SECTION;

extern void flush_cpu_cache(void)
    ENHANCED_CODE_SECTION;


/* --------------------------------------------------------------------------
 * EHCI Controller Functions
 * -------------------------------------------------------------------------- */

extern int init_usb_ehci(void)
    ENHANCED_CODE_SECTION;

extern int ehci_reset(void)
    ENHANCED_CODE_SECTION;

extern int ehci_start(void)
    ENHANCED_CODE_SECTION;

extern int ehci_take_ownership(void)
    ENHANCED_CODE_SECTION;

extern unsigned long ehci_port_read(
    unsigned char p)
    ENHANCED_CODE_SECTION;

extern void ehci_port_write(
    unsigned char p,
    unsigned long v)
    ENHANCED_CODE_SECTION;

extern int ehci_reset_port(
    unsigned char p)
    ENHANCED_CODE_SECTION;

extern int init_ehci(void)
    ENHANCED_CODE_SECTION;

extern int loadusb(void)
    ENHANCED_CODE_SECTION;


/* --------------------------------------------------------------------------
 * EHCI Queue / Transfer Functions
 * -------------------------------------------------------------------------- */

extern void qtd_set_buffer(
    ehci_qtd_t *qtd,
    unsigned long address)
    ENHANCED_CODE_SECTION;

extern int qtd_error(
    unsigned long token)
    ENHANCED_CODE_SECTION;

extern void ehci_build_async(void)
    ENHANCED_CODE_SECTION;

extern int ehci_start_async(void)
    ENHANCED_CODE_SECTION;

extern void setup_ehci_control_qh(
    unsigned char address,
    unsigned short max_packet)
    ENHANCED_CODE_SECTION;

extern void build_control_transfer(
    unsigned long data_address,
    unsigned short data_length,
    int data_in)
    ENHANCED_CODE_SECTION;

extern void build_no_data_control(void)
    ENHANCED_CODE_SECTION;

extern int wait_control_transfer(void)
    ENHANCED_CODE_SECTION;

extern int ehci_control_transfer(
    unsigned char address,
    unsigned short max_packet,
    unsigned char bmRequestType,
    unsigned char bRequest,
    unsigned short wValue,
    unsigned short wIndex,
    unsigned short wLength,
    void *buffer,
    int data_in)
    ENHANCED_CODE_SECTION;

extern int ehci_bulk_transfer(
    unsigned char address,
    unsigned char endpoint,
    int data_in,
    unsigned short max_packet,
    void *buffer,
    unsigned long length,
    unsigned char toggle)
    ENHANCED_CODE_SECTION;


/* --------------------------------------------------------------------------
 * USB Enumeration Functions
 * -------------------------------------------------------------------------- */

extern int usb_get_device_header(
    usb_device_descriptor_t *device_descriptor,
    unsigned short *max_packet)
    ENHANCED_CODE_SECTION;

extern int usb_get_full_device(
    usb_device_descriptor_t *device_descriptor,
    unsigned short max_packet)
    ENHANCED_CODE_SECTION;

extern int usb_set_address(
    unsigned char address,
    unsigned short max_packet)
    ENHANCED_CODE_SECTION;

extern int usb_get_string(
    unsigned char address,
    unsigned short max_packet,
    unsigned short langid,
    unsigned char index,
    char *str)
    ENHANCED_CODE_SECTION;

extern int usb_get_configuration(
    unsigned char address,
    unsigned short max_packet)
    ENHANCED_CODE_SECTION;

extern int usb_enumerate(
    unsigned char port)
    ENHANCED_CODE_SECTION;

extern usb_device_t *find_usb_ehci_device_by_port(
    unsigned char port)
    ENHANCED_CODE_SECTION;


/* --------------------------------------------------------------------------
 * EHCI Mass Storage Functions
 * -------------------------------------------------------------------------- */

extern int init_ehci_msc(void)
    ENHANCED_CODE_SECTION;

extern int get_ehci_msc_name(
    int id,
    char *buffer)
    ENHANCED_CODE_SECTION;

extern int ehci_msc_read_sector(
    int drive_id,
    unsigned long lba,
    unsigned short num_sectors,
    unsigned short sector_size,
    void *buffer)
    ENHANCED_CODE_SECTION;

extern void detectehci(void);
    //ENHANCED_CODE_SECTION;



#endif // __EHCI_H__
