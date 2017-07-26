#ifndef _USB_INTERFACE_H
#define _USB_INTERFACE_H

#include <libusb-1.0/libusb.h>

/* functions for the USB interfaces */
/*----------------------------------*/
void printdev(libusb_device *dev);
int check_usb();

#endif
