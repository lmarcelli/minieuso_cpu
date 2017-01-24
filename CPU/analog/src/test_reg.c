/* 
F. Capel
capel.francesca@gmail.com

Code to test reading and writing to the DM75xx registers for DIO
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
	int status, i;
	unsigned long int minor_number = 0;
	uint8_t val;

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
	fprintf(stdout, "Configuring Port 0 Direction to input ... \n");
	dm75xx_status = DM75xx_DIO_Set_Direction(board, DM75xx_DIO_PORT0, 0x00);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DIO_Set_Direction");
	
	/* Configure Port 1 Direction */
	fprintf(stdout, "Configuring Port 1 Direction to output ... \n");
	dm75xx_status = DM75xx_DIO_Set_Direction(board, DM75xx_DIO_PORT1, 0xFF);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DIO_Set_Direction");

	/* Read the port 1 output */
	DM75xx_DIO_Get_Port(board, DM75xx_DIO_PORT1, val);
	DM75xx_Exit_On_Error(board, dm75xx_status,
				     "DM75xx_DIO_Get_Port");
	fprintf(stdout, "Reading port 1 output: %2x \n", val);

	sleep(2);

	/* Write a value to the output port 1 */
	DM75xx_DIO_Set_Port(board, DM75xx_DIO_PORT1, 0x00);
	DM75xx_Exit_On_Error(board, dm75xx_status,
				     "DM75xx_DIO_Set_Port");
	fprintf(stdout, "Writing ort 1 output: %2x \n", 0x00);

	sleep(2);

	/* Read the port 1 output */
	DM75xx_DIO_Get_Port(board, DM75xx_DIO_PORT1, val);
	DM75xx_Exit_On_Error(board, dm75xx_status,
				     "DM75xx_DIO_Get_Port");
	fprintf(stdout, "Reading port 1 output: %2x \n", val);

	sleep(2);

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
