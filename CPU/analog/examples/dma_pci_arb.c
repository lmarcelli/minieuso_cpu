/**
    @file

    @brief
        Demonstrates the ability to DMA to an arbitrary PCI address.

    @verbatim

    This program will perform one 8k DMA transfer to video buffer.  This example
    program is used as a proof of concept that DMA is possible to an arbitrary
    PCI address.  This is useful if you want to DMA data directly to another
    device, such as a DSP.

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

		$Id: dma_pci_arb.c 99065 2016-04-26 18:03:23Z rgroner $
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
 * Size of the FIFO, in samples, to emulate in the driver
 */
#define FIFO 0x10000		//Simulate 4k fifo size in driver
/**
 * Amount of data, in samples, we want from the board
 */
#define NUM_DATA (FIFO * 2)	//total amount of data 8k
/**
 * Number of user ISR interrupts until we have the amount of data we want.
 */
#define NUM_INTS (NUM_DATA/(FIFO/2))
/**
 * Sampling rate
 */
#define ADC_RATE 100000		//1kHz
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
	fprintf(stderr,
		"    --pci:    Hex value of address to use in DMA transfer.\n");
	fprintf(stderr,
		"        ADDRESS:      Address in form of 0x########.\n");
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
		fprintf(stdout, "Interrupt Received %d of %d\n", NUM_INTS,
			interrupts);
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
	dm75xx_dma_channel_t channel = DM75xx_DMA_CHANNEL_0;
	struct sigaction sig_action;
	DM75xx_Error dm75xx_status;
	dm75xx_cgt_entry_t cgt;
	int status;
	float actualRate;
	uint8_t pci_master_status;
	uint16_t data = 0x0000;

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
		{"pci", 1, 0, 3},
		{0, 0, 0, 0}
	};

	uint8_t help_option = 0x00;
	uint8_t minor_option = 0x00;
	uint8_t address_option = 0x00;

	unsigned long int minor_number = 0;
	unsigned long int address = 0;

	fprintf(stdout,
		"\n\tDM75xx DMA to Arbitrary PCI Address Example Program\n\n");
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
			 * user entered '--pci'
			 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
		case 3:{
				int i;
				int len = strlen(optarg);
				/*
				 * Deny multiple '--pci' options
				 */
				if (address_option) {
					error(0, 0,
					      "ERROR: Duplicate option '--pci'");
					usage();
				}
				/*
				 * Convert option argument string to hex.
				 */
				errno = 0;
				for (i = (len - 1); i >= 0; i--) {
					uint8_t shift;
					shift = (((len - 1) - i) * 4);
					switch (optarg[i]) {
					case 48:
						address += 0 << shift;
						break;	// 0
					case 49:
						address += 1 << shift;
						break;	// 1
					case 50:
						address += 2 << shift;
						break;	// 2
					case 51:
						address += 3 << shift;
						break;	// 3
					case 52:
						address += 4 << shift;
						break;	// 4
					case 53:
						address += 5 << shift;
						break;	// 5
					case 54:
						address += 6 << shift;
						break;	// 6
					case 55:
						address += 7 << shift;
						break;	// 7
					case 56:
						address += 8 << shift;
						break;	// 8
					case 57:
						address += 9 << shift;
						break;	// 9
					case 65:
						address += 10 << shift;
						break;	// A
					case 66:
						address += 11 << shift;
						break;	// B
					case 67:
						address += 12 << shift;
						break;	// C
					case 68:
						address += 13 << shift;
						break;	// D
					case 69:
						address += 14 << shift;
						break;	// E
					case 70:
						address += 15 << shift;
						break;	// F
					case 97:
						address += 10 << shift;
						break;	// a
					case 98:
						address += 11 << shift;
						break;	// b
					case 99:
						address += 12 << shift;
						break;	// c
					case 100:
						address += 13 << shift;
						break;	//d
					case 101:
						address += 14 << shift;
						break;	//e
					case 102:
						address += 15 << shift;
						break;	//f
					default:
						fprintf(stdout,
							"Invalid pci address\n");
						exit(-1);
						break;
					}
				}
				/*
				 * Catch unsigned long int overflow
				 */
				if ((address == ULONG_MAX) && (errno == ERANGE)) {
					error(0, 0,
					      "ERROR: pci value caused numeric overflow");
					usage();
				}
				/*
				 * '--pci' option has been acknowledged
				 */
				address_option = 0xFF;
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
	if (address_option == 0x00) {
		error(0, 0, "ERROR: Option '--pci' is required");
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
	fprintf(stdout, "%f\n", actualRate);
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
	 * Initialize DMA on channel 0
	 */
	fprintf(stdout, "Initializing DMA Channel 0 ... \n");
	dm75xx_status = DM75xx_DMA_Init_Arb(board, DM75xx_DMA_CHANNEL_0,
					    DM75xx_DMA_FIFO_ADC,
					    DM75xx_DMA_DEMAND_FIFO_ADC,
					    FIFO, address);
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
	fprintf(stdout, "Enabling DMA Channel 0 ... \n");
	dm75xx_status = DM75xx_DMA_Enable(board, DM75xx_DMA_CHANNEL_0, 0xFF);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DMA_Enable");
	/*
	 * Start DMA
	 */
	fprintf(stdout, "Starting DMA Channel 0 ... \n");
	dm75xx_status = DM75xx_DMA_Start(board, DM75xx_DMA_CHANNEL_0);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DMA_Start");
	/*
	 *Start the pacer clock
	 */
	fprintf(stdout, "Starting Pacer Clock ... \n");
	dm75xx_status = DM75xx_PCLK_Start(board);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_PCLK_Start");
	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Receiving Data
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
	fprintf(stdout, "Obtaining Samples ... \n");
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
		}
		fprintf(stdout, "%d Completed Transfers\r", temp_interrupts);

		usleep(100);

	} while (temp_interrupts < NUM_INTS);
	fprintf(stdout, "\n");
	/*
	 * Stop the pacer clock
	 */
	fprintf(stdout, "Stopping Pacer Clock ... \n");
	dm75xx_status = DM75xx_PCLK_Stop(board);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_PCLK_Stop");
	/*
	 * Print out the samples we obtained.
	 */
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
	fprintf(stdout, "Disabling DMA ... \n");
	dm75xx_status = DM75xx_DMA_Abort(board, DM75xx_DMA_CHANNEL_0);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DMA_Enable");
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
