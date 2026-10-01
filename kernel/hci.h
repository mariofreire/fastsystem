// Fast System USB Host Controller Interface (HCI)
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2026 DSP Interactive.

#ifndef HCI_TYPES_H
#define HCI_TYPES_H

/* 
 * USB Host Controller Interface (HCI) Type Definitions
 */

/* USB 1.0 / 1.1 Interfaces */
#define HCI_TYPE_UHCI    0x01  /* Universal Host Controller Interface (Intel/VIA) */
#define HCI_TYPE_OHCI    0x02  /* Open Host Controller Interface (Compaq/Microsoft) */

/* USB 2.0 Interface */
#define HCI_TYPE_EHCI    0x03  /* Enhanced Host Controller Interface */

/* USB 3.0+ Interface */
#define HCI_TYPE_XHCI    0x04  /* eXtensible Host Controller Interface */

/* Helper macros to get string representation */
#define HCI_TYPE_TO_STR(type) \
    ((type) == HCI_TYPE_UHCI ? "UHCI (USB 1.x Intel)" : \
     (type) == HCI_TYPE_OHCI ? "OHCI (USB 1.x Open)"  : \
     (type) == HCI_TYPE_EHCI ? "EHCI (USB 2.0)"       : \
     (type) == HCI_TYPE_XHCI ? "XHCI (USB 3.x+)"      : "Unknown HCI")

#endif /* HCI_TYPES_H */
