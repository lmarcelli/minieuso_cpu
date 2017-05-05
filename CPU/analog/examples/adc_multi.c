/**
    @file

    @brief
        Demonstrates the use of Analog to Digital Burst sampling.

    @verbatim

    This example program using the Pacer Clock and Burst Clock to perform
    Burst sampling.  Burst sampling is near simultaneous sampling of a given
    number of channels as configured per the channel gain table.  In this
    example we sample from all 16 channels on each Pacer Clock conversion
    signal.  Samples are acquired until the FIFO is filled then they are
    printed to the screen.

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

		$Id: adc_multi.c 99065 2016-04-26 18:03:23Z rgroner $
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
 * The number of channels to sample
 */
#define NUM_CHANNELS 16
/**
 * Burst Clock Rate
 */
#define BURST_RATE 1000000
/**
 * Pacer Clock Rate
 */
#define PACER_RATE 100000
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
	dm75xx_cgt_entry_t cgt[NUM_CHANNELS];
	int status, i;
	float actualRate;
	uint16_t data = 0x0000;

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
		"\n\tDM75xx A/D Multi-Channel Burst Example Program\n\n");
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
	fprintf(stdout, "Enabling Channel Gain Table ... \n");
	dm75xx_status = DM75xx_CGT_Enable(board, 0xFF);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_CGT_Enable");
	/*
	 * Create CGT Entry
	 */
	fprintf(stdout, "Writing Channel Gain Table ... \n");
	for (i = 0; i < NUM_CHANNELS; i++) {
		/*
		 * Creating CGT entry.=
		 */
		cgt[i].channel = i;
		cgt[i].gain = 0;
		cgt[i].nrse = 0;
		cgt[i].range = 0;
		cgt[i].ground = 0;
		cgt[i].pause = 0;
		cgt[i].dac1 = 0;
		cgt[i].dac2 = 0;
		cgt[i].skip = 0;
		/*
		 * Write the entry to the CGT
		 */
		dm75xx_status = DM75xx_CGT_Write(board, cgt[i]);
		DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_CGT_Write");
	}
	fprintf(stdout, "Setting up Burst Clock ... \n");
	dm75xx_status = DM75xx_BCLK_Setup(board,
					  DM75xx_BCLK_START_PACER,
					  DM75xx_BCLK_FREQ_8_MHZ,
					  BURST_RATE, &actualRate);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_PCLK_Setup");
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
					  PACER_RATE, &actualRate);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_PCLK_Setup");
	/*
	 * Set ADC Conversion Signal Select
	 */
	dm75xx_status =
	    DM75xx_ADC_Conv_Signal(board, DM75xx_ADC_CONV_SIGNAL_BCLK);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_ADC_Conv_Signal");
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
		fprintf(stdout, "%2.2f\n",
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
