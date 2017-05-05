/**
    @file

    @brief
        Demonstrates the use of Digital to Analog conversion.

    @verbatim

    This example program produces a Sine Wave on DAC1 and a Saw-Toothed wave on
    DAC2.

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

		$Id: dac.c 99065 2016-04-26 18:03:23Z rgroner $
*/

#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <math.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dm75xx_library.h"
/**
 * Program name as invoked on the command line
 */
static char *program_name;
/**
 * Sample rate
 */
#define DAC_RATE 10000
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
********************************************************************************
@brief
	Main program code.
********************************************************************************
*/
int main(int argument_count, char **arguments)
{
	DM75xx_Board_Descriptor *board;
	DM75xx_Error dm75xx_status;
	int status, i;
	float temp;
	uint16_t sample = 0x000;

	program_name = arguments[0];

	struct option options[] = {
		{"help", 0, 0, 1},
		{"minor", 1, 0, 2},
		{0, 0, 0, 0}
	};

	uint8_t help_option = 0x00;
	uint8_t minor_option = 0x00;

	unsigned long int minor_number = 0;

	fprintf(stdout, "\n\tDM75xx Digital to Analog Example Program\n\n");
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
			/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
			 * User Entered '--help'
			 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
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
			/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
			 * user entered '--minor'
			 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
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
			/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
			 * User entered unsupported option
			 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
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
	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Main program code.
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
	/*
	 * Create DAC1 waveform sinusoidal
	 */
	for (i = 0; i < 1024; i++) {
		temp = 2047 * (sin((float)i * (2 * M_PI / (float)1024)));
		sample = DM75xx_DAC_PACK_DATA(temp, 0x00, 0x00);
		dm75xx_status =
		    DM75xx_DAC_FIFO_Write(board, DM75xx_DAC1, sample);
		DM75xx_Exit_On_Error(board, dm75xx_status,
				     "DM75xx_DAC_FIFO_Write");
	}
	/*
	 * Create DAC2 waveform sawtooth
	 */
	for (i = 0; i < 1024; i++) {
		temp = (int)(4095. * i / 1024) - 2047;
		sample = DM75xx_DAC_PACK_DATA(temp, 0x00, 0x00);

		dm75xx_status =
		    DM75xx_DAC_FIFO_Write(board, DM75xx_DAC2, sample);
		DM75xx_Exit_On_Error(board, dm75xx_status,
				     "DM75xx_DAC_FIFO_Write");
	}
	/*
	 * Setup DAC 1
	 */
	fprintf(stdout, "Setup DAC1 ... \n");
	dm75xx_status =
	    DM75xx_DAC_Setup(board, DM75xx_DAC1, DM75xx_DAC_RANGE_BIPOLAR_5,
			     DM75xx_DAC_UPDATE_CLOCK, DM75xx_DAC_MODE_CYCLE);
	/*
	 * Setup DAC 2
	 */
	fprintf(stdout, "Setup DAC2 ... \n");
	dm75xx_status =
	    DM75xx_DAC_Setup(board, DM75xx_DAC2, DM75xx_DAC_RANGE_BIPOLAR_5,
			     DM75xx_DAC_UPDATE_CLOCK, DM75xx_DAC_MODE_CYCLE);
	/*
	 * Set User Output Signal 0 to DAC1 Conversion
	 */
	fprintf(stdout, "Setting User Output Signal 0... \n");
	dm75xx_status = DM75xx_UIO_Select(board, DM75xx_UIO0, DM75xx_UIO_DAC1);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_UIO_Select");
	/*
	 * Set User Output Signal 1 to DAC2 Conversion
	 */
	fprintf(stdout, "Setting User Output Signal 1... \n");
	dm75xx_status = DM75xx_UIO_Select(board, DM75xx_UIO1, DM75xx_UIO_DAC2);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_UIO_Select");
	/*
	 * Set DAC Rate
	 */
	fprintf(stdout, "Setting DAC Rate ... \n");
	dm75xx_status =
	    DM75xx_DAC_Set_Rate(board, DM75xx_DAC_FREQ_8_MHZ, DAC_RATE, &temp);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DAC_Set_Rate");
	fprintf(stdout, "DAC Rate set to %6.2f\n", temp);

	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Program clean up
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
	/*
	 * Close the dm75xx device.
	 */
	fprintf(stdout, "Closing the dm75xx ... \n");
	dm75xx_status = DM75xx_Board_Close(board);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_Board_Close");
	exit(EXIT_SUCCESS);
}
