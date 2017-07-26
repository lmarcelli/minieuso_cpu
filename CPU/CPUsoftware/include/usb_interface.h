#ifndef _USB_INTERFACE_H
#define _USB_INTERFACE_H

#include <libusb-1.0/libusb.h>

#define MIN_DEVICE_NUM 6 /* number of devices without extra storage or config USBs */
#define NOMINAL_DEVICE_NUM 9 /* number of devices expected */
#define STORAGE_BUS 2
#define STORAGE_PORT_1 1
#define STORAGE_PORT_2 2

/* functions for the USB interfaces */
/*----------------------------------*/
void printdev(libusb_device *dev);
int check_usb();
int lookup_usb();

#endif
