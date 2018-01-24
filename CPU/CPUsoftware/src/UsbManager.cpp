#include "UsbManager.h"

/* default constructor */
UsbManager::UsbManager() {
  this->num_storage_dev = N_USB_UNDEF;
}

/* print a description of usb devices connected */
void UsbManager::PrintDev(libusb_device * dev) {

  /* get the device descriptor */
  libusb_device_descriptor desc;
  int r = libusb_get_device_descriptor(dev, &desc);
  if (r < 0) {
    std::cout << "failed to get device descriptor" << std::endl;
    return;
  }
  
  std::cout << "Number of possible configurations: " << (int)desc.bNumConfigurations << "  ";
  std::cout << "Device Class: " << (int)desc.bDeviceClass << "  ";
  std::cout << "VendorID: " << desc.idVendor << "  ";
  std::cout << "ProductID: " << desc.idProduct << std::endl;

  /* get the config descriptor */
  libusb_config_descriptor * config;
  libusb_get_config_descriptor(dev, 0, &config);

  /* print the number of interfaces */
  std::cout << "Interfaces: " << (int)config->bNumInterfaces << " ||| ";

  const libusb_interface * inter;
  const libusb_interface_descriptor * interdesc;
  const libusb_endpoint_descriptor * epdesc;

  /* print information on each device */
  for(int i=0; i < (int)config->bNumInterfaces; i++) {
    inter = &config->interface[i];
    std::cout << "Number of alternate settings: " << inter->num_altsetting << " | ";

    for(int j=0; j < inter->num_altsetting; j++) {
      interdesc = &inter->altsetting[j];
      std::cout << "Interface Number: " << (int)interdesc->bInterfaceNumber << " | ";
      std::cout << "Number of endpoints: " << (int)interdesc->bNumEndpoints << " | ";

      for(int k=0; k < (int)interdesc->bNumEndpoints; k++) {
	epdesc = &interdesc->endpoint[k];
	std::cout << "Descriptor Type: " << (int)epdesc->bDescriptorType << " | ";
	std::cout << "EP Address: " << (int)epdesc->bEndpointAddress << " | ";
      } 
    }    
  }
  
  std::cout << std::endl << std::endl << std::endl;

  /* free the descriptor */
  libusb_free_config_descriptor(config);
}

/* check the number of devices connected and print their info */
int UsbManager::CheckUsb() {
  libusb_device ** devs;
  libusb_context * ctx = NULL;
  int r;
  ssize_t cnt, i;

  /* initialise a libusb session */
  r = libusb_init(&ctx);
  if (r < 0) {
    std::cout << "init error for libusb" << std::endl;
    return 1;
  }

  /* set the verbosity level to 3, as suggested by docs */
  libusb_set_debug(ctx, 3);

  /* get the list of devices */
  cnt = libusb_get_device_list(ctx, &devs);
  if (cnt < 0) {
    std::cout << "get device error for libusb" << std::endl;
  }

  /* print the number of connected devices */
  std::cout << cnt << " USB devices connected" << std::endl;

  for (i = 0; i < cnt; i++) {
    /* print the specs of each device */
    PrintDev(devs[i]);
    std::cout << (int)libusb_get_port_number(devs[i]) << std::endl;
  }

  /* clean up */
  libusb_free_device_list(devs, 1);
  libusb_exit(ctx);
  
  return 0;
}

/* lookup usb storage devices connected and identify them */
uint8_t UsbManager::LookupUsbStorage() {
  libusb_device ** all_devs;
  libusb_device * dev;
  libusb_device_descriptor desc = {0};
  libusb_context * ctx = NULL;
  int r, num_storage_dev = 0;
  ssize_t cnt, i;

  clog << "info: " << logstream::info << "looking up USB storage devices" << std::endl;
  
  /* initialise a libusb session */
  r = libusb_init(&ctx);
  if (r < 0) {
    std::cout << "init error for libusb" << std::endl;
    clog << "error: " << logstream::error << "init error for libusb" << std::endl;
    return 1;
  }

  /* set the verbosity level to 3, as suggested by docs */
  libusb_set_debug(ctx, 3);

  /* get the list of devices */
  cnt = libusb_get_device_list(ctx, &all_devs);
  if (cnt < 0) {
    clog << "error: " << logstream::error << "get device error for libusb" << std::endl;
  }

  /* check number of connected devices */
  if (cnt == MIN_DEVICE_NUM) {
    clog << "error: " << logstream::error << "no storage or config USBs connected" << std::endl;
  }
  else if (cnt < MIN_DEVICE_NUM) {
   clog << "error: " << logstream::error << "less USB devices connected than expected" << std::endl;
  }
  else if (cnt > MIN_DEVICE_NUM) {

    /* some storage or config devices detected */     
    /* identify the devices */
    for (i = 0; i < cnt; i++) {
      dev = all_devs[i];
      r = libusb_get_device_descriptor(dev, &desc);
      if (r < 0) {
	clog << "error: " << logstream::error << "get device descriptor error for libusb" << std::endl;
      }

      /* require bDeviceClass as not a hub or vendor specified (cameras)
	 and presence on STORAGE_BUS */
      if (libusb_get_bus_number(dev) == STORAGE_BUS
	  && desc.bDeviceClass != LIBUSB_CLASS_HUB
	  && desc.bDeviceClass != LIBUSB_CLASS_VENDOR_SPEC) {

	num_storage_dev++;
      }
    }
  }

  /* log number of connected storage USBs */
  clog << "info: " << logstream::info << "There are " << num_storage_dev << " storage devices connected" << std::endl;
  
  /* clean up */
  libusb_free_device_list(all_devs, 1);
  libusb_exit(ctx);
  
  return num_storage_dev;  
}

/* define data backup based on LookupUsbStorage() */
int UsbManager::DataBackup() {

  int ret = 0;
  std::string cmd;
  std::string log_path(LOG_DIR);
  std::string inotify_log = "/inotify.log";
  std::string mp_0(USB_MOUNTPOINT_0);
  std::string mp_1(USB_MOUNTPOINT_1);

  clog << "info: " << logstream::info << "defining data backup procedure" << std::endl;
  this->num_storage_dev = LookupUsbStorage();
  
  /* require 2+ storage devices for backup */
  if (this->num_storage_dev >= 2 && this->num_storage_dev != N_USB_UNDEF) {

    /* run backup */
    /* synchronise /media/usb0 to /media/usb1 */
    cmd = "while inotifywait -m -r -e modify,create,delete -o "
      + log_path + inotify_log + " " + mp_0 +
      "; do rsync -avz " + mp_0 + " " + mp_1 + "; done"; 

    clog << "info: " << logstream::info << "running backup with: " << cmd << std::endl;
 
    const char * command = cmd.c_str();
    ret = system(command);
    if (ret == 0) {
      clog << "info: " << logstream::info << "the following: " << cmd << " exited successfully" << std::endl;
    }
    else{
      clog << "error: " << logstream::error << "the following: " << cmd << " exited with an error" << std::endl;
    }
    
  }
  else {
    clog << "info: " << logstream::info << "not enough storage devices for backup" << std::endl;
  }

  

  return 0;
}


/* spawn thread to run data backup in the background */
int UsbManager::RunDataBackup() {

   clog << "info: " << logstream::info << "running data backup in the background" << std::endl;
  
  /* run the backup */
  std::thread run_backup (&UsbManager::DataBackup, this);

  /* store the thread handle */
  this->backup_thread_handle = run_backup.native_handle();

  /* detach */
  run_backup.detach();
  
  return 0;
}

/* kill the data backup thread */
int UsbManager::KillDataBackup() {

  clog << "info: " << logstream::info << "killing the data backup thread" << std::endl;
  
  /* kill the thread */
  /* justifiable as no locked resources */
  pthread_cancel(this->backup_thread_handle);
  
  return 0;
}
