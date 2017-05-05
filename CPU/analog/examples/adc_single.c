/**
    @file

    @brief
        Demonstrates the use of Analog to Digital Conversion.

    @verbatim

    This example program demonstrates simple Analog sampling on a single
    channel.  When sampling on a signal channel the channel gain table latch
    must be set as shown in this example program.  Samples are gathered until
    the FIFO is filled, then they are printed to the screen.

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

		$Id: adc_single.c 99107 2016-04-27 21:09:01Z rgroner $
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
 * Name of the program as invoked on the command line.
 */
static char *program_name;
/**
 * Sample rate
 */
#define ADC_RATE    10000

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
	fprintf(stderr, "Usage: %s [--help] --minor MINOR --channel CHANNEL\n",
		program_name);
	fprintf(stderr, "\n");
	fprintf(stderr,
		"    --help:     Display usage information and exit.\n");
	fprintf(stderr, "\n");
	fprintf(stderr,
		"    --minor:    Use specified DM75xx device number.\n");
	fprintf(stderr, "        MINOR:        Device minor number (>= 0).\n");
	fprintf(stderr, "    --channel:  Use specified Analog Channel.\n");
	fprintf(stderr, "        CHANNEL:      Channel number (0 - 15).\n");

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
	dm75xx_cgt_entry_t cgt;
	int status, i;
	float actualRate;
	uint16_t data = 0x0000;

	program_name = arguments[0];

	struct option options[] = {
		{"help", 0, 0, 1},
		{"minor", 1, 0, 2},
		{"channel", 1, 0, 3},
		{0, 0, 0, 0}
	};

	uint8_t help_option = 0x00;
	uint8_t minor_option = 0x00;
	uint8_t channel_option = 0x00;

	unsigned long int minor_number = 0;
	unsigned long int channel = 0;

	fprintf(stdout,
		"\n\tDM75xx Single Channel Analog to Digital Example Program\n\n");
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
			 * user entered '--channel'
			 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
		case 3:{
				char *channel_string;
				/*
				 * Deny multiple '--channel' options
				 */
				if (channel_option) {
					error(0, 0,
					      "ERROR: Duplicate option '--channel'");
					usage();
				}
				/*
				 * Convert option argument string to unsigned long integer.
				 */
				errno = 0;
				channel = strtoul(optarg, &channel_string, 10);
				/*
				 * Catch unsigned long int overflow
				 */
				if ((channel == ULONG_MAX) && (errno == ERANGE)) {
					error(0, 0,
					      "ERROR: Channel value caused numeric overflow");
					usage();
				}
				/*
				 * Catch invalid argument strings.
				 */
				if ((*channel_string != '\0')
				    || (channel_string == optarg)) {
					error(0, 0,
					      "ERROR: Invalid channel number");
					usage();
				}
				/*
				 * '--channel' option has been acknowledged
				 */
				channel_option = 0xFF;
				break;
			}
			/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
			 * User entered unsupported option
			 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
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
	/*
	 * '--channel' option must be given
	 */
	if (channel_option == 0x00) {
		error(0, 0, "ERROR: Option '--channel' is required");
		usage();
	} else {
		if (channel < 0 || channel > 15) {
			error(0, 0, "ERROR: Channel value is invalid");
			usage();
		}
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
	 * Clear ADC FIFO
	 */
	fprintf(stdout, "Clearing ADC FIFO ... \n");
	dm75xx_status = DM75xx_ADC_Clear(board);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_Clear_AD_FIFO");
	/*
	 * Check FIFO Status
	 */
	dm75xx_status = DM75xx_FIFO_Get_Status(board, &data);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_FIFO_Get_Status");
	fprintf(stdout, "FIFO Status: 0x%4x\n", data);
	/*
	 * Enable Channel Gain Table
	 */
	fprintf(stdout, "Enabling Channel Gain Latch ... \n");
	dm75xx_status = DM75xx_CGT_Enable(board, 0x00);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_CGT_Enable");
	/*
	 * Create CGT Entry
	 */
	cgt.channel = channel;
	cgt.gain = 0;
	cgt.nrse = 0;
	cgt.range = 0;
	cgt.ground = 0;
	cgt.pause = 0;
	cgt.dac1 = 0;
	cgt.dac2 = 0;
	cgt.skip = 0;
	/*
	 * Write the entry to the CGT Latch register (used for single channel)
	 */
	fprintf(stdout, "Writing Channel Gain Table entry ... \n");
	dm75xx_status = DM75xx_CGT_Latch(board, cgt);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_CGT_Latch");
	/*
	 * Setup pacer clock
	 */
	fprintf(stdout, "Setting up Pacer Clock ... \n");
	dm75xx_status = DM75xx_PCLK_Setup(board,
					  DM75xx_PCLK_INTERNAL,
					  DM75xx_PCLK_FREQ_8_MHZ,
					  DM75xx_PCLK_NO_REPEAT,
					  DM75xx_PCLK_START_SOFTWARE,
					  DM75xx_PCLK_STOP_SOFTWARE,
					  ADC_RATE, &actualRate);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_PCLK_Setup");

	/*
	 * Set ADC Conversion Signal Select
	 */
	dm75xx_status =
	    DM75xx_ADC_Conv_Signal(board, DM75xx_ADC_CONV_SIGNAL_PCLK);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_ADC_Conv_Signal");
	/*
	 * Set User Output Signal 0 to A/D Conversion
	 */
	fprintf(stdout, "Setting User Output Signal 0... \n");
	dm75xx_status = DM75xx_UIO_Select(board, DM75xx_UIO0, DM75xx_UIO_ADC);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_UIO_Select");
	/*
	 *Start the pacer clock
	 */
	fprintf(stdout, "Starting Pacer Clock ... \n");
	dm75xx_status = DM75xx_PCLK_Start(board);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_PCLK_Start");

	fprintf(stdout, "Filling FIFO ... \n");
	do {
		/*
		 * Check FIFO Status
		 */
		dm75xx_status = DM75xx_FIFO_Get_Status(board, &data);
		DM75xx_Exit_On_Error(board, dm75xx_status,
				     "DM75xx_FIFO_Get_Status");
	} while (data & DM75xx_FIFO_ADC_NOT_FULL);
	/*
	 * Stop the pacer clock
	 */
	fprintf(stdout, "Stopping Pacer Clock ... \n");
	dm75xx_status = DM75xx_PCLK_Stop(board);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_PCLK_Stop");
	/*
	 * Read out samples
	 */
	fprintf(stdout, "Reading Samples ... \n");
	i = 0;
	do {
		/*
		 * Read AD FIFO
		 */
		dm75xx_status = DM75xx_ADC_FIFO_Read(board, &data);
		DM75xx_Exit_On_Error(board, dm75xx_status,
				     "DM75xx_ADC_FIFO_Read");
		fprintf(stdout, "\t%2.2f\n",
			((DM75xx_ADC_ANALOG_DATA(data) / 4096.) * 10));
		/*
		 * Increment sample counter.
		 */
		i++;
		/*
		 * Read the FIFO Status
		 */
		dm75xx_status = DM75xx_FIFO_Get_Status(board, &data);
		DM75xx_Exit_On_Error(board, dm75xx_status,
				     "DM75xx_FIFO_Get_Status");
	}
	while (data & DM75xx_FIFO_ADC_NOT_EMPTY);
	/*
	 * Pring how many samples were received
	 */
	fprintf(stdout, "Received %d samples ...\n", i);
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
