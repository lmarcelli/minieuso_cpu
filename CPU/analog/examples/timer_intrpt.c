/**
    @file

    @brief
        Demonstrates the use of the User Timer/Counter Out Interrupts.

    @verbatim

    UTC0 is set to 1kHz and UTC1 is set to 1Hz.  An interrupt will occur every
    UTC1 out and UTC1 Inverted Out.  This should end up causing an interrupt.
    Every second from each source but the interrupts will be shifted half a
    second apart. The program counts to ten interrupts, on each channel.

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

		$Id: timer_intrpt.c 99065 2016-04-26 18:03:23Z rgroner $
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
 * Rate of user timer/counter 0
 */
#define UTC0 8000
/**
 * Rate of user timer/counter 1
 */
#define UTC1 2000
/**
 * Program name as invoked on the command line.
 */
static char *program_name;
/**
 * Board's device descriptor
 */
DM75xx_Board_Descriptor *board;
/**
 * Global used to keep track of the number of interrupts received.
 */
uint8_t utc1_int;
/**
 * Global used to keep track of the number of interrupts received.
 */
uint8_t utc1_int_inverted;
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
	if (DM75xx_INTERRUPT_ACTIVE(status, DM75xx_INT_UTC1)) {
		utc1_int++;
	}

	if (DM75xx_INTERRUPT_ACTIVE(status, DM75xx_INT_UTC1_INV)) {
		utc1_int_inverted++;
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

	utc1_int = 0;
	utc1_int_inverted = 0;

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
		"\n\tDM75xx User Timer/Counter Interrupt Example Program\n\n");
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
	if (minor_option == 0x00) {
		error(0, 0, "Error: Option '--minor' is required");
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
	/*##########################################################################
	   Main Program Code
	   ###################################################################### */
	/*
	 * Setup User Timer/Counter 0
	 */
	fprintf(stdout, "User Timer/Counter 0 Setup ... \n");
	dm75xx_status = DM75xx_UTC_Setup(board,
					 DM75xx_UTC_0,
					 DM75xx_CUTC_8_MHZ,
					 DM75xx_GUTC_NOT_GATED,
					 DM75xx_UTC_MODE_RATE_GENERATOR,
					 (uint16_t) UTC0);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_UTC_Setup");
	/*
	 * Setup User Timer/Counter 1
	 */
	fprintf(stdout, "User Timer/Counter 1 Setup ... \n");
	dm75xx_status = DM75xx_UTC_Setup(board,
					 DM75xx_UTC_1,
					 DM75xx_CUTC_UTC_0_OUT,
					 DM75xx_GUTC_NOT_GATED,
					 DM75xx_UTC_MODE_SQUARE_WAVE,
					 (uint16_t) UTC1);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_UTC_Setup");
	/*
	 * Enable UTC1 Out and ~UTC1 Out Interrupt
	 */
	fprintf(stdout, "Enabling UTC1 and ~UTC1 Interrupts ... \n");
	dm75xx_status = DM75xx_Interrupt_Enable(board,
						DM75xx_INT_UTC1 |
						DM75xx_INT_UTC1_INV);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_Interrupt_Enable");
	/*
	 * Installing User-Space ISR
	 */
	fprintf(stdout, "Installing User-Space ISR ... \n");
	dm75xx_status = DM75xx_InstallISR(board, ISR);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_InstallISR");

	/*##########################################################################
	   Show Elapsed Time
	   ###################################################################### */
	/*
	 * Read the Clock
	 */
	fprintf(stdout, "\nWaiting to receive interrupts... \n\n");

	int temp1 = utc1_int;
	int temp2 = utc1_int_inverted;

	do {

		if (temp1 != utc1_int || temp2 != utc1_int_inverted) {

			temp1 = utc1_int;
			temp2 = utc1_int_inverted;

			fprintf(stdout, "UTC1: %d | ~UTC1: %d \r", temp1,
				temp2);
			fflush(stdout);
		}

		usleep(100);

	} while (temp1 < 10);

	fprintf(stdout, "\n \n");
	/*
	 * Set User Timer/Counter 0 gate to logic 1 to stop the clock
	 */
	fprintf(stdout, "Stopping User Timer / Counter 0... \n");
	dm75xx_status =
	    DM75xx_UTC_Set_Gate(board, DM75xx_UTC_0, DM75xx_GUTC_GATED);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_UTC_Set_Gate");
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
	fprintf(stdout, "Removing User-Space ISR... \n");
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
