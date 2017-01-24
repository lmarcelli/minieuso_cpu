/*-------------------------------\
                                 |
MAIN CPU PROGRAM                 |
                                 |
Executed on startup              | 
Loads configuration files        |  
Controls data acquisition mode   | 
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

	/* check for usb and mount */
	mount_usb(USB_PORT);

	/* power on systems */
	//TBC IF NEEDED

	/* check the systems */
	/* HK */
	/* Cameras */
	/* Zynq board (HV) */
	
	/* load the configuration file */

	/* set up the FTP server */

	/* set operational mode */

	/* end program */

	return 0; 
}


  
