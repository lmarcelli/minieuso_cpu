/**
    @file

    @brief
        Demonstrates simultaneous Analog and Digital sampling.

    @verbatim

    Program samples out the DAC and in the ADC simultaneously via DMA.
    Different driver FIFO sizes are emulated to show the diversity of the
    DMA engine.  Also, the DAC and ADC are sampled at different rates to show
    that each interrupt source can be handled at various times.  The data
    captured during this example program is saved to a file named
    'test.txt'.

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

        $Id: adc_dac_simul.c 99065 2016-04-26 18:03:23Z rgroner $
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
 * Variable used to count how many interrupts occurred
 */
volatile static int adc_ints;
/**
 * Variable used to count how many interrupts occurred
 */
volatile static int dac_ints;
/**
 * Size of the DAC FIFO to emulate in the driver
 */
#define DAC_FIFO 0x10000
/**
 * Size of the ADC FIFO to emulate in the driver
 */
#define ADC_FIFO 0x8000
/**
 * DAC Sample rate
 */
#define DAC_RATE 40000
/**
 * ADC Sample rate
 */
#define ADC_RATE 25000
/**
 * Variable to allow graceful exit from Ctrl-C
 */
volatile uint8_t exit_program;
/**
 * Filname to dump the data
 */
#define DAT_FILE "./test.dat"
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
 @brief User-Space ISR
********************************************************************************
 **/

void ISR(uint32_t status)
{
	if (DM75xx_INTERRUPT_ACTIVE(status, DM75xx_INT_DMA_0)) {
		dac_ints++;
		fprintf(stdout, "DMA 0: Transfer Finished\n");
	}
	if (DM75xx_INTERRUPT_ACTIVE(status, DM75xx_INT_DMA_1)) {
		adc_ints++;
		fprintf(stdout, "DMA 0: Transfer Finished\n");
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
	struct sigaction sig_action;
	FILE *file_handle;
	int status, i;
	float temp;
	uint8_t pci_master_status;
	uint16_t *dac_buf, *adc_buf, *dma_0_data, *dma_1_data;

	program_name = arguments[0];

	dac_ints = 0;
	adc_ints = 0;

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

	file_handle = fopen(DAT_FILE, "w");

	if (file_handle < 0) {
		DM75xx_Exit_On_Error(board, -1, "File Open Failure");
	}

	fprintf(stdout,
		"\n\tDM75xx ADC and DAC Simultaneous Sampling Example Program\n\n");
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
	   Initialize and Start DMA
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
	/*
	 * Create user buffer for collecting data from the device
	 */
	fprintf(stdout, "Creating User Buffer... \n");
	dm75xx_status = DM75xx_DMA_Buffer_Create(board,
						 &dma_0_data,
						 DM75xx_DMA_CHANNEL_0,
						 DAC_FIFO);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DMA_Buffer_Create");
	/*
	 * Create user buffer for collecting data from the device
	 */
	fprintf(stdout, "Creating User Buffer... \n");
	dm75xx_status = DM75xx_DMA_Buffer_Create(board,
						 &dma_1_data,
						 DM75xx_DMA_CHANNEL_1,
						 ADC_FIFO);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DMA_Buffer_Create");
	/*
	 * Initialize DMA on channel 0
	 */
	fprintf(stdout, "Initializing DMA Channel 0... \n");
	dm75xx_status = DM75xx_DMA_Initialize(board, DM75xx_DMA_CHANNEL_0,
					      DM75xx_DMA_FIFO_DAC2,
					      DM75xx_DMA_DEMAND_FIFO_DAC2,
					      DAC_FIFO, &dac_buf);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DMA_Initialize");
	/*
	 * Initialize DMA on channel 1
	 */
	fprintf(stdout, "Initializing DMA Channel 1... \n");
	dm75xx_status = DM75xx_DMA_Initialize(board, DM75xx_DMA_CHANNEL_1,
					      DM75xx_DMA_FIFO_ADC,
					      DM75xx_DMA_DEMAND_FIFO_ADC,
					      ADC_FIFO, &adc_buf);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DMA_Initialize");
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
	 * Enabling DMA Channel 0
	 */
	fprintf(stdout, "Enabling DMA Channel 0... \n");
	dm75xx_status = DM75xx_DMA_Enable(board, DM75xx_DMA_CHANNEL_0, 0xFF);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DMA_Enable");
	/*
	 * Enabling DMA Channel 1
	 */
	fprintf(stdout, "Enabling DMA Channel 1... \n");
	dm75xx_status = DM75xx_DMA_Enable(board, DM75xx_DMA_CHANNEL_1, 0xFF);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DMA_Enable");
	/*
	 * Start DMA Channel 0
	 */
	fprintf(stdout, "Starting DMA Channel 0... \n");
	dm75xx_status = DM75xx_DMA_Start(board, DM75xx_DMA_CHANNEL_0);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DMA_Start");
	/*
	 * Start DMA Channel 1
	 */
	fprintf(stdout, "Starting DMA Channel 1... \n");
	dm75xx_status = DM75xx_DMA_Start(board, DM75xx_DMA_CHANNEL_1);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DMA_Start");
	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Setup DAC
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
	/*
	 * Create DAC1 waveform sinusoidal
	 */
	for (i = 0; i < DAC_FIFO; i++) {
		temp = 2047 * (sin((float)i * (2 * M_PI / (float)1024)));
		dma_0_data[i] = DM75xx_DAC_PACK_DATA(temp, 0x00, 0x00);
	}
	/*
	 * Setup DAC 1
	 */
	fprintf(stdout, "Setup DAC2... \n");
	dm75xx_status =
	    DM75xx_DAC_Setup(board, DM75xx_DAC2, DM75xx_DAC_RANGE_BIPOLAR_5,
			     DM75xx_DAC_UPDATE_CLOCK, DM75xx_DAC_MODE_CYCLE);

	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Setup ADC
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
/*
     * Enable Channel Gain Table
     */
	fprintf(stdout, "Enabling Channel Gain Latch... \n");
	dm75xx_status = DM75xx_CGT_Enable(board, 0x00);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_CGT_Enable");
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
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_CGT_Latch");
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
					  ADC_RATE, &temp);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_PCLK_Setup");
	/*
	 * Set ADC Conversion Signal Select
	 */
	dm75xx_status =
	    DM75xx_ADC_Conv_Signal(board, DM75xx_ADC_CONV_SIGNAL_PCLK);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_ADC_Conv_Signal");
	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Wait for Samples
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
	/*
	 * Set DAC Rate
	 */
	fprintf(stdout, "Setting DAC Rate... \n");
	dm75xx_status =
	    DM75xx_DAC_Set_Rate(board, DM75xx_DAC_FREQ_8_MHZ, DAC_RATE, &temp);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DAC_Set_Rate");
	fprintf(stdout, "DAC Rate set to % 6.2f \n", temp);
	/*
	 *Start the pacer clock
	 */
	fprintf(stdout, "Starting Pacer Clock... \n");
	dm75xx_status = DM75xx_PCLK_Start(board);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_PCLK_Start");
	/*
	 * Obtain Samples
	 */
	int temp_adc_ints = adc_ints;
	int temp_dac_ints = dac_ints;

	do {
		if ((temp_dac_ints != dac_ints) && !exit_program) {

			temp_dac_ints = dac_ints;
			/*
			 * Write DMA data to Kernel Buffer
			 */
			dm75xx_status = DM75xx_DMA_Buffer_Write(board,
								DM75xx_DMA_CHANNEL_0,
								temp_dac_ints);

			DM75xx_Exit_On_Error(board, dm75xx_status,
					     "DM75xx_DMA_Buffer_Write");
		}
		if ((temp_adc_ints != adc_ints) && !exit_program) {

			temp_adc_ints = adc_ints;
			/*
			 * Read DMA Data from Kernel Buffer
			 */
			dm75xx_status = DM75xx_DMA_Buffer_Read(board,
							       DM75xx_DMA_CHANNEL_1,
							       temp_adc_ints);
			if (temp_adc_ints % 2 == 0) {
				for (i = 0; i < ADC_FIFO; i++) {
					fprintf(file_handle, "% 2.2f\n",
						((DM75xx_ADC_ANALOG_DATA
						  (dma_1_data[i]) / 4096.) *
						 10));
				}
			}
		}

		usleep(100);
	} while (!exit_program);
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
	 * Disable DMA Channel 1 Done Interrupt
	 */
	fprintf(stdout, "Disabling DMA Channel 1 Interrupt ... \n");
	dm75xx_status = DM75xx_Interrupt_Disable(board, DM75xx_INT_DMA_1);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_Interrupt_Disable");
	/*
	 * Free the user space DMA buffer
	 */
	fprintf(stdout, "Removing Collection Buffer... \n");
	dm75xx_status = DM75xx_DMA_Buffer_Free(board,
					       &dma_0_data,
					       DM75xx_DMA_CHANNEL_0);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DMA_Buffer_Free");
	/*
	 * Free the user space DMA buffer
	 */
	fprintf(stdout, "Removing Collection Buffer... \n");
	dm75xx_status = DM75xx_DMA_Buffer_Free(board,
					       &dma_1_data,
					       DM75xx_DMA_CHANNEL_1);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DMA_Buffer_Free");
	/*
	 * Disable DMA
	 */
	fprintf(stdout, "Disabling DMA 0... \n");
	dm75xx_status = DM75xx_DMA_Abort(board, DM75xx_DMA_CHANNEL_0);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DMA_Enable");
	/*
	 * Disable DMA
	 */
	fprintf(stdout, "Disabling DMA 1... \n");
	dm75xx_status = DM75xx_DMA_Abort(board, DM75xx_DMA_CHANNEL_1);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DMA_Enable");
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
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_Board_Reset ");
	/*
	 * Close the dm75xx device.
	 */
	fprintf(stdout, "Closing the dm75xx... \n");
	dm75xx_status = DM75xx_Board_Close(board);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_Board_Close ");
	exit(EXIT_SUCCESS);
}
