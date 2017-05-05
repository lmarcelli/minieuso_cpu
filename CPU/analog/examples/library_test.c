/**
    @file

    @brief
        Program which tests the basic functionality of the library

    @verbatim

    This program is used to test the library API.  It passes various valid and
    invalid parameters to each library function to ensure that only acceptable
    values are considered valid.

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

    @warning
        This program ABSOLUTELY IS NOT INTENDED to be an example of how to
        program a board.  Some of the techniques appearing herein can lead to
        erratic program or system behavior and are used only to cause specific
        error conditions.

		$Id: library_test.c 65468 2012-12-04 19:42:58Z rgroner $
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
	Checks to make sure the correct error code was returned.
********************************************************************************
*/
static void expect_failure_and_check(int status, int expected)
{
	if (status != -1) {
		error(EXIT_FAILURE, 0,
		      "FAILED: Expected failure but success occurred.");
	}

	if (errno != expected) {
		error(EXIT_FAILURE, 0,
		      "FAILED: Expected errno %d, got %d", expected, errno);
	}
	fprintf(stdout, "Passed!\n");
}

/**
********************************************************************************
@brief
	Validates a successful return code.
********************************************************************************
*/
static void expect_success(int status)
{
	if (status != 0) {
		error(EXIT_FAILURE, 0,
		      "FAILED %d: Expected success but failure occurred.",
		      status);
	}
	fprintf(stdout, "Passed!\n");
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
	int status, i, j;
	uint8_t pci_master_status, data8;
	uint16_t data16, *buf;
	float actualRate;

	program_name = arguments[0];

	struct option options[] = {
		{"help", 0, 0, 1},
		{"minor", 1, 0, 2},
		{0, 0, 0, 0}
	};

	uint8_t help_option = 0x00;
	uint8_t minor_option = 0x00;

	unsigned long int minor_number = 0;

	fprintf(stdout, "\n\tDM75xx Library Example Program\n\n");
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
			/*
			 * User Entered '--help'
			 */
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
			/*
			 * user entered '--minor'
			 */
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
			/*
			 * User entered unsupported option
			 */
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
	if (dm75xx_status != 0) {
		error(EXIT_FAILURE, errno, "ERROR: DM75xx_Board_Open() FAILED");
	}
	/*
	 * Initialize the board.
	 */
	fprintf(stdout, "Board Initialization ...\n");
	dm75xx_status = DM75xx_Board_Init(board);
	if (dm75xx_status != 0) {
		error(EXIT_FAILURE, errno, "ERROR: DM75xx_Board_Init() FAILED");
	}
	/*
	 * Reset the board.
	 */
	fprintf(stdout, "Board Reset ... \n");
	dm75xx_status = DM75xx_Board_Reset(board);
	if (dm75xx_status != 0) {
		error(EXIT_FAILURE, errno,
		      "ERROR: DM75xx_BrdCtl_Board() FAILED");
	}
	/*
	 * Check the boards PCI Master status.
	 */
	dm75xx_status = DM75xx_Board_PCI_Master(board, &pci_master_status);
	if (dm75xx_status != 0) {
		error(EXIT_FAILURE, errno,
		      "ERROR: DM75xx_BrdCtl_PCI_Master() FAILED");
	}
	if (!pci_master_status) {
		error(EXIT_FAILURE, errno, "ERROR: Board is not PCI Master!");
	}
	/*##########################################################################
	   Check User Timer/Counter Library Functions
	   ###################################################################### */
	fprintf(stdout,
		"\n\n####################################################\n"
		"User Timer/Counter Functions\n"
		"######################################################\n\n");
	/*===============================
	 * DM75xx_UTC_Set_Clock_Source()
	 ================================*/
	/*
	 * Invalid parameters
	 */
	fprintf(stdout, "Verifying DM75xx_UTC_Set_Clock_Source() ... \n");
	fprintf(stdout, "\tInvalid UTC\t\t\t");
	dm75xx_status =
	    DM75xx_UTC_Set_Clock_Source(board, DM75xx_UTC_2 + 1, 0x00);
	expect_failure_and_check(dm75xx_status, EINVAL);
	fprintf(stdout, "\tInvalid Source\t\t\t");
	dm75xx_status = DM75xx_UTC_Set_Clock_Source(board, DM75xx_UTC_0, 0xFF0);
	expect_failure_and_check(dm75xx_status, EINVAL);
	/*
	 * Valid parameters
	 */
	for (i = DM75xx_UTC_0; i <= DM75xx_UTC_2; i++) {
		fprintf(stdout, "\tValidating UTC %d ...\n", i);
		for (j = DM75xx_CUTC_8_MHZ; j <= DM75xx_CUTC_HSDIN_SIGNAL; j++) {

			if (j == DM75xx_CUTC_HSDIN_SIGNAL && i == DM75xx_UTC_1) {
				fprintf(stdout, "\t\tSource %d\t\t", j);
				dm75xx_status =
				    DM75xx_UTC_Set_Clock_Source(board, i, j);
				expect_success(dm75xx_status);
			} else if (j > DM75xx_CUTC_EXT_PCLK
				   && i == DM75xx_UTC_0) {
				continue;
			} else if (j != DM75xx_CUTC_HSDIN_SIGNAL) {
				fprintf(stdout, "\t\tSource %d\t\t", j);
				dm75xx_status =
				    DM75xx_UTC_Set_Clock_Source(board, i, j);
				expect_success(dm75xx_status);
			}
		}
	}
	/*========================
	 * DM75xxx_UTC_Set_Gate()
	 =========================*/
	/*
	 * Invalid Parameters
	 */
	fprintf(stdout, "Verifying DM75xx_UTC_Set_Gate() ... \n");
	fprintf(stdout, "\tInvalid UTC\t\t\t");
	dm75xx_status = DM75xx_UTC_Set_Gate(board, DM75xx_UTC_2 + 1, 0x00);
	expect_failure_and_check(dm75xx_status, EINVAL);
	fprintf(stdout, "\tInvalid Gate\t\t\t");
	dm75xx_status = DM75xx_UTC_Set_Gate(board, DM75xx_UTC_0, 0xFF);
	expect_failure_and_check(dm75xx_status, EINVAL);
	/*
	 * Valid parameters
	 */
	for (i = DM75xx_UTC_0; i <= DM75xx_UTC_2; i++) {
		fprintf(stdout, "\tValidating UTC %d ...\n", i);
		for (j = DM75xx_GUTC_NOT_GATED; j <= DM75xx_GUTC_UTC_0_OUT; j++) {
			if (i == DM75xx_UTC_0 && j > DM75xx_GUTC_EXT_TC_CLK_2) {
				continue;
			} else {
				fprintf(stdout, "\t\tGate %d\t\t\t", j);
				dm75xx_status =
				    DM75xx_UTC_Set_Gate(board, i, j);
				expect_success(dm75xx_status);
			}
		}
	}
	/*=======================
	 * DM75xx_UTC_Set_Mode()
	 ========================*/
	/*
	 * Invalid parameters
	 */
	fprintf(stdout, "Verifying DM75xx_UTC_Set_Mode() ... \n");
	fprintf(stdout, "\tInvalid UTC\t\t\t");
	dm75xx_status = DM75xx_UTC_Set_Mode(board, DM75xx_UTC_2 + 1, 0x00);
	expect_failure_and_check(dm75xx_status, EINVAL);
	fprintf(stdout, "\tInvalid Mode\t\t\t");
	dm75xx_status = DM75xx_UTC_Set_Mode(board, DM75xx_UTC_0, 0xFF);
	expect_failure_and_check(dm75xx_status, EINVAL);
	/*
	 * Valid parameters
	 */
	for (i = DM75xx_UTC_0; i <= DM75xx_UTC_2; i++) {
		fprintf(stdout, "\tValidating UTC %d ...\n", i);
		for (j = DM75xx_UTC_MODE_EVENT_COUNTER;
		     j <= DM75xx_UTC_MODE_HARDWARE_STROBE; j++) {
			fprintf(stdout, "\t\tMode %d\t\t\t", j);
			dm75xx_status = DM75xx_UTC_Set_Mode(board, i, j);
			expect_success(dm75xx_status);
		}
	}
	/*======================
	 * DM75xx_UTC_Get_Mode()
	 =======================*/
	/*
	 * Invalid parameters
	 */
	fprintf(stdout, "Verifying DM75xx_UTC_Get_Mode() ... \n");
	fprintf(stdout, "\tInvalid UTC\t\t\t");
	dm75xx_status = DM75xx_UTC_Get_Mode(board, DM75xx_UTC_2 + 1, 0x00);
	expect_failure_and_check(dm75xx_status, EINVAL);
	/*
	 * Valid parameters
	 */
	for (i = DM75xx_UTC_0; i <= DM75xx_UTC_2; i++) {
		fprintf(stdout, "\t\tUser Timer/Counter %d\t", i);
		dm75xx_status = DM75xx_UTC_Get_Mode(board, i, &data16);
		expect_success(dm75xx_status);
	}
	/*==========================
	 * DM75xx_UTC_Set_Divisor()
	 ===========================*/
	/*
	 * Invalid parameters
	 */
	fprintf(stdout, "Verifying DM75xx_UTC_Set_Divisor() ... \n");
	fprintf(stdout, "\tInvalid UTC\t\t\t");
	dm75xx_status = DM75xx_UTC_Set_Divisor(board, DM75xx_UTC_2 + 1, 0xFFFF);
	expect_failure_and_check(dm75xx_status, EINVAL);
	/*
	 * Valid parameters
	 */
	for (i = DM75xx_UTC_0; i <= DM75xx_UTC_2; i++) {
		fprintf(stdout, "\tValidating UTC %d ... \n", i);
		for (j = 0x0000; j <= 0xFFFF; j++) {
			fprintf(stdout, "\t\tDivisor 0x%4x\t\t", j);
			dm75xx_status = DM75xx_UTC_Set_Divisor(board, i, j);
			if (dm75xx_status != 0) {
				error(EXIT_FAILURE, 0,
				      "FAILED %d: Expected success but failure occurred.",
				      status);
			}
			fprintf(stdout, "Passed!\r");
		}
		fprintf(stdout, "\n");
	}
	/*========================
	 * DM75xx_UTC_Get_Count()
	 =========================*/
	fprintf(stdout, "Verifying DM75xx_UTC_Get_Count() ... \n");
	fprintf(stdout, "\tInvalid UTC\t\t\t");
	dm75xx_status = DM75xx_UTC_Get_Count(board, DM75xx_UTC_2 + 1, 0x00);
	expect_failure_and_check(dm75xx_status, EINVAL);
	/*
	 * Valid parameters
	 */
	for (i = DM75xx_UTC_0; i <= DM75xx_UTC_2; i++) {
		fprintf(stdout, "\t\tUser Timer/Counter %d\t", i);
		dm75xx_status = DM75xx_UTC_Get_Count(board, i, &data16);
		expect_success(dm75xx_status);
	}
	/*====================
	 * DM75xx_UTC_Setup()
	 =====================*/
	fprintf(stdout, "Verifying DM75xx_UTC_Setup() ... \n");
	fprintf(stdout, "\tInvalid UTC\t\t\t");
	dm75xx_status = DM75xx_UTC_Setup(board,
					 DM75xx_UTC_2 + 1,
					 DM75xx_CUTC_8_MHZ,
					 DM75xx_GUTC_NOT_GATED,
					 DM75xx_UTC_MODE_SQUARE_WAVE, 0xFFFF);
	expect_failure_and_check(dm75xx_status, EINVAL);
	fprintf(stdout, "\tInvalid Source \t\t\t");
	dm75xx_status = DM75xx_UTC_Setup(board,
					 DM75xx_UTC_0,
					 0xFF,
					 DM75xx_GUTC_NOT_GATED,
					 DM75xx_UTC_MODE_SQUARE_WAVE, 0xFFFF);
	expect_failure_and_check(dm75xx_status, EINVAL);
	fprintf(stdout, "\tInvalid Gate\t\t\t");
	dm75xx_status = DM75xx_UTC_Setup(board,
					 DM75xx_UTC_0,
					 DM75xx_CUTC_8_MHZ,
					 0xFF,
					 DM75xx_UTC_MODE_SQUARE_WAVE, 0xFFFF);
	expect_failure_and_check(dm75xx_status, EINVAL);
	fprintf(stdout, "\tInvavlid Mode\t\t\t");
	dm75xx_status = DM75xx_UTC_Setup(board,
					 DM75xx_UTC_0,
					 DM75xx_CUTC_8_MHZ,
					 DM75xx_GUTC_NOT_GATED, 0xFF, 0xFFFF);
	expect_failure_and_check(dm75xx_status, EINVAL);
	/*========================
	 * DM75xx_UTC_Get_Status()
	 =========================*/
	fprintf(stdout, "Verifying DM75xx_UTC_Get_Status() ... \n");
	fprintf(stdout, "\tInvalid UTC\t\t\t");
	dm75xx_status = DM75xx_UTC_Get_Status(board, DM75xx_UTC_2 + 1, 0x00);
	expect_failure_and_check(dm75xx_status, EINVAL);
	/*##########################################################################
	   Check Burst Clock Library Functions
	   ###################################################################### */
	fprintf(stdout,
		"\n\n####################################################\n"
		"Burst Clock Functions\n"
		"######################################################\n\n");
	/*===========================
	 * DM75xx_BCLK_Set_Rate
	 ============================*/
	/*
	 * Invalid Parameters
	 */
	fprintf(stdout, "Verifying DM75xx_BCLK_Set_Rate() ... \n");
	fprintf(stdout, "\tInvalid Frequency\t\t");
	dm75xx_status =
	    DM75xx_BCLK_Set_Rate(board, DM75xx_BCLK_FREQ_20_MHZ + 1, 0xFFFF,
				 &actualRate);
	expect_failure_and_check(dm75xx_status, EINVAL);

	fprintf(stdout, "\tInvalid Rate\t\t\t");
	dm75xx_status = DM75xx_BCLK_Set_Rate(board, 0x00, 0x0000, &actualRate);
	expect_failure_and_check(dm75xx_status, EINVAL);
	fprintf(stdout, "\tInvalid Rate\t\t\t");
	dm75xx_status = DM75xx_BCLK_Set_Rate(board, 0x00, 122, &actualRate);
	expect_failure_and_check(dm75xx_status, EINVAL);
	/*
	 * Valid parameters
	 */
	fprintf(stdout, "\tValidating Burst Clock \n");
	for (i = 123; i <= 0xFFFF; i++) {
		fprintf(stdout, "\t\tRate 0x%4x\t\t", i);
		dm75xx_status =
		    DM75xx_BCLK_Set_Rate(board, 0x00, i, &actualRate);
		if (dm75xx_status != 0) {
			error(EXIT_FAILURE, 0,
			      "FAILED %d: Expected success but failure occurred.",
			      status);
		}
		fprintf(stdout, "Passed!\r");
	}
	fprintf(stdout, "\n");
	/*=======================
	 * DM75xx_BCLK_Set_Start
	  =======================*/
	/*
	 * Invalid parameters
	 */
	fprintf(stdout, "Verifying DM75xx_BCLK_Set_Start() ... \n");
	fprintf(stdout, "\tInvalid Start\t\t\t");
	dm75xx_status =
	    DM75xx_BCLK_Set_Start(board, DM75xx_BCLK_START_SBUS2 + 1);
	expect_failure_and_check(dm75xx_status, EINVAL);

	/*
	 * Valid parameters
	 */
	fprintf(stdout, "\tValidating Burst Clock \n");
	for (i = DM75xx_BCLK_START_SOFTWARE; i <= DM75xx_BCLK_START_SBUS2; i++) {
		fprintf(stdout, "\t\tStart %d\t\t\t", i);
		dm75xx_status = DM75xx_BCLK_Set_Start(board, i);
		expect_success(dm75xx_status);
	}
	/*##########################################################################
	   Check Pacer Clock Library Functions
	   ###################################################################### */
	fprintf(stdout,
		"\n\n####################################################\n"
		"Pacer Clock Functions\n"
		"######################################################\n\n");
	/*===========================
	 * DM75xx_PCLK_Set_Frequency
	 ============================*/
	/*
	 * Invalid parameters
	 */
	fprintf(stdout, "Verifiying DM75xx_PCLK_Set_Frequency() ... \n");
	fprintf(stdout, "\tInvalid Frequency\t\t");
	dm75xx_status =
	    DM75xx_PCLK_Set_Frequency(board, DM75xx_PCLK_FREQ_20_MHZ + 1);
	expect_failure_and_check(dm75xx_status, EINVAL);
	/*
	 * Valid parameters
	 */
	fprintf(stdout, "\tValidating Pacer Clock ... \n");
	for (i = DM75xx_PCLK_FREQ_8_MHZ; i <= DM75xx_PCLK_FREQ_20_MHZ; i++) {
		fprintf(stdout, "\t\tFrequency %d\t\t", i);
		dm75xx_status = DM75xx_PCLK_Set_Frequency(board, i);
		expect_success(dm75xx_status);
	}
	/*========================
	 * DM75xx_PCKL_Set_Source
	 =========================*/
	/*
	 * Invalid parameters
	 */
	fprintf(stdout, "Verifying DM75xx_PCLK_Set_Source() ... \n");
	fprintf(stdout, "\tInvalid Source\t\t\t");
	dm75xx_status = DM75xx_PCLK_Set_Source(board, DM75xx_PCLK_INTERNAL + 1);
	expect_failure_and_check(dm75xx_status, EINVAL);
	/*
	 * Valid parameters
	 */
	fprintf(stdout, "\tValidating Pacer Clock ... \n");
	for (i = DM75xx_PCLK_EXTERNAL; i <= DM75xx_PCLK_INTERNAL; i++) {
		fprintf(stdout, "\t\tSource %d\t\t", i);
		dm75xx_status = DM75xx_PCLK_Set_Source(board, i);
		expect_success(dm75xx_status);
	}
	/*=======================
	 * DM75xx_PCLK_Set_Start
	 ========================*/
	/*
	 * Invalid parameters
	 */
	fprintf(stdout, "Verifying DM75xx_PCLK_Set_Start() ... \n");
	fprintf(stdout, "\tInvalid Start Trigger\t\t");
	dm75xx_status =
	    DM75xx_PCLK_Set_Start(board, DM75xx_PCLK_START_ETRIG_GATE + 1);
	expect_failure_and_check(dm75xx_status, EINVAL);
	/*
	 * Valid parameters
	 */
	fprintf(stdout, "\tValidating Pacer Clock ... \n");
	for (i = DM75xx_PCLK_START_SOFTWARE;
	     i <= DM75xx_PCLK_START_ETRIG_GATE; i++) {
		if (i > 10) {
			fprintf(stdout, "\t\tStart Trigger %d\t", i);
		} else if (i < 10) {
			fprintf(stdout, "\t\tStart Trigger %d\t\t", i);
		}
		dm75xx_status = DM75xx_PCLK_Set_Start(board, i);
		expect_success(dm75xx_status);
	}
	/*======================
	 * DM75xx_PCLK_Set_Stop
	 =======================*/
	/*
	 * Invalid parameters
	 */
	fprintf(stdout, "Verifying DM75xx_PCLK_Set_Stop() ... \n");
	fprintf(stdout, "\tInvalid Stop Trigger\t\t");
	dm75xx_status =
	    DM75xx_PCLK_Set_Stop(board, DM75xx_PCLK_STOP_ASBUS2 + 1);
	expect_failure_and_check(dm75xx_status, EINVAL);
	/*
	 * Valid parameters
	 */
	fprintf(stdout, "\tValidating Pacer Clock ... \n");
	for (i = DM75xx_PCLK_STOP_SOFTWARE; i <= DM75xx_PCLK_STOP_ASBUS2; i++) {
		fprintf(stdout, "\t\tStop Trigger %d\t\t", i);
		dm75xx_status = DM75xx_PCLK_Set_Stop(board, i);
		expect_success(dm75xx_status);
	}
	/*======================
	 * DM75xx_PCLK_Set_Mode
	 =======================*/
	/*
	 * Invalid parameters
	 */
	fprintf(stdout, "Verifying DM75xx_PCLK_Set_Mode() ... \n");
	fprintf(stdout, "\tInvalid Repeat Mode\t\t");
	dm75xx_status =
	    DM75xx_PCLK_Set_Trigger_Mode(board, DM75xx_PCLK_REPEAT + 1);
	expect_failure_and_check(dm75xx_status, EINVAL);
	/*
	 * Valid parameters
	 */
	fprintf(stdout, "\tValidating Pacer Clock ... \n");
	for (i = DM75xx_PCLK_NO_REPEAT; i <= DM75xx_PCLK_REPEAT; i++) {
		fprintf(stdout, "\t\tTrigger Mode %d\t\t", i);
		dm75xx_status = DM75xx_PCLK_Set_Trigger_Mode(board, i);
		expect_success(dm75xx_status);
	}
	/*=====================
	 * DM75xx_PCLK_Set_Rate
	 ======================*/
	/*
	 * Invalid Parameters
	 */
	fprintf(stdout, "Verifying DM75xx_PCLK_Set_Rate() ... \n");
	fprintf(stdout, "\tInvalid Frequency\t\t");
	dm75xx_status =
	    DM75xx_PCLK_Set_Rate(board, DM75xx_PCLK_FREQ_20_MHZ + 1, 0xFF,
				 &actualRate);
	expect_failure_and_check(dm75xx_status, EINVAL);
	/*
	 * Valid parameters
	 */
	fprintf(stdout, "\tValidaing Pacer Clock\t\t");
	for (i = 0x0001; i <= 0x1312d0; i++) {
		dm75xx_status =
		    DM75xx_PCLK_Set_Rate(board, 0x00, i, &actualRate);
		if (dm75xx_status != 0) {
			error(EXIT_FAILURE, 0,
			      "FAILED %d: Expected success but failure occurred.",
			      status);
		}
	}
	fprintf(stdout, "Passed!\n");
	/*===================
	 * DM75xx_PCLK_Setup
	 ====================*/
	fprintf(stdout, "Verifying DM75xx_PCLK_Setup() ... \n");
	fprintf(stdout, "\tInvalid Source\t\t\t");
	dm75xx_status = DM75xx_PCLK_Setup(board,
					  DM75xx_PCLK_INTERNAL + 1,
					  DM75xx_PCLK_FREQ_8_MHZ,
					  DM75xx_PCLK_NO_REPEAT,
					  DM75xx_PCLK_START_SOFTWARE,
					  DM75xx_PCLK_STOP_SOFTWARE,
					  0xFFFF, &actualRate);
	expect_failure_and_check(dm75xx_status, EINVAL);
	fprintf(stdout, "\tInvalid Frequency\t\t");
	dm75xx_status = DM75xx_PCLK_Setup(board,
					  DM75xx_PCLK_INTERNAL + 1,
					  DM75xx_PCLK_FREQ_20_MHZ + 1,
					  DM75xx_PCLK_NO_REPEAT,
					  DM75xx_PCLK_START_SOFTWARE,
					  DM75xx_PCLK_STOP_SOFTWARE,
					  0xFFFF, &actualRate);
	expect_failure_and_check(dm75xx_status, EINVAL);
	fprintf(stdout, "\tInvalid Mode\t\t\t");
	dm75xx_status = DM75xx_PCLK_Setup(board,
					  DM75xx_PCLK_INTERNAL,
					  DM75xx_PCLK_FREQ_8_MHZ,
					  DM75xx_PCLK_REPEAT + 1,
					  DM75xx_PCLK_START_SOFTWARE,
					  DM75xx_PCLK_STOP_SOFTWARE,
					  0xFFFF, &actualRate);
	expect_failure_and_check(dm75xx_status, EINVAL);
	fprintf(stdout, "\tInvalid Start Trigger\t\t");
	dm75xx_status = DM75xx_PCLK_Setup(board,
					  DM75xx_PCLK_INTERNAL,
					  DM75xx_PCLK_FREQ_8_MHZ,
					  DM75xx_PCLK_NO_REPEAT,
					  DM75xx_PCLK_START_ETRIG_GATE + 1,
					  DM75xx_PCLK_STOP_ASBUS2,
					  0xFFFF, &actualRate);
	expect_failure_and_check(dm75xx_status, EINVAL);
	fprintf(stdout, "\tInvalid Stop Trigger\t\t");
	dm75xx_status = DM75xx_PCLK_Setup(board,
					  DM75xx_PCLK_INTERNAL,
					  DM75xx_PCLK_FREQ_8_MHZ,
					  DM75xx_PCLK_NO_REPEAT,
					  DM75xx_PCLK_START_ETRIG_GATE,
					  DM75xx_PCLK_STOP_ASBUS2 + 1,
					  0xFFFF, &actualRate);
	expect_failure_and_check(dm75xx_status, EINVAL);
	/*##########################################################################
	   Check Analog to Digital Library Functions
	   ###################################################################### */
	fprintf(stdout,
		"\n\n####################################################\n"
		"Analog to Digital Functions\n"
		"######################################################\n\n");
	/*========================
	 * DM75xx_ADC_Conv_Signal
	 =========================*/
	/*
	 * Invalid Parameters
	 */
	fprintf(stdout, "Verifying DM75xx_ADC_Conv_Signal ... \n");
	fprintf(stdout, "\tInvalid Conversion Signal\t");
	DM75xx_ADC_Conv_Signal(board, DM75xx_ADC_CONV_SIGNAL_SBUS2 + 1);
	expect_failure_and_check(dm75xx_status, EINVAL);
	/*
	 * Valid Parameters
	 */
	fprintf(stdout, "\tValidating ADC ... \n");
	for (i = DM75xx_ADC_CONV_SIGNAL_SOFTWARE;
	     i <= DM75xx_ADC_CONV_SIGNAL_SBUS2; i++) {
		fprintf(stdout, "\t\tConversion Signal %d\t", i);
		dm75xx_status = DM75xx_ADC_Conv_Signal(board, i);
		expect_success(dm75xx_status);
	}
	/*##########################################################################
	   Check Digital to Analog Library Functions
	   ###################################################################### */
	fprintf(stdout,
		"\n\n####################################################\n"
		"Digital to Analog Functions\n"
		"######################################################\n\n");
	/*===============================
	 * DM75xx_DAC_Get_Update_Counter
	 ================================*/
	/*
	 * Invalid Parameters
	 */
	fprintf(stdout, "Verifying DM75xx_DAC_Get_Update_Counter ... \n");
	fprintf(stdout, "\tInvalid DAC Channel\t\t");
	dm75xx_status =
	    DM75xx_DAC_Get_Update_Counter(board, DM75xx_DAC2 + 1, &data16);
	expect_failure_and_check(dm75xx_status, EINVAL);
	/*
	 * Valid Parameters
	 */
	fprintf(stdout, "\tValidating DAC ... \n");
	for (i = DM75xx_DAC1; i <= DM75xx_DAC2; i++) {
		fprintf(stdout, "\t\tUpdate Counter %d\t", i);
		dm75xx_status =
		    DM75xx_DAC_Get_Update_Counter(board, i, &data16);
		expect_success(dm75xx_status);
	}
	/*===============================
	  DM75xx_DAC_Set_Update_Counter
	 ================================*/
	/*
	 * Invalid Parameters
	 */
	fprintf(stdout, "Verifying DM75xx_DAC_Set_Update_Counter ... \n");
	fprintf(stdout, "\tInvalid DAC Channel\t\t");
	dm75xx_status =
	    DM75xx_DAC_Set_Update_Counter(board, DM75xx_DAC2 + 1, data16);
	expect_failure_and_check(dm75xx_status, EINVAL);
	/*
	 * Valid Parameters
	 */
	fprintf(stdout, "\tValidating DAC ... \n");
	for (i = DM75xx_DAC1; i <= DM75xx_DAC2; i++) {
		fprintf(stdout, "\t\tUpdate Counter %d\t", i);
		dm75xx_status = DM75xx_DAC_Set_Update_Counter(board, i, data16);
		expect_success(dm75xx_status);
	}
	/*=======================
	 * DM75xx_DAC_Set_Range
	 ========================*/
	/*
	 * Invalid Parameters
	 */
	fprintf(stdout, "Verifying DM75xx_DAC_Set_Range ... \n");
	fprintf(stdout, "\tInvalid DAC Channel\t\t");
	dm75xx_status =
	    DM75xx_DAC_Set_Range(board, DM75xx_DAC2 + 1,
				 DM75xx_DAC_RANGE_UNIPOLAR_5);
	expect_failure_and_check(dm75xx_status, EINVAL);

	fprintf(stdout, "\tInvalid Range\t\t\t");
	dm75xx_status =
	    DM75xx_DAC_Set_Range(board, DM75xx_DAC1,
				 DM75xx_DAC_RANGE_BIPOLAR_10 + 1);
	expect_failure_and_check(dm75xx_status, EINVAL);
	/*
	 * Valid Parameters
	 */
	for (i = DM75xx_DAC1; i <= DM75xx_DAC2; i++) {
		fprintf(stdout, "\tValidating DAC %d ... \n", i);
		for (j = DM75xx_DAC_RANGE_UNIPOLAR_5;
		     j <= DM75xx_DAC_RANGE_BIPOLAR_10; j++) {
			fprintf(stdout, "\t\tRange %d\t\t\t", j);
			dm75xx_status = DM75xx_DAC_Set_Range(board, i, j);
			expect_success(dm75xx_status);
		}
	}
	/*======================
	 * DM75xx_DAC_Get_Range
	 =======================*/
	/*
	 * Invalid Parameters
	 */
	fprintf(stdout, "Verifying DM75xx_DAC_Set_Range ... \n");
	fprintf(stdout, "\tInvalid DAC Channel\t\t");
	dm75xx_status = DM75xx_DAC_Set_Range(board, DM75xx_DAC2 + 1, data16);
	expect_failure_and_check(dm75xx_status, EINVAL);
	/*
	 * Valid Parameters
	 */
	fprintf(stdout, "\tValidating DAC ... \n");
	for (i = DM75xx_DAC1; i <= DM75xx_DAC2; i++) {
		fprintf(stdout, "\t\tDAC Channel %d\t\t", i);
		dm75xx_status = DM75xx_DAC_Set_Range(board, i, data16);
		expect_success(dm75xx_status);
	}
	/*==============================
	 * DM75xx_DAC_Set_Update_Source
	 ===============================*/
	/*
	 * Invalid Parameters
	 */
	fprintf(stdout, "Verifying DM75xx_DAC_Set_Update_Source ... \n");
	fprintf(stdout, "\tInvalid DAC Channel\t\t");
	dm75xx_status =
	    DM75xx_DAC_Set_Update_Source(board, DM75xx_DAC2 + 1, 0x00);
	expect_failure_and_check(dm75xx_status, EINVAL);

	fprintf(stdout, "\tInvalid Source\t\t\t");
	dm75xx_status =
	    DM75xx_DAC_Set_Update_Source(board, DM75xx_DAC1,
					 DM75xx_DAC_UPDATE_SBUS2 + 1);
	expect_failure_and_check(dm75xx_status, EINVAL);
	/*
	 * Valid Parameters
	 */
	for (i = DM75xx_DAC1; i <= DM75xx_DAC2; i++) {
		fprintf(stdout, "\tValidating DAC %d ... \n", i);
		for (j = DM75xx_DAC_UPDATE_SOFTWARE;
		     j <= DM75xx_DAC_UPDATE_SBUS2; j++) {
			fprintf(stdout, "\t\tUpdate Source %d\t\t", j);
			dm75xx_status =
			    DM75xx_DAC_Set_Update_Source(board, i, j);
			expect_success(dm75xx_status);
		}
	}
	 /*=====================
	  * DM75xx_DAC_Set_Mode
	  ======================*/
	/*
	 * Invalid parameters
	 */
	fprintf(stdout, "Verifying DM75xx_DAC_Get_Mode ... \n");
	fprintf(stdout, "\tInvalid DAC Channel\t\t");
	dm75xx_status = DM75xx_DAC_Set_Mode(board, DM75xx_DAC2 + 1, 0x00);
	expect_failure_and_check(dm75xx_status, EINVAL);

	fprintf(stdout, "\tInvalid Mode\t\t\t");
	dm75xx_status =
	    DM75xx_DAC_Set_Mode(board, DM75xx_DAC1, DM75xx_DAC_MODE_CYCLE + 1);
	expect_failure_and_check(dm75xx_status, EINVAL);
	/*
	 * Valid parameters
	 */
	for (i = DM75xx_DAC1; i <= DM75xx_DAC2; i++) {
		fprintf(stdout, "\tValidating DAC %d ... \n", i);
		for (j = DM75xx_DAC_MODE_NOT_CYCLE; j <= DM75xx_DAC_MODE_CYCLE;
		     j++) {
			fprintf(stdout, "\t\tMode %d\t\t\t", j);
			dm75xx_status = DM75xx_DAC_Set_Mode(board, i, j);
			expect_success(dm75xx_status);
		}
	}
	/*==========================
	 * DM75xx_DAC_Set_Frequency
	 ===========================*/
	/*
	 * Invalid parameters
	 */
	fprintf(stdout, "Verifying DM75xx_DAC_Set_Frequency ... \n");
	fprintf(stdout, "\tInvalid Clock\t\t\t");
	dm75xx_status =
	    DM75xx_DAC_Set_Frequency(board, DM75xx_DAC_FREQ_20_MHZ + 1);
	expect_failure_and_check(dm75xx_status, EINVAL);
	/*
	 * Valid parameters
	 */
	fprintf(stdout, "\tValidating DAC Primary Frequency ... \n");
	for (i = DM75xx_DAC_FREQ_8_MHZ; i <= DM75xx_DAC_FREQ_20_MHZ; i++) {
		fprintf(stdout, "\t\tDAC Frequency %d\t\t", i);
		dm75xx_status = DM75xx_DAC_Set_Frequency(board, i);
		expect_success(dm75xx_status);
	}
	/*=====================
	 * DM75xx_DAC_Set_Rate
	 ======================*/
	/*
	 * Invalid parameters
	 */
	fprintf(stdout, "Verifying DM75xx_DAC_Set_Rate ... \n");
	fprintf(stdout, "\tInvalid Clock Source\t\t");
	dm75xx_status = DM75xx_DAC_Set_Rate(board, 0xFF, 0xFFFF, &actualRate);
	expect_failure_and_check(dm75xx_status, EINVAL);

	fprintf(stdout, "\tInvalid Rate\t\t\t");
	dm75xx_status =
	    DM75xx_DAC_Set_Rate(board, 0x00, 0xFFFFFFFF, &actualRate);
	expect_failure_and_check(dm75xx_status, EINVAL);
	/*
	 * Valid parameters
	 */
	fprintf(stdout,
		"The following DAC rate tests could take a few minutes ... \n");
	for (i = DM75xx_DAC_FREQ_8_MHZ; i <= DM75xx_DAC_FREQ_20_MHZ; i++) {
		fprintf(stdout, "\tValidating DAC Rate %d\t\t", i);
		for (j = 0x0001; j <= 0xFFFFFF; j++) {
			dm75xx_status =
			    DM75xx_DAC_Set_Rate(board, i, j, &actualRate);
			if (dm75xx_status != 0) {
				error(EXIT_FAILURE, 0,
				      "FAILED %d: Expected success but failure occurred.",
				      status);
			}
		}
		fprintf(stdout, "Passed!\n");
	}
	/*============================
	 * DM75xx_DAC_Set_Clock_Start
	 =============================*/
	/*
	 * Invalid parameters
	 */
	fprintf(stdout, "Verifying DM75xx_DAC_Set_Clock_Start ... \n");
	fprintf(stdout, "\tInvalid Start\t\t\t");
	dm75xx_status =
	    DM75xx_DAC_Set_Clock_Start(board,
				       DM75xx_DAC_CLK_START_SOFTWARE + 1);
	expect_failure_and_check(dm75xx_status, EINVAL);
	/*
	 * Valid parameters
	 */
	fprintf(stdout, "\tValidating DAC Clock Start\n");
	for (i = DM75xx_DAC_CLK_START_SOFTWARE_PACER;
	     i <= DM75xx_DAC_CLK_START_SOFTWARE; i++) {
		fprintf(stdout, "\t\tStart %d\t\t\t", i);
		dm75xx_status = DM75xx_DAC_Set_Clock_Start(board, i);
		expect_success(dm75xx_status);
	}
	/*============================
	 * DM75xx_DAC_Set_Clock_Stop
	 =============================*/
	/*
	 * Invalid parameters
	 */
	fprintf(stdout, "Verifying DM75xx_DAC_Set_Clock_Stop ... \n");
	fprintf(stdout, "\tInvalid Stop\t\t\t");
	dm75xx_status =
	    DM75xx_DAC_Set_Clock_Stop(board, DM75xx_DAC_CLK_STOP_DAC2_UCNT + 1);
	expect_failure_and_check(dm75xx_status, EINVAL);
	/*
	 * Valid parameters
	 */
	fprintf(stdout, "\tValidating DAC Clock Stop\n");
	for (i = DM75xx_DAC_CLK_STOP_SOFTWARE_PACER;
	     i <= DM75xx_DAC_CLK_STOP_DAC2_UCNT; i++) {
		fprintf(stdout, "\t\tStart %d\t\t\t", i);
		dm75xx_status = DM75xx_DAC_Set_Clock_Stop(board, i);
		expect_success(dm75xx_status);
	}
	/*=========================
	 * DM75xx_DAC_CLK_Mode
	 ==========================*/
	/*
	 * Invalid parameters
	 */
	fprintf(stdout, "Verifying DM75xx_DAC_Set_CLK_Mode ... \n");
	fprintf(stdout, "\tInvalid Mode\t\t\t");
	dm75xx_status =
	    DM75xx_DAC_Set_CLK_Mode(board, DM75xx_DAC_CLK_START_STOP + 1);
	expect_failure_and_check(dm75xx_status, EINVAL);
	/*
	 * Valid parameters
	 */
	fprintf(stdout, "\tValidating DAC Clock Mode\n");
	for (i = DM75xx_DAC_CLK_FREE_RUN; i <= DM75xx_DAC_CLK_START_STOP; i++) {
		fprintf(stdout, "\t\tMode %d\t\t\t", i);
		dm75xx_status = DM75xx_DAC_Set_CLK_Mode(board, i);
		expect_success(dm75xx_status);
	}
	/*##########################################################################
	   HSDIN Library Functions
	   ###################################################################### */
	fprintf(stdout,
		"\n\n####################################################\n"
		"High Speed Digital Functions\n"
		"######################################################\n\n");
	/*==============================
	 * DM75xx_HSDIN_Sample_Signal
	 ===============================*/
	/*
	 * Invalid parameters
	 */
	fprintf(stdout, "Verifying DM75xx_HSDIN_Sample_Signal ... \n");
	fprintf(stdout, "\tInvalid Start\t\t\t");
	dm75xx_status =
	    DM75xx_HSDIN_Sample_Signal(board, DM75xx_HSDIN_SIGNAL_ETRIG + 1);
	expect_failure_and_check(dm75xx_status, EINVAL);
	/*
	 * Valid parameters
	 */
	fprintf(stdout, "\tValidating Sample Signal\n");
	for (i = DM75xx_HSDIN_SIGNAL_SOFTWARE; i <= DM75xx_HSDIN_SIGNAL_ETRIG;
	     i++) {
		fprintf(stdout, "\t\tSignal %d\t\t", i);
		dm75xx_status = DM75xx_HSDIN_Sample_Signal(board, i);
		expect_success(dm75xx_status);
	}
	/*##########################################################################
	   SyncBus Library Functions
	   ###################################################################### */
	fprintf(stdout,
		"\n\n####################################################\n"
		"SyncBus Functions\n"
		"######################################################\n\n");
	/*===========================
	 * DM75xx_SBUS_Set_Source
	 ============================*/
	/*
	 * Invalid parameters
	 */
	fprintf(stdout, "Verifying DM75xx_SBUS_Set_Source ... \n");
	fprintf(stdout, "\tInvalid SyncBus\t\t\t");
	dm75xx_status =
	    DM75xx_SBUS_Set_Source(board, DM75xx_SBUS2 + 1,
				   DM75xx_SBUS_SRC_UTC2);
	expect_failure_and_check(dm75xx_status, EINVAL);
	fprintf(stdout, "\tInvalid Source\t\t\t");
	dm75xx_status =
	    DM75xx_SBUS_Set_Source(board, DM75xx_SBUS0,
				   DM75xx_SBUS_SRC_UTC2 + 1);
	expect_failure_and_check(dm75xx_status, EINVAL);
	/*
	 * Valid parameters
	 */
	for (i = DM75xx_SBUS0; i <= DM75xx_SBUS2; i++) {
		fprintf(stdout, "\tValidating SyncBus %d\n", i);
		for (j = DM75xx_SBUS_SRC_SOFT_ADC; j <= DM75xx_SBUS_SRC_UTC2;
		     j++) {
			fprintf(stdout, "\t\tSource %d\t\t", j);
			dm75xx_status = DM75xx_SBUS_Set_Source(board, i, j);
			expect_success(dm75xx_status);
		}
	}
	/*##########################################################################
	   About Counter Library Functions
	   ###################################################################### */
	fprintf(stdout,
		"\n\n####################################################\n"
		"About Counter Functions\n"
		"######################################################\n\n");
	/*===========================
	 * DM75xx_ACNT_Set_Count
	 ============================*/
	/*
	 * Valid parameters
	 */
	fprintf(stdout, "Verifying DM75xx_ACNT_Set_Count ... \n");
	fprintf(stdout, "\tValidating Count\n");
	for (i = 0x0000; i <= 0xFFFF; i++) {
		fprintf(stdout, "\t\tCount 0x%4x\t\t", i);
		dm75xx_status = DM75xx_ACNT_Set_Count(board, i);
		if (dm75xx_status != 0) {
			error(EXIT_FAILURE, 0,
			      "FAILED %d: Expected success but failure occurred.",
			      status);
		}
		fprintf(stdout, "Passed!\r");
	}
	fprintf(stdout, "\n");
	/*##########################################################################
	   Delay Counter Library Functions
	   ###################################################################### */
	fprintf(stdout,
		"\n\n####################################################\n"
		"Delay Counter Functions\n"
		"######################################################\n\n");
	/*===========================
	 * DM75xx_DCNT_Set_Count
	 ============================*/
	/*
	 * Valid parameters
	 */
	fprintf(stdout, "Verifying DM75xx_DCNT_Set_Count ... \n");
	fprintf(stdout, "\tValidating Count\n");
	for (i = 0x0000; i <= 0xFFFF; i++) {
		fprintf(stdout, "\t\tCount 0x%4x\t\t", i);
		dm75xx_status = DM75xx_DCNT_Set_Count(board, i);
		if (dm75xx_status != 0) {
			error(EXIT_FAILURE, 0,
			      "FAILED %d: Expected success but failure occurred.",
			      status);
		}
		fprintf(stdout, "Passed!\r");
	}
	fprintf(stdout, "\n");
	/*##########################################################################
	   External Trigger/Interrupt Polarity
	   ###################################################################### */
	fprintf(stdout,
		"\n\n####################################################\n"
		"External Trigger/Interrupt Functions\n"
		"######################################################\n\n");
	/*==============================
	 * DM75xx_ETRIG_Polarity_Select
	 ===============================*/
	/*
	 * Invalid Parameters.
	 */
	fprintf(stdout, "Verifying DM75xx_ETRIG_Polarity_Select ... \n");
	fprintf(stdout, "\tInvalid Polarity\t\t");
	dm75xx_status =
	    DM75xx_ETRIG_Polarity_Select(board, DM75xx_EXT_POLARITY_NEG + 1);
	expect_failure_and_check(dm75xx_status, EINVAL);
	/*
	 * Valid Parameters.
	 */
	fprintf(stdout, "\tValiding Polarity\n");
	for (i = DM75xx_EXT_POLARITY_POS; i <= DM75xx_EXT_POLARITY_NEG; i++) {
		fprintf(stdout, "\t\tPolarity %d\t\t", i);
		dm75xx_status = DM75xx_ETRIG_Polarity_Select(board, i);
		expect_success(dm75xx_status);
	}
	/*==============================
	 * DM75xx_EINT_Polarity_Select
	 ===============================*/
	/*
	 * Invalid Parameters.
	 */
	fprintf(stdout, "Verifying DM75xx_EINT_Polarity_Select ... \n");
	fprintf(stdout, "\tInvalid Polarity\t\t");
	dm75xx_status =
	    DM75xx_EINT_Polarity_Select(board, DM75xx_EXT_POLARITY_NEG + 1);
	expect_failure_and_check(dm75xx_status, EINVAL);
	/*
	 * Valid Parameters.
	 */
	fprintf(stdout, "\tValidating Polarity\n");
	for (i = DM75xx_EXT_POLARITY_POS; i <= DM75xx_EXT_POLARITY_NEG; i++) {
		fprintf(stdout, "\t\tPolarity %d\t\t", i);
		dm75xx_status = DM75xx_EINT_Polarity_Select(board, i);
		expect_success(dm75xx_status);
	}
	/*##########################################################################
	   Digital I/O
	   ###################################################################### */
	fprintf(stdout,
		"\n\n####################################################\n"
		"Digital I/O Functions\n"
		"######################################################\n\n");
	/*=====================
	 * DM75xx_DIO_Set_Port
	 ======================*/
	/*
	 * Invalid Parameters
	 */
	fprintf(stdout, "Verifying DM75xx_DIO_Set_Port ... \n");
	fprintf(stdout, "\tInvalid Polarity\t\t");
	dm75xx_status = DM75xx_DIO_Set_Port(board, DM75xx_DIO_PORT1 + 1, 0x00);
	expect_failure_and_check(dm75xx_status, EINVAL);
	/*
	 * Valid Parameters.
	 */
	fprintf(stdout, "\tValidating DIO Port\n");
	for (i = DM75xx_DIO_PORT0; i <= DM75xx_DIO_PORT1; i++) {
		fprintf(stdout, "\t\tPort %d\t\t\t", i);
		dm75xx_status = DM75xx_DIO_Set_Port(board, i, 0x00);
		expect_success(dm75xx_status);
	}
	/*=====================
	 * DM75xx_DIO_Get_Port
	  =====================*/
	/*
	 * Invalid Parameters
	 */
	fprintf(stdout, "Verifying DM75xx_DIO_Get_Port ... \n");
	fprintf(stdout, "\tInvalid Port\t\t\t");
	dm75xx_status =
	    DM75xx_DIO_Get_Port(board, DM75xx_DIO_PORT1 + 1, &data8);
	expect_failure_and_check(dm75xx_status, EINVAL);
	/*
	 * Valid Parameters
	 */
	fprintf(stdout, "\tValidating DIO Ports\n");
	for (i = DM75xx_DIO_PORT0; i <= DM75xx_DIO_PORT1; i++) {
		fprintf(stdout, "\t\tPort %d\t\t\t", i);
		dm75xx_status = DM75xx_DIO_Get_Port(board, i, &data8);
		expect_success(dm75xx_status);
	}
	/*=========================
	 * DM75xx_DIO_Set_Direction
	 ==========================*/
	/*
	 * Invalid parameters
	 */
	fprintf(stdout, "Verifying DM75xx_DIO_Set_Diretion ... \n");
	fprintf(stdout, "\tInvalid Port\t\t\t");
	dm75xx_status =
	    DM75xx_DIO_Set_Direction(board, DM75xx_DIO_PORT1 + 1, 0x00);
	expect_failure_and_check(dm75xx_status, EINVAL);
	/*
	 * Vaild parameters
	 */
	fprintf(stdout, "\tValidating DIO Port Direction\n");
	for (i = DM75xx_DIO_PORT0; i <= DM75xx_DIO_PORT1; i++) {
		fprintf(stdout, "\tPort %d\n", i);
		for (j = 0x00; j <= 0xFF; j++) {
			fprintf(stdout, "\t\tDirection 0x%4x\t", j);
			dm75xx_status = DM75xx_DIO_Set_Direction(board, i, j);
			if (dm75xx_status != 0) {
				error(EXIT_FAILURE, 0,
				      "FAILED %d: Expected success but failure occurred.",
				      status);
			}
			fprintf(stdout, "Passed!\r");
		}
		fprintf(stdout, "\n");
	}
	/*=====================
	 * DM75xx_DIO_Set_Mask
	 ======================*/
	/*
	 * Valid parameters
	 */
	fprintf(stdout, "Verifying DM75xx_DIO_Set_Mask ... \n");
	fprintf(stdout, "\tValidating DIO Mask\n");
	for (i = 0x00; i <= 0xFF; i++) {
		fprintf(stdout, "\t\tMask 0x%4x\t\t", i);
		dm75xx_status = DM75xx_DIO_Set_Mask(board, i);
		if (dm75xx_status != 0) {
			error(EXIT_FAILURE, 0,
			      "FAILED %d: Expected success but failure occurred.",
			      status);
		}
		fprintf(stdout, "Passed!\r");
	}
	fprintf(stdout, "\n");
	/*=======================
	 * DM75xx_DIO_Set_Compare
	 ========================*/
	/*
	 * Valid parameters
	 */
	fprintf(stdout, "Verifying DM75xx_DIO_Set_Compare ... \n");
	fprintf(stdout, "\tValidating DIO Compare\n");
	for (i = 0x00; i <= 0xFF; i++) {
		fprintf(stdout, "\t\tCompare 0x%4x\t\t", i);
		dm75xx_status = DM75xx_DIO_Set_Compare(board, i);
		if (dm75xx_status != 0) {
			error(EXIT_FAILURE, 0,
			      "FAILED %d: Expected success but failure occurred.",
			      status);
		}
		fprintf(stdout, "Passed!\r");
	}
	fprintf(stdout, "\n");
	/*=====================
	 * DM75xx_DIO_IRQ_Mode
	 ======================*/
	/*
	 * Invalid parameters
	 */
	fprintf(stdout, "Verifying DM75xx_DIO_IRQ_Mode ... \n");
	fprintf(stdout, "\tInvalid IRQ Mode\t\t");
	dm75xx_status = DM75xx_DIO_IRQ_Mode(board, DM75xx_DIO_MODE_MATCH + 1);
	expect_failure_and_check(dm75xx_status, EINVAL);
	/*
	 * Valid parameters
	 */
	fprintf(stdout, "\tValidating IRQ Mode\n");
	for (i = DM75xx_DIO_MODE_EVENT; i <= DM75xx_DIO_MODE_MATCH; i++) {
		fprintf(stdout, "\t\tMode %d\t\t\t", i);
		dm75xx_status = DM75xx_DIO_IRQ_Mode(board, i);
		expect_success(dm75xx_status);
	}
	/*==================
	 * DM75xx_DIO_Clock
	 ===================*/
	/*
	 * Invalid parameters
	 */
	fprintf(stdout, "Verifying DM75xx_DIO_Clock ... \n");
	fprintf(stdout, "\tInvaild DIO Clock\t\t");
	dm75xx_status = DM75xx_DIO_Clock(board, DM75xx_DIO_CLK_UTC1 + 1);
	expect_failure_and_check(dm75xx_status, EINVAL);
	/*
	 * Valid parameters
	 */
	fprintf(stdout, "\tValidating Digital I/O Clock Sources\n");
	for (i = DM75xx_DIO_CLK_8MHZ; i <= DM75xx_DIO_CLK_UTC1; i++) {
		fprintf(stdout, "\t\tClock %d\t\t\t", i);
		dm75xx_status = DM75xx_DIO_Clock(board, i);
		expect_success(dm75xx_status);
	}
	/*##########################################################################
	   User I/O
	   ###################################################################### */
	fprintf(stdout,
		"\n\n####################################################\n"
		"User I/O Functions\n"
		"######################################################\n\n");
	/*==================
	 * DM75xx_UIO_Select
	 ===================*/
	/*
	 * Invaild parameters
	 */
	fprintf(stdout, "Verifying DM75xx_UIO_Select ... \n");
	fprintf(stdout, "\tInvalid Channel\t\t\t");
	dm75xx_status =
	    DM75xx_UIO_Select(board, DM75xx_UIO1 + 1, DM75xx_UIO_ADC);
	expect_failure_and_check(dm75xx_status, EINVAL);

	fprintf(stdout, "\tInvalid Source\t\t\t");
	dm75xx_status =
	    DM75xx_UIO_Select(board, DM75xx_UIO0, DM75xx_UIO_PRG + 1);
	expect_failure_and_check(dm75xx_status, EINVAL);
	/*
	 * Valid paramters
	 */
	fprintf(stdout, "\tValidating User I/O Sources\n");
	for (i = DM75xx_UIO0; i <= DM75xx_UIO1; i++) {
		fprintf(stdout, "\tChannel %d\n", i);
		for (j = DM75xx_UIO_ADC; j <= DM75xx_UIO_PRG; j++) {
			fprintf(stdout, "\t\tSource %d\t\t", j);
			dm75xx_status = DM75xx_UIO_Select(board, i, j);
			expect_success(dm75xx_status);
		}
		fprintf(stdout, "\n");
	}
	/*##########################################################################
	   DMA
	   ###################################################################### */
	fprintf(stdout,
		"\n\n####################################################\n"
		"DMA Functions\n"
		"######################################################\n\n");
	/*======================
	 * DM75xx_DMA_Initialize
	 =======================*/
	/*
	 * Invalid parameters
	 */
	fprintf(stdout, "Verifying DM75xx_DMA_Initialize ... \n");
	fprintf(stdout, "\tInvalid Channel\t\t\t");
	dm75xx_status = DM75xx_DMA_Initialize(board, DM75xx_DMA_CHANNEL_1 + 1,
					      0x00, 0x00, 0x00, &buf);
	expect_failure_and_check(dm75xx_status, EINVAL);

	fprintf(stdout, "\tInvalid Source\t\t\t");
	dm75xx_status =
	    DM75xx_DMA_Initialize(board, 0x00, DM75xx_DMA_FIFO_HSDIN + 1, 0x00,
				  0x00, &buf);
	expect_failure_and_check(dm75xx_status, EINVAL);

	fprintf(stdout, "\tInvalid Request\t\t\t");
	dm75xx_status = DM75xx_DMA_Initialize(board, 0x00, 0x00,
					      DM75xx_DMA_DEMAND_UTC1 + 1, 0x00,
					      &buf);
	expect_failure_and_check(dm75xx_status, EINVAL);

	fprintf(stdout, "\tInvalid Request\t\t\t");
	dm75xx_status = DM75xx_DMA_Initialize(board, 0x00, 0x00,
					      DM75xx_DMA_DEMAND_FIFO_ADC - 1,
					      0x00, &buf);
	expect_failure_and_check(dm75xx_status, EINVAL);

	fprintf(stdout, "\tInvalid Request\t\t\t");
	dm75xx_status = DM75xx_DMA_Initialize(board, 0x00, 0x00,
					      DM75xx_DMA_DEMAND_FIFO_DAC2 + 1,
					      0x00, &buf);
	expect_failure_and_check(dm75xx_status, EINVAL);
	/*##########################################################################
	   Interrupt
	   ###################################################################### */
	fprintf(stdout,
		"\n\n####################################################\n"
		"Interrupt Functions\n"
		"######################################################\n\n");
    /*========================
	 * DM75xx_Interrupt_Enable
	 =========================*/
	/*
	 * Invalid parameters
	 */
	fprintf(stdout, "Verifying DM75xx_Interrupt_Enable ... \n");
	/*
	 * Valid parameters
	 */
	fprintf(stdout, "\tValidating Interrupt Sources\n");
	for (i = 0x0001; i <= 0xFFFF; i++) {
		fprintf(stdout, "\t\tSource 0x%4x\t\t", i);
		dm75xx_status = DM75xx_Interrupt_Enable(board, i);
		if (dm75xx_status != 0) {
			error(EXIT_FAILURE, 0,
			      "FAILED %d: Expected success but failure occurred.",
			      status);
		}
		fprintf(stdout, "Passed!\r");
	}
	/*##########################################################################
	   Cleanup and exit
	   ###################################################################### */
	/*
	 * Close the dm75xx device.
	 */
	fprintf(stdout,
		"\n\n+++ Library Tests Are Complete! +++\nClosing the dm75xx ... \n");
	dm75xx_status = DM75xx_Board_Close(board);
	if (dm75xx_status != 0) {
		error(EXIT_FAILURE, errno,
		      "ERROR: DM75xx_Board_Close() FAILED");
	}
	exit(EXIT_SUCCESS);
}
