/**
    @file

    @brief
        Demonstrates the use of High Speed Digital acquisition via DMA.

    @verbatim

    This example program uses UTC1 as a demand mode source for HSDIN DMA
    operations.  UTC1 will act as a sample counter and each time FIFO Half is
    counted it will trigger a demand mode DMA.

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

        $Id: hd_dma.c 99065 2016-04-26 18:03:23Z rgroner $
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
 * Amount of data we want from the board
 */
#define NUM_DATA 0x10000
/**
 * Size of the FIFO to emulate in the driver
 */
#define FIFO 0x4000
/**
 * Number of user ISR interrupts until we have the amount of data we want.
 */
#define NUM_INTS (NUM_DATA/(FIFO/2))
/**
 * Rate at which samples are collected
 */
#define RATE    50000		//50kHz
/**
 * Board descriptor
 */
DM75xx_Board_Descriptor *board;
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
	DM75xx_Exit_On_Error(board, -1, "Regular Operation");
}

/**
********************************************************************************
 @brief User-Space ISR
********************************************************************************
 **/

void ISR(uint32_t status)
{
	if (DM75xx_INTERRUPT_ACTIVE(status, DM75xx_INT_DMA_0)) {
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
	struct sigaction sig_action;
	DM75xx_Error dm75xx_status;
	int status, i;
	unsigned int fifo_size;
	uint16_t *buf;
	uint16_t *dma_data;
	uint8_t pci_master_status;

	program_name = arguments[0];

	interrupts = 0;

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

	fprintf(stdout,
		"\n\tDM75xx High Speed Digital with DMA Example Program\n\n");
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
	 * Check the boards PCI Master status.
	 */
	fprintf(stdout, "Checking PCI Master status ... \n");
	dm75xx_status = DM75xx_Board_PCI_Master(board, &pci_master_status);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_Board_PCI_Master");
	if (!pci_master_status) {
		error(EXIT_FAILURE, errno, "ERROR: Board is not PCI Master!");
	}
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
	 * Create user buffer for collecting data from the device
	 */
	fprintf(stdout, "Creating User Buffer ... \n");
	dm75xx_status = DM75xx_DMA_Buffer_Create(board,
						 &dma_data,
						 DM75xx_DMA_CHANNEL_0,
						 NUM_DATA);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DMA_Buffer_Create ");
	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Setup User Timer/Counters
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
	/*
	 * Set high speed digital sampling signal
	 */
	fprintf(stdout, "Setting High Speed Digital Sampling Signal... \n");
	dm75xx_status = DM75xx_HSDIN_Sample_Signal(board,
						   DM75xx_HSDIN_SIGNAL_UTC0);
	DM75xx_Exit_On_Error(board, dm75xx_status,
			     "DM75xx_HSDIN_Sample_Signal ");
	/*
	 * Setup User Timer/Counter 0
	 */
	fprintf(stdout, "User Timer / Counter 0 Setup... \n");
	dm75xx_status = DM75xx_UTC_Setup(board,
					 DM75xx_UTC_0,
					 DM75xx_CUTC_8_MHZ,
					 DM75xx_GUTC_GATED,
					 DM75xx_UTC_MODE_RATE_GENERATOR,
					 (uint16_t) (8000000 / RATE));
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_UTC_Setup ");
	/*
	 * Setup User Timer/Counter 1
	 */
	dm75xx_status = DM75xx_FIFO_Size(board, &fifo_size);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_FIFO_Size ");

	fprintf(stdout, "User Timer / Counter 1 Setup... \n");
	dm75xx_status = DM75xx_UTC_Setup(board,
					 DM75xx_UTC_1,
					 DM75xx_CUTC_HSDIN_SIGNAL,
					 DM75xx_GUTC_NOT_GATED,
					 DM75xx_UTC_MODE_RATE_GENERATOR,
					 (fifo_size / 2));
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_UTC_Setup ");
	/*
	 * Clear high speed digital FIFO
	 */
	fprintf(stdout, "Clearing high speed digital FIFO... \n");
	dm75xx_status = DM75xx_HSDIN_Clear(board);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_HSDIN_Clear ");
	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Initialize and Start DMA
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
	/*
	 * Initialize DMA on channel 0
	 */
	fprintf(stdout, "Initializing DMA Channel 0... \n");
	dm75xx_status = DM75xx_DMA_Initialize(board, DM75xx_DMA_CHANNEL_0,
					      DM75xx_DMA_FIFO_HSDIN,
					      DM75xx_DMA_DEMAND_UTC1,
					      FIFO, &buf);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DMA_Initialize ");
	/*
	 * Enable DMA Channel 0 Done Interrupt
	 */
	fprintf(stdout, "Enabling DMA Channel 0 Interrupt ... \n");
	dm75xx_status = DM75xx_Interrupt_Enable(board, DM75xx_INT_DMA_0);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_Interrupt_Enable");
	/*
	 * Enabling DMA
	 */
	fprintf(stdout, "Enabling DMA Channel 0... \n");
	dm75xx_status = DM75xx_DMA_Enable(board, DM75xx_DMA_CHANNEL_0, 0xFF);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DMA_Enable ");
	/*
	 * Start DMA
	 */
	fprintf(stdout, "Starting DMA Channel 0... \n");
	dm75xx_status = DM75xx_DMA_Start(board, DM75xx_DMA_CHANNEL_0);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DMA_Start ");
	/*
	 * Start UTC 0
	 */
	fprintf(stdout, "Starting UTC 0... \n");
	dm75xx_status =
	    DM75xx_UTC_Set_Gate(board, DM75xx_UTC_0, DM75xx_GUTC_NOT_GATED);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_UTC_Set_Gate ");

	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Receiving Data
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
	fprintf(stdout, "Obtaining Samples... \n");
	int temp_interrupts = interrupts;
	/*
	 * Loop here and wait for the User ISR to let us know we have received an
	 * interrupt.  If we have, copy the correct half out of the DMA buffer into
	 * our buffer so we can analyze it.
	 */
	do {
		if (temp_interrupts != interrupts) {
			/*
			 * Each time we see a new interrupts, make a copy of the new
			 * interrupt count as this number can changed in the middle of
			 * this loop iteration.
			 */
			temp_interrupts = interrupts;
			/*
			 * Copy the kernel dma buffer data to our buffer
			 */
			DM75xx_DMA_Buffer_Read(board,
					       DM75xx_DMA_CHANNEL_0,
					       temp_interrupts);
		}

		usleep(100);

	} while (temp_interrupts < NUM_INTS);
	/*
	 * Set User Timer/Counter 0 gate on to stop the clock
	 */
	fprintf(stdout, "Stopping User Timer / Counter 0... \n");
	dm75xx_status =
	    DM75xx_UTC_Set_Gate(board, DM75xx_UTC_0, DM75xx_GUTC_GATED);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_UTC_Set_Gate ");
	/*
	 * Set User Timer/Counter 1 gate on to stop the clock
	 */
	fprintf(stdout, "Stopping User Timer / Counter 1... \n");
	dm75xx_status =
	    DM75xx_UTC_Set_Gate(board, DM75xx_UTC_1, DM75xx_GUTC_GATED);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_UTC_Set_Gate ");
	/*
	 * Print out the samples we obtained.
	 */
	for (i = 0; i < (NUM_DATA); i++) {
		fprintf(stdout, "%d %x\n", i, (uint8_t) dma_data[i]);
		//if ((i + 1) % 16 == 0) {
		//  fprintf(stdout, "\n");
		//}
	}
	fprintf(stdout, "\n");
	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Program clean up
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
	/*
	 * Disable DMA Channel 0 Done Interrupt
	 */
	fprintf(stdout, "Disabling DMA Channel 0 Interrupt ... \n");
	dm75xx_status = DM75xx_Interrupt_Disable(board, DM75xx_INT_DMA_0);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_Interrupt_Disable");
	/*
	 * Disable DMA
	 */
	fprintf(stdout, "Disabling DMA... \n");
	dm75xx_status = DM75xx_DMA_Abort(board, DM75xx_DMA_CHANNEL_0);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DMA_Enable ");
	/*
	 * Free the user space DMA buffer
	 */
	fprintf(stdout, "Removing Collection Buffer... \n");
	dm75xx_status = DM75xx_DMA_Buffer_Free(board,
					       &dma_data, DM75xx_DMA_CHANNEL_0);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DMA_Buffer_Free ");
	/*
	 * Remove User-Space ISR
	 */
	fprintf(stdout, "Removing User - Space ISR... \n");
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
