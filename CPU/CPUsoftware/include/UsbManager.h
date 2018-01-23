#ifndef _USB_INTERFACE_H
#define _USB_INTERFACE_H

#include <libusb-1.0/libusb.h>

#include <iostream>
#include <fstream>

#include "log.h"

#define MIN_DEVICE_NUM 6 /* number of devices without extra storage or config USBs */
#define NOMINAL_DEVICE_NUM 9 /* number of devices expected */

#define USB_MOUNTPOINT_0 "/media/usb0"
#define USB_MOUNTPOINT_1 "/media/usb1"

/* configuration for spare CPU in Stockholm */
#define STORAGE_BUS_0 1
#define STORAGE_BUS_1 2
#define STORAGE_PORT_0 2
#define STORAGE_PORT_1 1
#define CONFIG_BUS 0
#define CONFIG_PORT 0

class UsbManager {
public:
  uint8_t num_storage_dev;
  
  UsbManager();
  static int CheckUsb();
  uint8_t LookupUsb();
  int DataBackup();

private:
  static void PrintDev(libusb_device * dev);
  
};
#endif
/* _USB_INTERFACE_H */
