/**
    @file

    @brief
        Demonstrates auto-calibration of SDM7540/8540

    @verbatim

        This program utilizes the on-board DSP to auto calibrate the A/D and D/A
        converters.

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

		$Id: calibrate.c 99065 2016-04-26 18:03:23Z rgroner $
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
#include <time.h>
#include <unistd.h>

#include "dm75xx_library.h"
/**
 * Name of the program as invoked on the command line.
 */
static char *program_name;
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
		"    --dac1range:  Range at which to calibrate DAC1.\n");
	fprintf(stderr,
		"        DAC1Range:    0 = 0 - 5V\n"
		"                      1 = 0 - 10V\n"
		"                      2 = +/- 5V\n"
		"                      3 = +/- 10V\n");
	fprintf(stderr,
		"    --dac1val:    Value to set DAC1 after calibration.\n");
	fprintf(stderr,
		"        DAC1VAL:      -2048 to 2047 bipolar, 0 to 4096 unipolar.\n");
	fprintf(stderr,
		"    --dac2range:  Range at which to calibrate DAC2.\n");
	fprintf(stderr,
		"        DAC2Range:    0 = 0 - 5V\n"
		"                      1 = 0 - 10V\n"
		"                      2 = +/- 5V\n"
		"                      3 = +/- 10V\n");
	fprintf(stderr,
		"    --dac2val:    Value to set DAC2 after calibration.\n");
	fprintf(stderr,
		"        DAC2VAL:      -2048 to 2047 bipolar, 0 to 4096 unipolar.\n");
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
	struct timespec before, after;
	uint8_t data;
	int status;

	program_name = arguments[0];

	struct option options[] = {
		{"help", 0, 0, 1},
		{"minor", 1, 0, 2},
		{"dac1range", 1, 0, 3},
		{"dac1val", 1, 0, 4},
		{"dac2range", 1, 0, 5},
		{"dac2val", 1, 0, 6},
		{0, 0, 0, 0}
	};

	uint8_t help_option = 0x00;
	uint8_t minor_option = 0x00;
	uint8_t dac1r_option = 0x00;
	uint8_t dac1v_option = 0x00;
	uint8_t dac2r_option = 0x00;
	uint8_t dac2v_option = 0x00;

	unsigned long int minor_number = 0;
	unsigned long int dac1_range = 0;
	unsigned long int dac2_range = 0;
	long int dac1_val = 0;
	long int dac2_val = 0;

	fprintf(stdout, "\n\tDM75xx AutoCal Example Program\n\n");
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
			 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
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
			 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
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
			 * user entered '--dac1range'
			 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
		case 3:{
				char *dac1range_string;
				/*
				 * Deny multiple '--dac1range' options
				 */
				if (dac1r_option) {
					error(0, 0,
					      "ERROR: Duplicate option '--dac1range'");
					usage();
				}
				/*
				 * Convert option argument string to unsigned long integer.
				 */
				errno = 0;
				dac1_range =
				    strtoul(optarg, &dac1range_string, 10);
				/*
				 * Catch unsigned long int overflow
				 */
				if ((dac1_range == ULONG_MAX)
				    && (errno = ERANGE)) {
					error(0, 0,
					      "ERROR: dac1range numeric overflow");
					usage();
				}
				/*
				 * Catch invalid argument strings
				 */
				if ((*dac1range_string != '\0')
				    || (dac1range_string == optarg)) {
					error(0, 0, "ERROR: invalid dac1range");
					usage();
				}
				/*
				 * '--dac1range' option has been acknowledged
				 */
				dac1r_option = 0xFF;
				break;
			}
			/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
			 * user entered '--dac1val'
			 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
		case 4:{
				char *dac1val_string;
				/*
				 * Deny multiple '--dac1val' options
				 */
				if (dac1v_option) {
					error(0, 0,
					      "ERROR: Duplicate option '--dac1val'");
					usage();
				}
				/*
				 * Convert option argument string to unsigned long integer.
				 */
				errno = 0;
				dac1_val = strtoul(optarg, &dac1val_string, 10);
				/*
				 * Catch unsigned long int overflow
				 */
				if ((dac1_val == ULONG_MAX) && (errno = ERANGE)) {
					error(0, 0,
					      "ERROR: dac1val numeric overflow");
					usage();
				}
				/*
				 * Catch invalid argument strings
				 */
				if ((*dac1val_string != '\0')
				    || (dac1val_string == optarg)) {
					error(0, 0, "ERROR: invalid dac1val");
					usage();
				}
				/*
				 * '--dac1val' option has been acknowledged
				 */
				dac1v_option = 0xFF;
				break;
			}
			/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
			 * user entered '--dac2range'
			 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
		case 5:{
				char *dac2range_string;
				/*
				 * Deny multiple '--dac2range' options
				 */
				if (dac2r_option) {
					error(0, 0,
					      "ERROR: Duplicate option '--dac2range'");
					usage();
				}
				/*
				 * Convert option argument string to unsigned long integer.
				 */
				errno = 0;
				dac2_range =
				    strtoul(optarg, &dac2range_string, 10);
				/*
				 * Catch unsigned long int overflow
				 */
				if ((dac2_range == ULONG_MAX)
				    && (errno = ERANGE)) {
					error(0, 0,
					      "ERROR: dac2range numeric overflow");
					usage();
				}
				/*
				 * Catch invalid argument strings
				 */
				if ((*dac2range_string != '\0')
				    || (dac2range_string == optarg)) {
					error(0, 0, "ERROR: invalid dac2range");
					usage();
				}
				/*
				 * '--dac2range' option has been acknowledged
				 */
				dac2r_option = 0xFF;
				break;
			}
			/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
			 * user entered '--dac2val'
			 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
		case 6:{
				char *dac2val_string;
				/*
				 * Deny multiple '--dac2val' options
				 */
				if (dac2v_option) {
					error(0, 0,
					      "ERROR: Duplicate option '--dac2val'");
					usage();
				}
				/*
				 * Convert option argument string tolong integer.
				 */
				errno = 0;
				dac2_val = strtol(optarg, &dac2val_string, 10);
				/*
				 * Catch unsigned long int overflow
				 */
				if ((dac2_val == ULONG_MAX) && (errno = ERANGE)) {
					error(0, 0,
					      "ERROR: dac1range numeric overflow");
					usage();
				}
				/*
				 * Catch invalid argument strings
				 */
				if ((*dac2val_string != '\0')
				    || (dac2val_string == optarg)) {
					error(0, 0, "ERROR: invalid dac2val");
					usage();
				}
				/*
				 * '--dac2val' option has been acknowledged
				 */
				dac2v_option = 0xFF;
				break;
			}
			/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
			 * User entered unsupported option
			 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
		case '?':
			error(0, 0, "ERROR: Unsupported option");
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
	/*
	 * '--dac1range' option must be given
	 */
	if (dac1r_option == 0x00) {
		error(0, 0, "Error: Option '--dac1range' is required");
		usage();
	} else if (dac1_range < 0 || dac1_range > 4) {
		error(0, 0, "Error: Invalid value for '--dac1range'");
		usage();
	}
	/*
	 * '--dac2range' option must be given
	 */
	if (dac2r_option == 0x00) {
		error(0, 0, "Error: Option '--dac2range' is required");
		usage();
	} else if (dac2_range < 0 || dac2_range > 4) {
		error(0, 0, "Error: Invalid value for '--dac2range'");
		usage();
	}
	/*
	 * '--dac1val' option must be given
	 */
	if (dac1v_option == 0x00) {
		error(0, 0, "Error: Option '--dac1val' is required");
		usage();
	} else if (dac1_val > 4096 || dac1_val < -2048) {
		error(0, 0, "Error: Invalid value for '--dac1val'");
		usage();
	}
	/*
	 * '--dac2val' option must be given
	 */
	if (dac2v_option == 0x00) {
		error(0, 0, "Error: Option '--dac2val' is required");
		usage();
	} else if (dac2_val > 4096 || dac2_val < -2048) {
		error(0, 0, "Error: Invalid value for '--dac2val'");
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
	   Main program code.
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
	/*
	 * Calibrate the board
	 */
	fprintf(stdout, "Calibrating the board ... \n");
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &before);
	dm75xx_status = DM75xx_Calibrate(board,
					 dac1_val,
					 dac2_val, dac1_range, dac2_range);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_Calibrate");

	/*
	 * Wait for the command to finish
	 */

	fprintf(stdout, "Waiting for Calibration Algorithm to complete\n");
	do {
		dm75xx_status = DM75xx_DSP_CMD_Complete(board, &data);
		DM75xx_Exit_On_Error(board, dm75xx_status,
				     "DM75xx_DSP_CMD_Complete");
	} while (data & 0xFF);

	/*
	 * Checking status of DSP command
	 */

	fprintf(stdout, "Reading DSP Command Status\n");
	dm75xx_status = DM75xx_DSP_CMD_Status(board, DM75xx_DSP_CAL_AUTO);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DSP_CMD_Status");

	/*
	 * If DM75xx_DSP_CMD_Status return a 0 then the command we requested was
	 * successful.
	 */

	fprintf(stdout, "Calibration complete!\n");

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &after);
	double time = (after.tv_nsec - before.tv_nsec) / 1000000000.;
	fprintf(stdout, "Finished in %2.6f seconds\n", time);

	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Program clean up
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
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
