/*
F. Capel
capel.francesca@gmail.com

Testing the sending of 5V pulses for 10 ms using the DM75xx analog interface board. 
Such pulses will be used to turn on and off the subsystems via the LVPS.

*/

#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dm75xx_library.h"

DM75xx_Board_Descriptor *board;

/* Main program code */
/*********************/
int main() {

	DM75xx_Error dm75xx_status;
	unsigned long int minor_number = 0;
	uint8_t high = 0xFF;
	uint8_t low = 0x00;

	/* Initialisation */
	/******************/

	/* Open the dm75xx device */
	fprintf(stdout, "Opening dm75xx with minor number %lu ...\n",
		minor_number);
	dm75xx_status = DM75xx_Board_Open(minor_number, &board);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_Board_Open");
	
	 /* Initialize the board */
	fprintf(stdout, "Board Initialisation ...\n");
	dm75xx_status = DM75xx_Board_Init(board);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_Board_Init");

	/* Configure the digital I/O ports */
	/***********************************/
	
	/* Set DIO Clock */
	fprintf(stdout, "Setting DIO Sample Clock to 8MHz ... \n");
	dm75xx_status = DM75xx_DIO_Clock(board, DM75xx_DIO_CLK_8MHZ);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DIO_Clock");

	/* Configure Port 0 Direction */
	fprintf(stdout, "Configuring Port 0 Direction to 1111 1111 ... \n");
	dm75xx_status = DM75xx_DIO_Set_Direction(board, DM75xx_DIO_PORT0, 0xFF);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DIO_Set_Direction");
	
	/* Configure Port 1 Direction */
	fprintf(stdout, "Configuring Port 1 Direction to output ... \n");
	dm75xx_status = DM75xx_DIO_Set_Direction(board, DM75xx_DIO_PORT1, 0xFF);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DIO_Set_Direction");

	/* Drive the output high on Port 0 for 10 ms */
	DM75xx_DIO_Set_Port(board, DM75xx_DIO_PORT0, high);
	DM75xx_Exit_On_Error(board, dm75xx_status,
			     "DM75xx_DIO_Set_Port");
	fprintf(stdout, "Port 0 Output:0x%2x \n", high);
	nanosleep((const struct timespec[]){{0, 10000000L}}, NULL); // 10 ms

	/* Drive the output low on Port 0 */ 
	DM75xx_DIO_Set_Port(board, DM75xx_DIO_PORT0, low);
	DM75xx_Exit_On_Error(board, dm75xx_status,
			     "DM75xx_DIO_Set_Port");
	fprintf(stdout, "Port 0 Output:0x%2x \n", low);

	/* wait for user input */
	fprintf(stdout, "Press enter to continue...");
	getchar();

	/* Clean up */
	/************/
	
	/* Reset the board */
	fprintf(stdout, "Board Reset... \n");
	dm75xx_status = DM75xx_Board_Reset(board);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_Board_Reset");
	
	/* Close the dm75xx device */
	fprintf(stdout, "Closing the dm75xx... \n");
	dm75xx_status = DM75xx_Board_Close(board);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_Board_Close");
	exit(EXIT_SUCCESS);
	//return 0;
}
