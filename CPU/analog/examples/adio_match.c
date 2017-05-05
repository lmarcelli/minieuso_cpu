/**
    @file

    @brief
        Demonstrates the use of the Digital I/O Match Mode to generate
        interrupts.

    @verbatim

    The compare register is first set to 0xAB.  When this
    value is written out Port1 and received by Port0 an interrupt is
    received.  In the user-space ISR, the DIO interrupt is cleared and the
    compare register is changed to 0x3C.  When 0x3C is written out Port1
    and received by Port0 another interrupt is received.

    Port0 must be connected to Port1 bit-per-bit (Port0 bit 0 connected to
    Port1 bit 0 and so on ...).

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

		$Id: adio_match.c 99065 2016-04-26 18:03:23Z rgroner $
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
 * Variable to count the number of interrupts
 */
int interrupts;
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
	fprintf(stderr, "\n");
	exit(EXIT_FAILURE);
}

/**
 @brief User-Space ISR
 **/
void ISR(uint32_t status)
{
	DM75xx_Error dm75xx_status;
	if (DM75xx_INTERRUPT_ACTIVE(status, DM75xx_INT_DIO)) {
		interrupts++;
		/*
		 * Clear Digital I/O IRQ
		 */
		dm75xx_status = DM75xx_DIO_Clear_IRQ(board);
		DM75xx_Exit_On_Error(board, dm75xx_status,
				     "DM75xx_DIO_Clear_IRQ");
		/*
		 * Configure Compare Register
		 */
		dm75xx_status = DM75xx_DIO_Set_Compare(board, 0x3c);
		DM75xx_Exit_On_Error(board, dm75xx_status,
				     "DM75xx_DIO_Set_Compare");
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
	int status, i;
	uint8_t vals[] = { 0x00, 0xFF, 0xCC, 0xBA, 0xAB,
		0x12, 0xF3, 0x3F, 0x60, 0x3C
	};

	interrupts = 0;

	program_name = arguments[0];

	struct option options[] = {
		{"help", 0, 0, 1},
		{"minor", 1, 0, 2},
		{0, 0, 0, 0}
	};

	uint8_t help_option = 0x00;
	uint8_t minor_option = 0x00;

	unsigned long int minor_number = 0;

	fprintf(stdout,
		"\n\tDM75xx Digital I/O Match Mode Interrupt Example Program\n\n");
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

	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Device initialization
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
	/*
	 * Open the dm75xx device.
	 */
	fprintf(stdout, "Opening dm75xx with minor number %lu  ...\n",
		minor_number);
	dm75xx_status = DM75xx_Board_Open(minor_number, &board);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_Board_Open");
	/*
	 * Initialize the board.
	 */
	fprintf(stdout, "Board Initialization  ...\n");
	dm75xx_status = DM75xx_Board_Init(board);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_Board_Init");
	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Enable Interrupt and User-Space ISR
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
	/*
	 * Enable Digital Interrupt Interrupt
	 */
	fprintf(stdout, "Enabling Digital Interrupt  ... \n");
	dm75xx_status = DM75xx_Interrupt_Enable(board, DM75xx_INT_DIO);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_Interrupt_Enable");
	/*
	 * Installing User-Space ISR
	 */
	fprintf(stdout, "Installing User-Space ISR  ... \n");
	dm75xx_status = DM75xx_InstallISR(board, ISR);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_InstallISR");
	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Setup Digital Input/Output Configuration
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
	/*
	 * Set DIO IRQ Mode
	 */
	fprintf(stdout, "Setting DIO Match Mode ... \n");
	dm75xx_status = DM75xx_DIO_IRQ_Mode(board, DM75xx_DIO_MODE_MATCH);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DIO_IRQ_Mode");
	/*
	 * Set DIO Clock
	 */
	fprintf(stdout, "Setting DIO Sample Clock to 8 MHz ... \n");
	dm75xx_status = DM75xx_DIO_Clock(board, DM75xx_DIO_CLK_8MHZ);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DIO_Clock");
	/*
	 * Enable DIO IRQ
	 */
	fprintf(stdout, "Enabling DIO Interrupts ... \n");
	dm75xx_status = DM75xx_DIO_Enable_IRQ(board, 0xFF);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DIO_Enable_IRQ");
	/*
	 * Configure Port 0 Direction
	 */
	fprintf(stdout, "Configuring Port 0 Direction ... \n");
	dm75xx_status = DM75xx_DIO_Set_Direction(board, DM75xx_DIO_PORT0, 0x00);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DIO_Set_Direction");
	/*
	 * Configure Port 1 Direction
	 */
	fprintf(stdout, "Configuring Port 1 Direction ... \n");
	dm75xx_status = DM75xx_DIO_Set_Direction(board, DM75xx_DIO_PORT1, 0xFF);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DIO_Set_Direction");
	/*
	 * Configure Compare Register
	 */
	fprintf(stdout, "Configuring Port 0 Compare Value ... \n");
	dm75xx_status = DM75xx_DIO_Set_Compare(board, 0xAB);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DIO_Set_Compare");
	/*##########################################################################
	   Main Program Code
	   ###################################################################### */
	/*
	 * Wait for and test the interrupts as they are received.
	 */
	for (i = 0; i < 10; i++) {
		DM75xx_DIO_Set_Port(board, DM75xx_DIO_PORT1, vals[i]);
		DM75xx_Exit_On_Error(board, dm75xx_status,
				     "DM75xx_DIO_Set_Port");
		fprintf(stdout, "Port 1 Output: 0x%2x Interrupts: %d \r",
			vals[i], interrupts);
		fflush(stdout);
		sleep(1);
	}
	fprintf(stdout, "Received Interrupts: %d \n", interrupts);
	/*
	 * Remove User-Space ISR
	 */
	fprintf(stdout, "Removing User-Space ISR ... \n");
	dm75xx_status = DM75xx_RemoveISR(board);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_RemoveISR");
	/*
	 * Reset the board.
	 */
	fprintf(stdout, "Board Reset ... \n");
	dm75xx_status = DM75xx_Board_Reset(board);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_Board_Reset");
	/*
	 * Close the dm75xx device.
	 */
	fprintf(stdout, "Closing the dm75xx ... \n");
	dm75xx_status = DM75xx_Board_Close(board);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_Board_Close");
	exit(EXIT_SUCCESS);
}
