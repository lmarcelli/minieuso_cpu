#ifndef _USB_INTERFACE_H
#define _USB_INTERFACE_H

#include <libusb-1.0/libusb.h>

#include <thread>

#include "log.h"
#include "CpuTools.h"

#define MIN_DEVICE_NUM 5 /* number of devices without extra storage or config USBs */
#define NOMINAL_DEVICE_NUM 9 /* number of devices expected */

#define USB_MOUNTPOINT_0 "/media/usb0"
#define USB_MOUNTPOINT_1 "/media/usb1"

#define N_USB_UNDEF 0xFF

/* configuration for spare CPU in Stockholm */
#define STORAGE_BUS 2
#define CAMERA_BUS 1
#define CONFIG_BUS 0

/* configuration for new CPU in Italy */
#define STORAGE_BUS_NEW 1
#define CAMERA_BUS_NEW 2

/**
 * handles the interface to all USB devices, including 
 * data storage and automated backup
 */
class UsbManager {
public:
  /**
   * defines the bus upon which USB storage is present
   * depends on the CPU module
   */
  uint8_t storage_bus;
  /**
   * stores the number of storage devices connected
   */
  uint8_t num_storage_dev;
  /**
   * stores true if a data backup is running
   */
  bool backup_launched;
  
  UsbManager();
  static int CheckUsb();
  uint8_t LookupUsbStorage();
  int RunDataBackup();
  int KillDataBackup();
  
private:
  /**
   * stores the backup thread handle
   */
  std::thread::native_handle_type backup_thread_handle;  
  
  void CheckCpuModel();
  static void PrintDev(libusb_device * dev);
  int DataBackup();
  int GetDeviceInterface(libusb_device * dev);
};
#endif
/* _USB_INTERFACE_H */
