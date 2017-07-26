#include "globals.h"

/* print a description of usb devices connected */
void printdev(libusb_device * dev) {

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
int check_usb() {
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
    printdev(devs[i]);
    std::cout << (int)libusb_get_port_number(devs[i]) << std::endl;
  }

  /* clean up */
  libusb_free_device_list(devs, 1);
  libusb_exit(ctx);
  
  return 0;
}

/* lookup usb devices connected and their ID */
int lookup_usb() {
  libusb_device ** all_devs;
  libusb_device * dev;
  libusb_context * ctx = NULL;
  int r;
  uint8_t port_nums[8];
  ssize_t cnt, i;
  size_t j;

  /* set up logging */
  std::ofstream log_file(log_name, std::ios::app);
  logstream clog(log_file, logstream::all);
  clog << "info: " << logstream::info << "looking up USB devices" << std::endl;
  
  /* initialise a libusb session */
  r = libusb_init(&ctx);
  if (r < 0) {
    std::cout << "init error for libusb" << std::endl;
    return 1;
  }

  /* set the verbosity level to 3, as suggested by docs */
  libusb_set_debug(ctx, 3);

  /* get the list of devices */
  cnt = libusb_get_device_list(ctx, &all_devs);
  if (cnt < 0) {
    std::cout << "get device error for libusb" << std::endl;
    clog << "error: " << logstream::error << "get device error for libusb" << std::endl;
  }

  /* identify the devices */
  for (i = 0; i < cnt; i++) {
    dev = all_devs[i];

    (int)libusb_get_port_numbers(dev, port_nums, sizeof(port_nums));
    
    std::cout << "dev address: " << (int)libusb_get_device_address(dev) << std::endl;
    std::cout << "bus no: " << (int)libusb_get_bus_number(dev) << std::endl;
    std::cout << "port no: " << (int)libusb_get_port_number(dev) << std::endl;
    std::cout << "port no.s "; 
    for (j = 0; j < sizeof(port_nums); j++) {
      std::cout << (int)port_nums[j]; 
    }
    std::cout << std::endl;
  }
  
  /* clean up */
  libusb_free_device_list(all_devs, 1);
  libusb_exit(ctx);
  
  return 0;  
}
