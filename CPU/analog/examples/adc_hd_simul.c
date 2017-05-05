/**
    @file

    @brief
        Demonstrates Analog to Digital and High Speed Digital simultaneously.

    @brief
    This program simultaneously samples Analog and High Speed Digital data
    acquisition via DMA.

    About 500,000 samples are gathered on each source at various speeds and
    driver FIFO sizes.  This is done to show the versatility of the driver's DMA
    engine.

    The samples are printed to the screen at the end of the program.

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

        $Id: adc_hd_simul.c 99065 2016-04-26 18:03:23Z rgroner $
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
static volatile int dma_0_ints;
/**
 * Variable used to count how many interrupts occurred
 */
static volatile int dma_1_ints;
/**
 * Amount of data we want from the board
 */
#define ADC_NUM_DATA 0x80000
/**
 * Size of the FIFO to emulate in the driver
 */
#define ADC_FIFO 0x10000
/**
 * Amount of data we want from the board
 */
#define HD_NUM_DATA 0x80000
/**
 * Size of the FIFO to emulate in the driver
 */
#define HD_FIFO 0x8000
/**
 * Number of user ISR interrupts until we have the amount of data we want.
 */
#define ADC_NUM_INTS (ADC_NUM_DATA/(ADC_FIFO/2))
/**
 * Number of user ISR interrupts until we have the amount of data we want.
 */
#define HD_NUM_INTS (HD_NUM_DATA/(HD_FIFO/2))
/**
 * A/D Sampling rate
 */
#define ADC_RATE 50000
/**
 * HD Sampling rate
 */
#define HSDIN_RATE 30000
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
		fprintf(stdout, "Interrupt: DMA 0\n");
		dma_0_ints++;
	}

	if (DM75xx_INTERRUPT_ACTIVE(status, DM75xx_INT_DMA_1)) {
		fprintf(stdout, "Interrupt: DMA 1\n");
		dma_1_ints++;
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
	dm75xx_cgt_entry_t cgt;
	struct sigaction sig_action;
	int status, i;
	float actualRate;
	uint8_t pci_master_status;
	uint16_t *adc_buf, *hd_buf;
	uint16_t *dma_0_data, *dma_1_data;
	unsigned int fifo_size;
	uint8_t dma_0_enabled, dma_1_enabled;

	program_name = arguments[0];

	dma_0_ints = 0;
	dma_1_ints = 0;

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

	fprintf(stdout, "\n\tDM75xx simultaneous A/D and HD Acquisition\n\n");
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
	   Setup A/D
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
	/*
	 * Enable Channel Gain Table
	 */
	fprintf(stdout, "Enabling Channel Gain Latch... \n");
	dm75xx_status = DM75xx_CGT_Enable(board, 0x00);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_CGT_Enable ");
	/*
	 * Create CGT Entry
	 */
	cgt.channel = 0;
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
	fprintf(stdout, "Writing Channel Gain Table entry... \n");
	dm75xx_status = DM75xx_CGT_Latch(board, cgt);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_CGT_Latch ");
	/*
	 * Setup pacer clock
	 */
	fprintf(stdout, "Setting up Pacer Clock... \n");
	dm75xx_status = DM75xx_PCLK_Setup(board,
					  DM75xx_PCLK_INTERNAL,
					  DM75xx_PCLK_FREQ_8_MHZ,
					  DM75xx_PCLK_NO_REPEAT,
					  DM75xx_PCLK_START_SOFTWARE,
					  DM75xx_PCLK_STOP_SOFTWARE,
					  ADC_RATE, &actualRate);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_PCLK_Setup ");
	/*
	 * Set ADC Conversion Signal Select
	 */
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
	 * Clear ADC FIFO
	 */
	fprintf(stdout, "Clearing ADC FIFO... \n");
	dm75xx_status = DM75xx_ADC_Clear(board);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_Clear_AD_FIFO ");
	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Setup User Timer Counter for HD DMA
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
					 (uint16_t) (8000000 / HSDIN_RATE));
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
	 * Create user buffer for collecting data from the device
	 */
	fprintf(stdout, "Creating User Buffer... \n");
	dm75xx_status = DM75xx_DMA_Buffer_Create(board,
						 &dma_0_data,
						 DM75xx_DMA_CHANNEL_0,
						 ADC_NUM_DATA);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DMA_Buffer_Create ");
	/*
	 * Create user buffer for collecting data from the device
	 */
	fprintf(stdout, "Creating User Buffer... \n");
	dm75xx_status = DM75xx_DMA_Buffer_Create(board,
						 &dma_1_data,
						 DM75xx_DMA_CHANNEL_1,
						 HD_NUM_DATA);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DMA_Buffer_Create ");
	/*
	 * Initialize DMA on channel 0
	 */
	fprintf(stdout, "Initializing DMA Channel 0... \n");
	dm75xx_status = DM75xx_DMA_Initialize(board, DM75xx_DMA_CHANNEL_0,
					      DM75xx_DMA_FIFO_ADC,
					      DM75xx_DMA_DEMAND_FIFO_ADC,
					      ADC_FIFO, &adc_buf);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DMA_Initialize ");
	/*
	 * Initialize DMA on channel 1
	 */
	fprintf(stdout, "Initializing DMA Channel 1... \n");
	dm75xx_status = DM75xx_DMA_Initialize(board, DM75xx_DMA_CHANNEL_1,
					      DM75xx_DMA_FIFO_HSDIN,
					      DM75xx_DMA_DEMAND_UTC1,
					      HD_FIFO, &hd_buf);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DMA_Initialize ");
	/*
	 * Enable DMA Channel 0 Done Interrupt
	 */
	fprintf(stdout, "Enabling DMA Channel 0 Interrupt ... \n");
	dm75xx_status = DM75xx_Interrupt_Enable(board, DM75xx_INT_DMA_0);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_Interrupt_Enable");
	/*
	 * Enable DMA Channel 1 Done Interrupt
	 */
	fprintf(stdout, "Enabling DMA Channel 1 Interrupt ... \n");
	dm75xx_status = DM75xx_Interrupt_Enable(board, DM75xx_INT_DMA_1);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_Interrupt_Enable");
	/*
	 * Enabling DMA
	 */
	fprintf(stdout, "Enabling DMA Channel 0... \n");
	dm75xx_status = DM75xx_DMA_Enable(board, DM75xx_DMA_CHANNEL_0, 0xFF);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DMA_Enable ");
	dma_0_enabled = 0xFF;
	/*
	 * Enabling DMA
	 */
	fprintf(stdout, "Enabling DMA Channel 1... \n");
	dm75xx_status = DM75xx_DMA_Enable(board, DM75xx_DMA_CHANNEL_1, 0xFF);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DMA_Enable ");
	dma_1_enabled = 0xFF;
	/*
	 * Start DMA
	 */
	fprintf(stdout, "Starting DMA Channel 0... \n");
	dm75xx_status = DM75xx_DMA_Start(board, DM75xx_DMA_CHANNEL_0);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DMA_Start ");
	/*
	 * Start DMA
	 */
	fprintf(stdout, "Starting DMA Channel 1... \n");
	dm75xx_status = DM75xx_DMA_Start(board, DM75xx_DMA_CHANNEL_1);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DMA_Start ");
	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Receiving Data
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
	/*
	 *Start the pacer clock
	 */
	fprintf(stdout, "Starting Pacer Clock... \n");
	dm75xx_status = DM75xx_PCLK_Start(board);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_PCLK_Start ");
	/*
	 * Start UTC 0
	 */
	fprintf(stdout, "Starting UTC 0... \n");
	dm75xx_status =
	    DM75xx_UTC_Set_Gate(board, DM75xx_UTC_0, DM75xx_GUTC_NOT_GATED);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_UTC_Set_Gate ");
	fprintf(stdout, "Obtaining Samples... \n");
	int adc_temp_ints = dma_0_ints, hd_temp_ints = dma_1_ints;

	/*
	 * Loop here and wait for the User ISR to let us know we have received an
	 * interrupt.  If we have, copy the correct half out of the DMA buffer into
	 * our buffer so we can analyze it.
	 */
	do {
		if ((adc_temp_ints != dma_0_ints) && dma_0_enabled) {
			/*
			 * Each time we see a new interrupts, make a copy of the new
			 * interrupt count as this number can changed in the middle of
			 * this loop iteration.
			 */
			adc_temp_ints = dma_0_ints;
			/*
			 * Copy the kernel dma buffer data to our buffer
			 */
			DM75xx_DMA_Buffer_Read(board,
					       DM75xx_DMA_CHANNEL_0,
					       adc_temp_ints);

		}
		if ((hd_temp_ints != dma_1_ints) && dma_1_enabled) {

			hd_temp_ints = dma_1_ints;

			DM75xx_DMA_Buffer_Read(board,
					       DM75xx_DMA_CHANNEL_1,
					       hd_temp_ints);
		}
		/*
		 * Determine if either channel is finished with their transfers
		 */
		if ((adc_temp_ints >= ADC_NUM_INTS) && dma_0_enabled) {
			dma_0_enabled = 0x00;
			/*
			 * Disable DMA Channel 0 Done Interrupt
			 */
			fprintf(stdout,
				"Disabling DMA Channel 0 Interrupt ... \n");
			dm75xx_status =
			    DM75xx_Interrupt_Disable(board, DM75xx_INT_DMA_0);
			DM75xx_Exit_On_Error(board, dm75xx_status,
					     "DM75xx_Interrupt_Disable");
			/*
			 * Disable DMA channel 0
			 */
			fprintf(stdout, "Disabling DMA channel 0... \n");
			dm75xx_status =
			    DM75xx_DMA_Abort(board, DM75xx_DMA_CHANNEL_0);
			DM75xx_Exit_On_Error(board, dm75xx_status,
					     "DM75xx_DMA_Enable ");
			/*
			 * Stop the pacer clock
			 */
			fprintf(stdout, "Stopping Pacer Clock... \n");
			dm75xx_status = DM75xx_PCLK_Stop(board);
			DM75xx_Exit_On_Error(board, dm75xx_status,
					     "DM75xx_PCLK_Stop ");
			fprintf(stdout, "DMA Channel 0 Finished ! \n");
		}
		if ((hd_temp_ints >= HD_NUM_INTS) && dma_1_enabled) {
			dma_1_enabled = 0x00;
			/*
			 * Disable DMA Channel 1 Done Interrupt
			 */
			fprintf(stdout,
				"Disabling DMA Channel 1 Interrupt ... \n");
			dm75xx_status =
			    DM75xx_Interrupt_Disable(board, DM75xx_INT_DMA_1);
			DM75xx_Exit_On_Error(board, dm75xx_status,
					     "DM75xx_Interrupt_Disable");
			/*
			 * Disable DMA channel 1
			 */
			fprintf(stdout, "Disabling DMA channel 1... \n");
			dm75xx_status =
			    DM75xx_DMA_Abort(board, DM75xx_DMA_CHANNEL_1);
			DM75xx_Exit_On_Error(board, dm75xx_status,
					     "DM75xx_DMA_Enable ");
			/*
			 * Set User Timer/Counter 0 gate on to stop the clock
			 */
			fprintf(stdout,
				"Stopping User Timer / Counter 0... \n");
			dm75xx_status =
			    DM75xx_UTC_Set_Gate(board, DM75xx_UTC_0,
						DM75xx_GUTC_GATED);
			DM75xx_Exit_On_Error(board, dm75xx_status,
					     "DM75xx_UTC_Set_Gate ");
			fprintf(stdout, "DMA Channel 1 Finished ! \n");
		}

		usleep(100);

	} while ((adc_temp_ints < ADC_NUM_INTS)
		 || (hd_temp_ints < HD_NUM_INTS));
	/*
	 * Print out the samples we obtained.
	 */
	for (i = 0; i < (ADC_NUM_DATA / sizeof(uint16_t)); i++) {
		fprintf(stdout, "%2.2f | 0x%02x \n",
			((DM75xx_ADC_ANALOG_DATA(dma_0_data[i]) / 4096.) * 10),
			(uint8_t) dma_1_data[i]);

	}
	fprintf(stdout, "\n");
	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Program clean up
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
	/*
	 * Set User Timer/Counter 1 gate on to stop the clock
	 */
	fprintf(stdout, "Stopping User Timer / Counter 1... \n");
	dm75xx_status =
	    DM75xx_UTC_Set_Gate(board, DM75xx_UTC_1, DM75xx_GUTC_GATED);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_UTC_Set_Gate ");
	/*
	 * Free the user space DMA buffer
	 */
	fprintf(stdout, "Removing Collection Buffer... \n");
	dm75xx_status = DM75xx_DMA_Buffer_Free(board,
					       &dma_0_data,
					       DM75xx_DMA_CHANNEL_0);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DMA_Buffer_Free ");
	/*
	 * Free the user space DMA buffer
	 */
	fprintf(stdout, "Removing Collection Buffer... \n");
	dm75xx_status = DM75xx_DMA_Buffer_Free(board,
					       &dma_1_data,
					       DM75xx_DMA_CHANNEL_1);
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
