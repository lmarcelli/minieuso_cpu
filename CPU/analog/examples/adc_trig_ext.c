/**
    @file

    @brief
        Demonstrates the use of Analog to Digital sampling via external trigger.

    @verbatim

    This example program uses the external trigger to toggle the Pacer Clock.
    While the External Trigger is high the Pacer Clock will run and while it is
    low the Pacer Clock will stop.  The status of the Pacer Clock will be
    printed to the screen as External Trigger Edge interrupts are received by
    the user-space ISR.

    Digital I/O Port 1 is used as an input to the external trigger.  The value
    on Port 1 is toggled with the strike of a key on the keyboard.  This
    effectively enables/disables acquisition.

    Note: This program uses DMA

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

		$Id: adc_trig_ext.c 99065 2016-04-26 18:03:23Z rgroner $
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
#include <sys/select.h>		//kbhit() implementation
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
 * Size of the FIFO to emulate in the driver
 */
#define FIFO 0x1000
/**
 * Amount of data we want from the board
 */
#define NUM_DATA (FIFO * 4)
/**
 * Number of user ISR interrupts until we have the amount of data we want.
 */
#define NUM_INTS (NUM_DATA/(FIFO/2))
/**
 * Sampling rate
 */
#define ADC_RATE 1000
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
	fprintf(stderr, "    --channel:  Use specified Analog Channel.\n");
	fprintf(stderr, "        CHANNEL:      Channel number (0 - 15).\n");

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
@brief
    User-Space ISR
********************************************************************************
 **/

void ISR(uint32_t status)
{

	if (DM75xx_INTERRUPT_ACTIVE(status, DM75xx_INT_DMA_0)) {
		interrupts++;
	}

	if (DM75xx_INTERRUPT_ACTIVE(status, DM75xx_INT_ETRIG_RISING)) {
		fprintf(stdout, "External Trigger High -- Pacer Clock On!\n");
		fflush(stdout);
	}

	if (DM75xx_INTERRUPT_ACTIVE(status, DM75xx_INT_ETRIG_FALLING)) {
		fprintf(stdout, "External Trigger Low -- Pacer Clock OFF!\n");
		fflush(stdout);
	}
}

/**
********************************************************************************
@brief
    Implementation of kbhit()
********************************************************************************
*/
int kbhit(void)
{
	struct timeval tv;
	fd_set read_fd;

	tv.tv_sec = 0;
	tv.tv_usec = 0;

	FD_ZERO(&read_fd);
	FD_SET(0, &read_fd);

	if (select(1, &read_fd, NULL, NULL, &tv) == -1) {
		return 0;
	}

	if (FD_ISSET(0, &read_fd)) {
		return 1;
	}
	return 0;
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
	struct sigaction sig_action;
	dm75xx_cgt_entry_t cgt;
	int status, i;
	float actualRate;
	uint16_t *buf;
	uint16_t *dma_data;
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
		{"channel", 1, 0, 3},
		{0, 0, 0, 0}
	};

	uint8_t help_option = 0x00;
	uint8_t minor_option = 0x00;
	uint8_t channel_option = 0x00;

	unsigned long int minor_number = 0;
	unsigned long int channel = 0;

	fprintf(stdout,
		"\n\tDM75xx Analog to Digital with Pacer "
		"clock control by External Trigger\n\n");
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
	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Main program code.
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
	/*
	 * Configure Port 1 Direction
	 */
	fprintf(stdout, "Configuring Port 1 Direction ... \n");
	dm75xx_status = DM75xx_DIO_Set_Direction(board, DM75xx_DIO_PORT1, 0xFF);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DIO_Set_Direction");
	/*
	 * Setting Port 1 Output
	 */
	fprintf(stdout, "Setting Port 1 Output ... \n");
	DM75xx_DIO_Set_Port(board, DM75xx_DIO_PORT1, 0x00);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DIO_Set_Port");
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
	fprintf(stdout, "Writing Channel Gain Latch ... \n");
	dm75xx_status = DM75xx_CGT_Latch(board, cgt);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_CGT_Latch");
	/*
	 * Setup pacer clock
	 */
	fprintf(stdout, "Setting up Pacer Clock ... \n");
	dm75xx_status = DM75xx_PCLK_Setup(board,
					  DM75xx_PCLK_INTERNAL,
					  DM75xx_PCLK_FREQ_8_MHZ,
					  DM75xx_PCLK_REPEAT,
					  DM75xx_PCLK_START_ETRIG_GATE,
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
	 * Installing User-Space ISR
	 */
	fprintf(stdout, "Installing User-Space ISR ... \n");
	dm75xx_status = DM75xx_InstallISR(board, ISR);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_InstallISR");
	/*
	 * Setting External Trigger Polarity
	 */
	fprintf(stdout, "Setting External Trigger Polarity ... \n");
	dm75xx_status =
	    DM75xx_ETRIG_Polarity_Select(board, DM75xx_EXT_POLARITY_POS);
	DM75xx_Exit_On_Error(board, dm75xx_status,
			     "DM75xx_ETRIG_Polarity_Select");
	/*
	 * Enabling external trigger interrupts
	 */
	fprintf(stdout,
		"Enabling External Trigger Rising / Falling Interrupts... \n");
	dm75xx_status =
	    DM75xx_Interrupt_Enable(board,
				    DM75xx_INT_ETRIG_RISING |
				    DM75xx_INT_ETRIG_FALLING);
	DM75xx_Exit_On_Error(board, dm75xx_status, " DM75xx_Interrupt_Enable ");
	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Initialize and Start DMA
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
	/*
	 * Create user buffer for collecting data from the device
	 */
	fprintf(stdout, "Creating User Buffer... \n");
	dm75xx_status = DM75xx_DMA_Buffer_Create(board,
						 &dma_data,
						 DM75xx_DMA_CHANNEL_0,
						 NUM_DATA);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_DMA_Buffer_Create ");
	/*
	 * Initialize DMA on channel 0
	 */
	fprintf(stdout, "Initializing DMA Channel 0... \n");
	dm75xx_status = DM75xx_DMA_Initialize(board, DM75xx_DMA_CHANNEL_0,
					      DM75xx_DMA_FIFO_ADC,
					      DM75xx_DMA_DEMAND_FIFO_ADC,
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
	 *Start the pacer clock
	 */
	fprintf(stdout, "Starting Pacer Clock... \n");
	dm75xx_status = DM75xx_PCLK_Start(board);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_PCLK_Start ");

	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Receiving Data
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
	fprintf(stdout, "Waiting for External Trigger... \n");
	int temp_interrupts = interrupts;
	uint8_t current_state = 0x00;
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
			 * Our destination pointer increases each iteration while our
			 * Source pointer goes back and forth between the middle of our DMA
			 * buffer and the beginning.  This effectively grabs the respective
			 * halves of the buffer and places them in order one after another
			 * in our larger user space buffer.
			 */
			DM75xx_DMA_Buffer_Read(board,
					       DM75xx_DMA_CHANNEL_0,
					       temp_interrupts);
		}
		if (kbhit()) {
			getchar();
			if (current_state) {
				fprintf(stdout, "Disabling Pacer Clock!\n");
				DM75xx_DIO_Set_Port(board, DM75xx_DIO_PORT1,
						    0x00);
				DM75xx_Exit_On_Error(board, dm75xx_status,
						     "DM75xx_DIO_Set_Port");
				current_state = 0x00;
			} else {
				fprintf(stdout, "Enabling Pacer Clock!\n");
				DM75xx_DIO_Set_Port(board, DM75xx_DIO_PORT1,
						    0xFF);
				DM75xx_Exit_On_Error(board, dm75xx_status,
						     "DM75xx_DIO_Set_Port");
				current_state = 0xFF;
			}
		}

		usleep(100);
	} while (temp_interrupts < NUM_INTS);
	/*
	 * Stop the pacer clock
	 */
	fprintf(stdout, "Stopping Pacer Clock... \n");
	dm75xx_status = DM75xx_PCLK_Stop(board);
	DM75xx_Exit_On_Error(board, dm75xx_status, "DM75xx_PCLK_Stop ");
	/*
	 * Print out the samples we obtained.
	 */
	for (i = 0; i < (NUM_DATA / sizeof(uint16_t)); i++) {
		fprintf(stdout, "% 2.2f \n",
			((DM75xx_ADC_ANALOG_DATA(dma_data[i]) / 4096.) * 10));
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
