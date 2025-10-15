#pragma once
#include "kernel.h"

/*
 * USB core (Phase 2 stubs)
 * - Controller drivers: xHCI/EHCI/OHCI (stubs)
 * - Hub enumeration (stub)
 * - HID (keyboard, mouse) via input subsystem
 */

typedef enum { USB_XHCI=0, USB_EHCI=1, USB_OHCI=2 } usb_ctrl_kind_t;

typedef struct usb_ctrl {
    usb_ctrl_kind_t kind;
    void* drv; /* driver-private */
} usb_ctrl_t;

void usb_init(void);

/* HID to input */
void usb_hid_kbd_input(u8 keycode, int pressed);
void usb_hid_mouse_input(int dx, int dy, u8 buttons);