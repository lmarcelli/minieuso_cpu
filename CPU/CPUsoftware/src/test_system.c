/*-------------------------------\
                                 |
TEST CPU PROGRAM                 |
                                 |
Launch all the systems			 |
PDM, cameras and photodiode 	 |
                                 | 
Francesca Capel                  |
capel.francesca@gmail.com        | 
                                 |
--------------------------------*/
#include "globals.h"

int main(void) {
  
	/* start-up */  

	/* create the log file */
	create_log();
	tlog("SYS","LOG CREATED");
	printf("log created\n");

	/* check for usb and mount */
	mount_usb(USB_PORT);
	printf("mount usb\n");

	/* check communication with systems */
	check_IP_com(ZYNQ_IP_ADDRESS);
	printf("checked IP\n");
	/* launch different systems */



	return 0; 
}


  
