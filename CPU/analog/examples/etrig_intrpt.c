/**
    @file

    @brief
        Demonstrates the use of the External Trigger rising/falling interrupts.

    @verbatim

    This program uses UTC1 out to set off the interrupts.  User Timer/Counter 1
    Out Pin must be routed to External Trigger Pin.  This should cause an
    interrupt to be received every second.

    @endverbatim

    @verbatim
    --------------------------------------------------------------------------
    This file and its contents are copyright (C) RTD Embedded Technologies,
    Inc.  All Rights Reserved.

    This software is licensed as described in the RTD End-User Software License
    Agreement.  For a copy of this agreement, refer to the file LICENSE.TXT
    (which should be included with this software) or contact RTD Embedded
    Technologies, Inc.
    --------------------------------------------------------------------------
    @endverbatim

		$Id: etrig_intrpt.c 99065 2016-04-26 18:03:23Z rgroner $
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
/**
 * Program name as invoked on the command line.
 */
static char *program_name;
/**
 * Board's device descriptor
 */
DM75xx_Board_Descriptor *board;
/**
 * Value to denote rising edge interrupts
 */
#define RISING_EDGE 0
/**
 * Value to denote falling edge interrupts
 */
#define FALLING_EDGE 1
/**
 * Global used to keep track of the number of interrupts received.
 */
uint8_t interrupts;
/**
 * Global used to indicate which edge value we will interrupt on.
 */
uint16_t edge_val;
/**
*******************************************************************************
@brief
    Print information on stderr about how the program is to be used.  After
    doing so, the program is exited.
 *******************************************************************************
*/

static void usage(void)
{
	fprintf(stderr, "\n");
	fprintf(stderr, "%s\n", program_name);
	fprintf(stderr, "\n");
	fprintf(stderr, "Usage: %s [--help] --minor MINOR\n", program_name);
	fprintf(stderr, "\n");
	fprintf(stderr,
		"    --help:     Display usage information and exit.\n");
	fprintf(stderr, "\n");
	fprintf(stderr,
		"    --minor:    Use specified DM75xx device number.\n");
	fprintf(stderr, "        MINOR:        Device minor number (>= 0).\n");
	fprintf(stderr,
		"    --edge:     Edge which will set off the interrupt.\n");
	fprintf(stderr, "        EDGE:         0 - Rising | 1 - Falling.\n");
	fprintf(stderr, "\n");
	exit(EXIT_FAILURE);
}

/**
 @brief User-Space ISR
 **/
void ISR(uint32_t status)
{
	if (DM75xx_INTERRUPT_ACTIVE(status, edge_val)) {
		interrupts++;
	}
}

/**
********************************************************************************
@brief
	Main program code.
********************************************************************************
*/
int main(int argument_count, char **arguments)
{
	DM75xx_Error dm75xx_status;
	int status;
	char *edge_string = NULL;

	edge_val = 0x0000;

	interrupts = 0;

	program_name = arguments[0];

	struct option options[] = {
		{"help", 0, 0, 1},
		{"minor", 1, 0, 2},
		{"edge", 1, 0, 3},
		{0, 0, 0, 0}
	};

	uint8_t help_option = 0x00;
	uint8_t minor_option = 0x00;
	uint8_t edge_option = 0x00;

	unsigned long int minor_number = 0;
	unsigned long int edge = 0;

	fprintf(stdout,
		"\n\tDM75xx External Trigger Interrupt Example Program\n\n");
	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Process command line options
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
	while (1) {
		/*
		 * Parse the next command line option and any arguments it may require.
		 */
		status = getopt_long(argument_count,
				     arguments, "", options, NULL);
		/*
		 * If getopt_long() returned -1, then all options have been processed.
		 */
		if (status == -1) {
			break;
		}
		/*
		 * Figure out what getop_long() found
		 */
		switch (status) {
			/*
			 * User Entered '--help'
			 */
		case 1:
			/*
			 * Deny multiple '--help' options
			 */
			if (help_option) {
				error(0, 0,
				      "ERROR: Dupliciate option '--help'");
				usage();
			}
			/*
			 * '--help' option has been acknowledged
			 */
			help_option = 0xFF;
			break;
			/*
			 * user entered '--minor'
			 */
		case 2:{
				char *minor_string;
				/*
				 * Deny mulitple '--minor' options
				 */
				if (minor_option) {
					error(0, 0,
					      "ERROR: Duplicate option '--minor'");
					usage();
				}
				/*
				 * Convert option argument string to unsigned long integer.
				 */
				errno = 0;
				minor_number =
				    strtoul(optarg, &minor_string, 10);
				/*
				 * Catch unsigned long int overflow
				 */
				if ((minor_number == ULONG_MAX)
				    && (errno == ERANGE)) {
					error(0, 0,
					      "ERROR: Device minor caused numeric overflow");
					usage();
				}
				/*
				 * Catch invalid argument strings
				 */
				if ((*minor_string != '\0')
				    || (minor_string == optarg)) {
					error(0, 0,
					      "ERROR: Invalid minor number");
					usage();
				}
				/*
				 * '--minor' option has been acknowledged
				 */
				minor_option = 0xFF;
				break;
			}
			/*
			 * User entered unsupported option
			 */
		case 3:{
				char *edge_string;
				/*
				 * Deny multiple '--edge' options.
				 */
				if (edge_option) {
					error(0, 0,
					      "ERROR: Duplicate option '--edge'");
					usage();
				}
				/*
				 * Convert option argument string to unsigned long integer
				 */
				errno = 0;
				edge = strtoul(optarg, &edge_string, 10);
				/*
				 * Catch unsigned long int overflow
				 */
				if ((edge == ULONG_MAX) && (errno = ERANGE)) {
					error(0, 0,
					      "ERROR: Invalid edge value");
					usage();
				}
				/*
				 * Catch invalid argument strings
				 */
				if ((*edge_string != '\0')
				    || (edge_string == optarg)) {
					error(0, 0,
					      "ERROR: Invalid edge value");
					usage();
				}
				/*
				 * '--edge' option has been acknowledged
				 */
				edge_option = 0xFF;
				break;
			}
		case '?':
			usage();
			break;
			/*
			 * getop_long() returned an unexpected value
			 */
		default:
			error(EXIT_FAILURE, 0,
			      "ERROR: getop_long() returned unexpected value%#x",
			      status);
			break;
		}
	}
	/*
	 * Handle '--help' option
	 */
	if (help_option) {
		usage();
	}
	/*
	 * '--minor' option must be given
	 */
	if (!minor_option) {
		error(0, 0, "Error: Option '--minor' is required");
		usage();
	}
	/*
	 * '--edge' option must be given and validated
	 */
	if (!edge_option) {
		error(0, 0, "Error: Option '--edge' is required");
		usage();
	} else if (edge == 0) {
		edge_val = DM75xx_INT_ETRIG_RISING;
		edge_string = "Rising";
	} else if (edge == 1) {
		edge_val = DM75xx_INT_ETRIG_FALLING;
		edge_string = "Falling";
	} else {
		error(0, 0, "Error: Invalid '--edge' value");
		usage();
	}
	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Device initialization
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
	/*
	 * Open the dm75xx device.
	 */
	fprintf(stdout, "Opening dm75xx with minor number %lu ...\n",
		minor_number);
	dm75xx_status = DM75xx_Board_Open(minor_number, &board);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_Board_Open");
	/*
	 * Initialize the board.
	 */
	fprintf(stdout, "Board Initialization ...\n");
	dm75xx_status = DM75xx_Board_Init(board);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_Board_Init");
	/*
	 * Setup User Timer/Counter 0 to 800Hz
	 */
	fprintf(stdout, "User Timer/Counter 0 Setup: 800Hz ... \n");
	dm75xx_status = DM75xx_UTC_Setup(board,
					 DM75xx_UTC_0,
					 DM75xx_CUTC_8_MHZ,
					 DM75xx_GUTC_NOT_GATED,
					 DM75xx_UTC_MODE_RATE_GENERATOR,
					 (uint16_t) 8000);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_UTC_Setup");
	/*
	 * Setup User Timer/Counter 1 to 1Hz
	 */
	fprintf(stdout, "User Timer/Counter 1 Setup: 1Hz ... \n");
	dm75xx_status = DM75xx_UTC_Setup(board,
					 DM75xx_UTC_1,
					 DM75xx_CUTC_UTC_0_OUT,
					 DM75xx_GUTC_NOT_GATED,
					 DM75xx_UTC_MODE_RATE_GENERATOR,
					 (uint16_t) 1000);

	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_UTC_Setup");
	/*
	 * Enable interrupt based on user choice.
	 */
	if (edge == RISING_EDGE) {
		/*
		 * Enable External Trigger Rising Interrupt
		 */
		fprintf(stdout,
			"Enabling External Trigger Rising Interrupt ... \n");
		dm75xx_status =
		    DM75xx_Interrupt_Enable(board, DM75xx_INT_ETRIG_RISING);
		DM75xx_Exit_On_Error(board, dm75xx_status,
				     "DM75xx_Interrupt_Enable");
	} else if (edge == FALLING_EDGE) {
		/*
		 * Enable External Trigger Falling Interrupt
		 */
		fprintf(stdout,
			"Enabling External Trigger Falling Interrupt ... \n");
		dm75xx_status =
		    DM75xx_Interrupt_Enable(board, DM75xx_INT_ETRIG_FALLING);
		DM75xx_Exit_On_Error(board, dm75xx_status,
				     "DM75xx_Interrupt_Enable");
	} else {
		error(0, 0, "ERROR: Invalid '--edge' value");
		usage();
	}
	/*
	 * Installing User-Space ISR
	 */
	fprintf(stdout, "Installing User-Space ISR ... \n");
	dm75xx_status = DM75xx_InstallISR(board, ISR);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_InstallISR");

	/*##########################################################################
	   Main Program Code
	   ###################################################################### */
	/*
	 * Wait for and test the interrupts as they are received.
	 */

	fprintf(stdout, "Waiting for %s edge interrupts \n", edge_string);
	int temp_ints;
	do {
		/*
		 * Cache a copy of the number of interrupts
		 */
		temp_ints = interrupts;
		fprintf(stdout, "%d Received ! \r", temp_ints);
		fflush(stdout);

		usleep(100);
	} while (temp_ints < 10);
	/*
	 * Set User Timer/Counter 0 gate to logic 1 to stop the clock
	 */
	fprintf(stdout, "\nStopping User Timer / Counter 0... \n");
	dm75xx_status =
	    DM75xx_UTC_Set_Gate(board, DM75xx_UTC_0, DM75xx_GUTC_GATED);
	DM75xx_Exit_On_Error(board, dm75xx_status, " DM75xx_UTC_Set_Gate");
	/*
	 * Set User Timer/Counter 1 gate to logic 1 to stop the clock
	 */
	fprintf(stdout, "Stopping User Timer / Counter 1... \n");
	dm75xx_status =
	    DM75xx_UTC_Set_Gate(board, DM75xx_UTC_1, DM75xx_GUTC_GATED);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_UTC_Set_Gate");
	/*
	 * Remove User-Space ISR
	 */
	fprintf(stdout, "Removing User - Space ISR... \n");
	dm75xx_status = DM75xx_RemoveISR(board);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_RemoveISR");
	/*
	 * Reset the board.
	 */
	fprintf(stdout, "Board Reset... \n");
	dm75xx_status = DM75xx_Board_Reset(board);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_Board_Reset");
	/*
	 * Close the dm75xx device.
	 */
	fprintf(stdout, "Closing the dm75xx... \n");
	dm75xx_status = DM75xx_Board_Close(board);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_Board_Close");
	exit(EXIT_SUCCESS);
}
