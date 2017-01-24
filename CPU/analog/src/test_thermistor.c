/*
F. Capel
capel.francesca@gmail.com

Testing the readout of the OneWire thermistors using the DM75xx analog board.
Simple Program to implement the OneWire reset protocol.

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
#include <sched.h>

#include "dm75xx_library.h"

DM75xx_Board_Descriptor *board;

/* OneWire Reset function */
/**************************/
uint8_t OneWire_reset(void) {

	DM75xx_Error dm75xx_status;
	uint8_t *r = malloc(8);

	/* Could add interrupt to wait until the wire is high before beginning... */

	/* Configure Port 1 Direction to output */
	fprintf(stdout, "Configuring Port 1 Direction to output ... \n");
	dm75xx_status = DM75xx_DIO_Set_Direction(board, DM75xx_DIO_PORT1, 0xFF);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DIO_Set_Direction");

	/* Drive output low */
	DM75xx_DIO_Set_Port(board, DM75xx_DIO_PORT1, 0x00);
	DM75xx_Exit_On_Error(board, dm75xx_status,
				     "DM75xx_DIO_Set_Port");

	nanosleep((const struct timespec[]){{0, 480000L}}, NULL); // 480 us

	/* Allow it to float */
	fprintf(stdout, "Configuring Port 1 Direction to input ... \n");
	dm75xx_status = DM75xx_DIO_Set_Direction(board, DM75xx_DIO_PORT1, 0x00);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DIO_Set_Direction");

	nanosleep((const struct timespec[]){{0, 70000L}}, NULL); // 70 us

	/* Read the value on the input */
	DM75xx_DIO_Get_Port(board, DM75xx_DIO_PORT1, r);
	DM75xx_Exit_On_Error(board, dm75xx_status,
				     "DM75xx_DIO_Get_Port");
	fprintf(stdout, "Reading port 1 output: %2x \n", *r);

	free(r);
	return 0;
}

/* OneWire Write bit function */
/******************************/
uint8_t OneWire_writebit(uint8_t v) {

	DM75xx_Error dm75xx_status;

	if (v & 1) {
		
		dm75xx_status = DM75xx_DIO_Set_Direction(board, DM75xx_DIO_PORT1, 0xFF); 
		DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DIO_Set_Direction");
		DM75xx_DIO_Set_Port(board, DM75xx_DIO_PORT1, 0x00); /* Drive output low */
		DM75xx_Exit_On_Error(board, dm75xx_status,
				     "DM75xx_DIO_Set_Port");
		nanosleep((const struct timespec[]){{0, 10000L}}, NULL); // 10 us
		DM75xx_DIO_Set_Port(board, DM75xx_DIO_PORT1, 0xFF); /* Drive output high */
		nanosleep((const struct timespec[]){{0, 55000L}}, NULL); // 55 us

	} else {

		dm75xx_status = DM75xx_DIO_Set_Direction(board, DM75xx_DIO_PORT1, 0xFF); 
		DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DIO_Set_Direction");
		DM75xx_DIO_Set_Port(board, DM75xx_DIO_PORT1, 0x00); /* Drive output low */
		DM75xx_Exit_On_Error(board, dm75xx_status,
				     "DM75xx_DIO_Set_Port");
		nanosleep((const struct timespec[]){{0, 65000L}}, NULL); // 65 us
		DM75xx_DIO_Set_Port(board, DM75xx_DIO_PORT1, 0xFF); /* Drive output high */
		nanosleep((const struct timespec[]){{0, 5000L}}, NULL); // 5 us

	}
	return 0;
}


/* Main program code */
/*********************/
int main() {

	DM75xx_Error dm75xx_status;
	unsigned long int minor_number = 0;
	struct sched_param param;

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

	/* Set to real-time process */
	param.sched_priority = 99;
	if (sched_setscheduler(0, SCHED_FIFO, & param) != 0) {
    	perror("sched_setscheduler");
		exit(EXIT_FAILURE);  
	}
	else {
		fprintf(stdout, "Set to real-time schedule ...\n");
	}

	/* Configure the digital I/O ports */
	/***********************************/
	
	/* Set DIO Clock */
	fprintf(stdout, "Setting DIO Sample Clock to 8MHz ... \n");
	dm75xx_status = DM75xx_DIO_Clock(board, DM75xx_DIO_CLK_8MHZ);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DIO_Clock");
	
	/* Main actions */
	/****************/

	//OneWire_reset();
	OneWire_writebit(1);

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