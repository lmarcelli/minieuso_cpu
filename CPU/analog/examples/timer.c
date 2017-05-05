/**
    @file

    @brief
        Demonstrates the use of the User Timer/Counters for keeping time.

    @verbatim

    This example program demonstrates using the 8254 User Timer/Counters as a
    simple time keeper.

    UTC 1 is being fed by UTC 0

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

        $Id: timer.c 99065 2016-04-26 18:03:23Z rgroner $
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
 * Rate of the Timer.
 */
#define TIMER_RATE 200
/**
 * Program name as invoked on the command line.
 */
static char *program_name;
/**
 * Variable to allow graceful exit from Ctrl-C
 */
volatile uint8_t exit_program;

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
*******************************************************************************
@brief
    Exit gracefully if the user enters CTRL-C.
*******************************************************************************
*/
static void sigint_handler(int sig_num)
{
	if (sig_num != SIGINT) {
		error(EXIT_FAILURE, errno,
		      "Signal handler received unexpected signal");
	}
	exit_program = 1;
}

/**
********************************************************************************
@brief
    Main program code.
********************************************************************************
*/
int main(int argument_count, char **arguments)
{
	DM75xx_Board_Descriptor *board;
	DM75xx_Error dm75xx_status;
	struct sigaction sig_action;
	int status;
	uint16_t count = 0x0000;

	program_name = arguments[0];

	exit_program = 0x00;

	sig_action.sa_handler = sigint_handler;
	sigfillset(&sig_action.sa_mask);
	sig_action.sa_flags = 0;

	if (sigaction(SIGINT, &sig_action, NULL) < 0) {
		error(EXIT_FAILURE, errno, "sigaction() FAILED");
	}

	struct option options[] = {
		{"help", 0, 0, 1},
		{"minor", 1, 0, 2},
		{0, 0, 0, 0}
	};

	uint8_t help_option = 0x00;
	uint8_t minor_option = 0x00;

	unsigned long int minor_number = 0;

	fprintf(stdout, "\n\tDM75xx User Timer/Counter Example Program\n\n");
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
	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
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

	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Setup User Timer/Counters
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
	/*
	 * Setup User Timer/Counter 0
	 */
	fprintf(stdout, "User Timer/Counter 0 Setup ... \n");
	dm75xx_status = DM75xx_UTC_Setup(board,
					 DM75xx_UTC_0,
					 DM75xx_CUTC_8_MHZ,
					 DM75xx_GUTC_NOT_GATED,
					 DM75xx_UTC_MODE_SQUARE_WAVE,
					 (uint16_t) (8000000 / TIMER_RATE));
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_UTC_Setup");
	/*
	 * Setup User Timer/Counter 1
	 */
	fprintf(stdout, "User Timer/Counter 1 Setup ... \n");
	dm75xx_status = DM75xx_UTC_Setup(board,
					 DM75xx_UTC_1,
					 DM75xx_CUTC_UTC_0_OUT,
					 DM75xx_GUTC_NOT_GATED,
					 DM75xx_UTC_MODE_EVENT_COUNTER, 0xFFFF);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_UTC_Setup");

	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Show Elapsed Time
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
	/*
	 * Read the Clock
	 */
	fprintf(stdout, "\nPress Control-C to exit. \n");
	fprintf(stdout, "\nElapsed time (in seconds) ... \n");
	do {
		dm75xx_status = DM75xx_UTC_Get_Count(board,
						     DM75xx_UTC_1, &count);
		DM75xx_Exit_On_Error(board, dm75xx_status,
				     "DM75xx_UTC_Get_Count");

		fprintf(stdout, "     %2.2f   \r",
			((float)(0xFFFF - count) / TIMER_RATE));

	} while (!exit_program);
	fprintf(stdout, "\n\n");
	/*
	 * Set User Timer/Counter 0 gate on to stop the clock
	 */
	fprintf(stdout, "Stopping User Timer/Counter 0 ... \n");
	dm75xx_status =
	    DM75xx_UTC_Set_Gate(board, DM75xx_UTC_0, DM75xx_GUTC_GATED);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_UTC_Set_Gate");
	/*
	 * Set User Timer/Counter 1 gate on to stop the clock
	 */
	fprintf(stdout, "Stopping User Timer/Counter 1 ... \n");
	dm75xx_status =
	    DM75xx_UTC_Set_Gate(board, DM75xx_UTC_1, DM75xx_GUTC_GATED);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_UTC_Set_Gate");
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
