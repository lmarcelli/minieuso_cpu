/*
F. Capel
capel.francesca@gmail.com

Testing the readout of the OneWire thermistors using the DM75xx analog board.
Simple program to write values to the output pins of port 1

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
	int i;
	unsigned long int minor_number = 0;
	uint8_t vals[] = { 0x00, 0x00, 0x00, 0xFF, 0xFF,
		0xFF, 0x00, 0x00, 0x00, 0xFF};

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

	/* Write some values to the output port */ 
	for (i = 0; i < 10; i++) {
		DM75xx_DIO_Set_Port(board, DM75xx_DIO_PORT1, vals[i]);
		DM75xx_Exit_On_Error(board, dm75xx_status,
				     "DM75xx_DIO_Set_Port");
		sleep(2);
		fprintf(stdout, "Port 1 Output:0x%2x \n", vals[i]);
	}

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