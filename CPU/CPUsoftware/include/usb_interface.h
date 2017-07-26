#ifndef _USB_INTERFACE_H
#define _USB_INTERFACE_H

#include <libusb-1.0/libusb.h>

#define MIN_DEVICE_NUM 6 /* number of devices without extra storage or config USBs */
#define NOMINAL_DEVICE_NUM 9 /* number of devices expected */

/* configuration for spare CPU in Stockholm */
#define STORAGE_BUS_0 1
#define STORAGE_BUS_1 2
#define STORAGE_PORT_0 2
#define STORAGE_PORT_1 1
#define CONFIG_BUS 0
#define CONFIG_PORT 0

/* functions for the USB interfaces */
/*----------------------------------*/
void printdev(libusb_device *dev);
int check_usb();
uint8_t lookup_usb();
int def_data_backup(uin8_t num_storage_dev);

#endif
