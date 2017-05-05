/**
    @file

    @brief
        Demonstrates the use of Digital to Analog sampling via DMA.

    @verbatim

    This example program is similar to adc_dma except data is sampled at a
    slower rate and instead of logging to a buffer, the data is dumped to disk.
    This program will continually gather A/D samples until you ask it to quit.

    This program will run until the user presses Ctrl+C to quit.

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

        $Id: adc_dma_continuous.c 99065 2016-04-26 18:03:23Z rgroner $
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
static volatile int interrupts;
/**
 * Amount of data we want from the board
 */
#define NUM_DATA 0x80000
/**
 * Size of the FIFO to emulate in the driver
 */
#define FIFO 0x10000
/**
 * Number of user ISR interrupts until we have that much data
 */
#define NUM_INTS (NUM_DATA/(FIFO/2))
/**
 * Filname to dump the data
 */
#define DAT_FILE "./test.dat"
/**
 * Variable to allow graceful exit from Ctrl-C
 */
volatile uint8_t exit_program;
/**
 * Sampling rate
 */
#define ADC_RATE 50000

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
	fprintf(stderr, "    --channel:  Use specified Analog Channel.\n");
	fprintf(stderr, "        CHANNEL:      Channel number (0 - 15).\n");

	fprintf(stderr, "\n");
	exit(EXIT_FAILURE);
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
*******************************************************************************
@brief
    Exit gracefully if the user enters CTRL-C.
*******************************************************************************
*/
void sigint_handler(int sig_num, siginfo_t * info, void *ptr)
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
	struct sigaction sig_action;
	DM75xx_Error dm75xx_status;
	dm75xx_cgt_entry_t cgt;
	int status, i;
	float actualRate;
	uint8_t pci_master_status;
	uint16_t *buf, *dma_0_data;
	FILE *file_handle;

	program_name = arguments[0];

	interrupts = 0;

	exit_program = 0x00;

	struct option options[] = {
		{"help", 0, 0, 1},
		{"minor", 1, 0, 2},
		{"channel", 1, 0, 3},
		{0, 0, 0, 0}
	};
	/*
	 * Installation of signal handler for Ctrl-C
	 */
	memset(&sig_action, 0, sizeof(sig_action));

	sig_action.sa_sigaction = sigint_handler;
	//sigfillset(&sig_action.sa_mask);
	sig_action.sa_flags = 0;

	if (sigaction(SIGINT, &sig_action, NULL) < 0) {
		error(EXIT_FAILURE, errno, "sigaction() FAILED");
	}

	uint8_t help_option = 0x00;
	uint8_t minor_option = 0x00;
	uint8_t channel_option = 0x00;

	unsigned long int minor_number = 0;
	unsigned long int channel = 0;

	file_handle = fopen(DAT_FILE, "w");

	if (file_handle < 0) {
		DM75xx_Exit_On_Error(board, -1, "File Open Failure");
	}

	fprintf(stdout,
		"\n\tDM75xx Continuous A/D with DMA Example Program\n\n");
	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Process command line options
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
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
			 * user entered '--channel'
			 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
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
	/*
	 * '--channel' option must be given
	 */
	if (channel_option == 0x00) {
		error(0, 0, "ERROR: Option '--channel' is required");
		usage();
	} else {
		if (channel < 0 || channel > 15) {
			error(0, 0, "ERROR: Channel value is invalide");
			usage();
		}
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
	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Main program code.
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
	/*
	 * Clear ADC FIFO
	 */
	fprintf(stdout, "Clearing ADC FIFO ... \n");
	dm75xx_status = DM75xx_ADC_Clear(board);
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
	 * Installing User-Space ISR
	 */
	fprintf(stdout, "Installing User-Space ISR ... \n");
	dm75xx_status = DM75xx_InstallISR(board, ISR);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_InstallISR");
	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Initialize and Start DMA
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
	/*
	 * Create user buffer for collecting data from the device
	 */
	fprintf(stdout, "Creating User Buffer... \n");
	dm75xx_status = DM75xx_DMA_Buffer_Create(board,
						 &dma_0_data,
						 DM75xx_DMA_CHANNEL_0, FIFO);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DMA_Buffer_Create");
	/*
	 * Initialize DMA on channel 0
	 */
	fprintf(stdout, "Initializing DMA Channel 0... \n");
	dm75xx_status = DM75xx_DMA_Initialize(board, DM75xx_DMA_CHANNEL_0,
					      DM75xx_DMA_FIFO_ADC,
					      DM75xx_DMA_DEMAND_FIFO_ADC,
					      FIFO, &buf);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DMA_Initialize");
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
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DMA_Enable");
	/*
	 * Start DMA
	 */
	fprintf(stdout, "Starting DMA Channel 0... \n");
	dm75xx_status = DM75xx_DMA_Start(board, DM75xx_DMA_CHANNEL_0);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DMA_Start");
	/*
	 *Start the pacer clock
	 */
	fprintf(stdout, "Starting Pacer Clock... \n");
	dm75xx_status = DM75xx_PCLK_Start(board);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_PCLK_Start");
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
			uint16_t data = 0x0000;

			/*
			 * Each time we see a new interrupts, make a copy of the new
			 * interrupt count as this number can changed in the middle of
			 * this loop iteration.
			 */
			temp_interrupts = interrupts;
			/*
			 * Here we convert and write the data from the correct half of the
			 * kernel's emulated FIFO into the file.
			 */
			dm75xx_status = DM75xx_DMA_Buffer_Read(board,
							       DM75xx_DMA_CHANNEL_0,
							       temp_interrupts);

			if (temp_interrupts % 2 == 0) {
				for (i = 0; i < FIFO; i++) {

					fprintf(file_handle, "%2.2f\n",
						((DM75xx_ADC_ANALOG_DATA
						  (dma_0_data[i]) / 4096.) *
						 10));
				}
			}

			dm75xx_status = DM75xx_FIFO_Get_Status(board, &data);
			DM75xx_Exit_On_Error(board, dm75xx_status,
					     "DM75xx_FIFO_Get_Status");

			if (!(data & DM75xx_FIFO_ADC_NOT_FULL)) {
				fprintf(stdout, "FIFO full!  Exiting.\n");
				exit_program = 1;
			}
		}

		usleep(100);

	} while (!exit_program);
	/*
	 * Stop the pacer clock
	 */
	fprintf(stdout, "Stopping Pacer Clock... \n");
	dm75xx_status = DM75xx_PCLK_Stop(board);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_PCLK_Stop");

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
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DMA_Enable");
	/*
	 * Free the user space DMA buffer
	 */
	fprintf(stdout, "Removing Collection Buffer... \n");
	dm75xx_status = DM75xx_DMA_Buffer_Free(board,
					       &dma_0_data,
					       DM75xx_DMA_CHANNEL_0);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DMA_Buffer_Free");
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
