/**
    @file

    @brief
        This example program demonstrates the use of the About Counter Interrupt

    @verbatim

    Samples are gathered via the Pacer Clock which is triggered by User Timer/
    Counter 2.  The About Counter is loaded to trigger an interrupt every 100
    samples.

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

		$Id: about_intrpt.c 99065 2016-04-26 18:03:23Z rgroner $
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
 * Variable used to count how many interrupts occurred
 */
static int interrupts;
/**
 * Rate for User Timer/Counter 2
 */
#define UTC2_RATE   400
/**
 * Sampling rate
 */
#define ADC_RATE    1000
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
	if (DM75xx_INTERRUPT_ACTIVE(status, DM75xx_INT_ABOUT)) {
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
	DM75xx_Board_Descriptor *board;
	DM75xx_Error dm75xx_status;
	dm75xx_cgt_entry_t cgt;
	int status, i;
	float actualRate;
	uint16_t data = 0x0000;

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
		"\n\tDM75xx About Counter Interrupt Example Program\n\n");
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
	/*
	 * Installing User-Space ISR
	 */
	fprintf(stdout, "Installing User-Space ISR ... \n");
	dm75xx_status = DM75xx_InstallISR(board, ISR);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_InstallISR");
	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Main program code.
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
	/*
	 * Clear ADC FIFO
	 */
	fprintf(stdout, "Clearing ADC FIFO... \n");
	dm75xx_status = DM75xx_ADC_Clear(board);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_Clear_AD_FIFO ");
	/*
	 * Check FIFO Status
	 */
	dm75xx_status = DM75xx_FIFO_Get_Status(board, &data);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_FIFO_Get_Status ");
	fprintf(stdout, "FIFO Status: 0x%4x \n", data);
	/*
	 * Enable Channel Gain Table
	 */
	fprintf(stdout, "Enabling Channel Gain Table... \n");
	dm75xx_status = DM75xx_CGT_Enable(board, 0xFF);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_CGT_Enable ");
	/*
	 * Create CGT Entry
	 */
	fprintf(stdout, "Writing Channel Gain Table Entries... \n");
	/*
	 * Create one entry for each channel.
	 */
	for (i = 0; i < 16; i++) {
		cgt.channel = i;
		cgt.gain = 0;
		cgt.nrse = 0;
		cgt.range = 0;
		cgt.ground = 0;
		cgt.pause = 0;
		cgt.dac1 = 0;
		cgt.dac2 = 0;
		cgt.skip = 0;
		/*
		 * Write the entry to the CGT
		 */
		dm75xx_status = DM75xx_CGT_Write(board, cgt);
		DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_CGT_Write ");
	}
	/*
	 * Setup pacer clock
	 */
	fprintf(stdout, "Setting up Pacer Clock... \n");
	dm75xx_status = DM75xx_PCLK_Setup(board,
					  DM75xx_PCLK_INTERNAL,
					  DM75xx_PCLK_FREQ_8_MHZ,
					  DM75xx_PCLK_REPEAT,
					  DM75xx_PCLK_START_UTC2,
					  DM75xx_PCLK_STOP_ACNT,
					  ADC_RATE, &actualRate);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_PCLK_Setup ");
	/*
	 * Set ADC Conversion Signal Select
	 */
	fprintf(stdout, "Setting A / D Conversion Sample Signal.. \n");
	dm75xx_status =
	    DM75xx_ADC_Conv_Signal(board, DM75xx_ADC_CONV_SIGNAL_PCLK);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_ADC_Conv_Signal ");
	/*
	 * Set User Output Signal 0 to A/D Conversion
	 */
	fprintf(stdout, "Setting User Output Signal 0... \n");
	dm75xx_status = DM75xx_UIO_Select(board, DM75xx_UIO0, DM75xx_UIO_ADC);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_UIO_Select ");
	/*
	 * Enabling About Counter
	 */
	fprintf(stdout, "Enabling About Counter... \n");
	dm75xx_status = DM75xx_ADC_About_Enable(board, 0xFF);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_ADC_About_Enable ");
	/*
	 * Load About Counter
	 */
	fprintf(stdout, "Loading About Counter... \n");
	dm75xx_status = DM75xx_ACNT_Set_Count(board, (100) - 1);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_ACNT_Set_Count ");
	/*
	 * Setup UTC2
	 */
	fprintf(stdout, "Setting up User Timer / Counter 2... \n");
	dm75xx_status = DM75xx_UTC_Setup(board,
					 DM75xx_UTC_2,
					 DM75xx_CUTC_8_MHZ,
					 DM75xx_GUTC_NOT_GATED,
					 DM75xx_UTC_MODE_SQUARE_WAVE,
					 (uint16_t) (8000000 / UTC2_RATE));
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_UTC_Setup ");
	/*
	 * Enable About Counter Interrupt
	 */
	fprintf(stdout, "Enabling About Counter Interrupt... \n");
	dm75xx_status = DM75xx_Interrupt_Enable(board, DM75xx_INT_ABOUT);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_Interrupt_Enable ");
	/*
	 *Start the pacer clock
	 */
	fprintf(stdout, "Starting Pacer Clock... \n");
	dm75xx_status = DM75xx_PCLK_Start(board);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_PCLK_Start ");
	fprintf(stdout, "Waiting for Interrupts... \n\n");
	int temp_ints;
	do {
		/*
		 * Cache a copy of the number of interrupts
		 */
		temp_ints = interrupts;
		fprintf(stdout, "Interrupts: %d \r", temp_ints);
		usleep(100);
	} while (temp_ints < 10);

	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Program clean up
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
	/*
	 * Remove User-Space ISR
	 */
	fprintf(stdout, "\n\nRemoving User-Space ISR... \n");
	dm75xx_status = DM75xx_RemoveISR(board);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_RemoveISR ");
	/*
	 * Reset the board.
	 */
	fprintf(stdout, "Board Reset... \n");
	dm75xx_status = DM75xx_Board_Reset(board);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_Board_Reset ");
	/*
	 * Close the dm75xx device.
	 */
	fprintf(stdout, "Closing the dm75xx... \n");
	dm75xx_status = DM75xx_Board_Close(board);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_Board_Close ");
	exit(EXIT_SUCCESS);
}
