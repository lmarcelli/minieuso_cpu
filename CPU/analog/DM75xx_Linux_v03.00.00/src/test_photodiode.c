/*
F. Capel
Program to test acquisition from the photodiodes via the analog brd. 

All 16 analog channels are repeatedly read into the FIFO, resulting in 1024 samples. 
This is read out into a structure of size 1024 and saved as the file "outputX.dat",
where X is the packet number. 
Pass the number of iterations of the test program as an input.

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

#define CHANNELS 16
#define BURST_RATE 1000000
#define PACER_RATE 100000

struct acq {
	float val[1024];
};

/* Main program code */
int main(int argc, char *argv[])
{
	DM75xx_Board_Descriptor *brd;
	DM75xx_Error dm75xx_status;
	dm75xx_cgt_entry_t cgt[CHANNELS];
	int i, k, k_max = atoi(argv[1]);
	float actR;
	uint16_t data = 0x0000;

	unsigned long int minor_number = 0;
	struct acq *acq_output = malloc(sizeof(struct acq));

	FILE * logfile_pd = fopen("/home/minieusouser/log/test_photodiode_log.log","w");
    if (logfile_pd == NULL) {
        fprintf(stderr, "ERROR, log file not opened\n");
    }

	fprintf(logfile_pd, "\nPhotodiode test program\n\n");
	
	/* Device initialisation */
	fprintf(logfile_pd, "Initialising the device\n");
	dm75xx_status = DM75xx_Board_Open(minor_number, &brd);
	DM75xx_Exit_On_Error(brd, dm75xx_status, "DM75xx_Board_Open");
	dm75xx_status = DM75xx_Board_Init(brd);
	DM75xx_Exit_On_Error(brd, dm75xx_status, "DM75xx_Board_Init");

	/* Main acquisition code */
	for(k=0; k<k_max; k++){ 
		char fname[64];
    	snprintf(fname, sizeof(char) * 64, "/home/minieusouser/DATA/output%i.dat", k);  
		
		/* Clear the FIFO */
		fprintf(logfile_pd, "Clearing the FIFO\n");
		dm75xx_status = DM75xx_ADC_Clear(brd);
		DM75xx_Exit_On_Error(brd, dm75xx_status, "DM75xx_Clear_AD_FIFO");
		dm75xx_status = DM75xx_FIFO_Get_Status(brd, &data);
		DM75xx_Exit_On_Error(brd, dm75xx_status, "DM75xx_FIFO_Get_Status");
		fprintf(logfile_pd, "FIFO Status: 0x%4x\n", data);
		
		/* Enable the channel gain table */
		fprintf(logfile_pd, "Enable and set the channel gain table\n");
		dm75xx_status = DM75xx_CGT_Enable(brd, 0xFF);
		DM75xx_Exit_On_Error(brd, dm75xx_status, "DM75xx_CGT_Enable");
		
		/* Set the channel gain table for all channels */
		for (i = 0; i < CHANNELS; i++) {
			cgt[i].channel = i;
			cgt[i].gain = 0;
			cgt[i].nrse = 0;
			cgt[i].range = 0;
			cgt[i].ground = 0;
			cgt[i].pause = 0;
			cgt[i].dac1 = 0;
			cgt[i].dac2 = 0;
			cgt[i].skip = 0;
			dm75xx_status = DM75xx_CGT_Write(brd, cgt[i]);
			DM75xx_Exit_On_Error(brd, dm75xx_status, "DM75xx_CGT_Write");
		}

		/* Setup clocks */
		fprintf(logfile_pd, "Setting the burst and pacer clocks for multi-channel acquisition\n");
		dm75xx_status = DM75xx_BCLK_Setup(brd,
						  DM75xx_BCLK_START_PACER,
						  DM75xx_BCLK_FREQ_8_MHZ,
						  BURST_RATE, &actR);
		DM75xx_Exit_On_Error(brd, dm75xx_status, "DM75xx_PCLK_Setup");
		dm75xx_status = DM75xx_PCLK_Setup(brd,
						  DM75xx_PCLK_INTERNAL,
						  DM75xx_PCLK_FREQ_8_MHZ,
						  DM75xx_PCLK_NO_REPEAT,
						  DM75xx_PCLK_START_SOFTWARE,
						  DM75xx_PCLK_STOP_SOFTWARE,
						  PACER_RATE, &actR);
		DM75xx_Exit_On_Error(brd, dm75xx_status, "DM75xx_PCLK_Setup");
		
		/* Set ADC Conversion Signal Select */
		dm75xx_status =
		    DM75xx_ADC_Conv_Signal(brd, DM75xx_ADC_CONV_SIGNAL_BCLK);
		DM75xx_Exit_On_Error(brd, dm75xx_status, "DM75xx_ADC_Conv_Signal");
		
		/* Start the pacer clock */
		fprintf(logfile_pd, "Starting the pacer clock\n");
		dm75xx_status = DM75xx_PCLK_Start(brd);
		DM75xx_Exit_On_Error(brd, dm75xx_status, "DM75xx_PCLK_Start");

		/* Read data into the FIFO */
		fprintf(logfile_pd, "Collecting data until FIFO is full\n");
		do {
			dm75xx_status = DM75xx_FIFO_Get_Status(brd, &data);
			DM75xx_Exit_On_Error(brd, dm75xx_status,
					     "DM75xx_FIFO_Get_Status");
		} while (data & DM75xx_FIFO_ADC_NOT_FULL);
		
		/* Stop the pacer clock */
		fprintf(logfile_pd, "Stopping the pacer clock\n");
		dm75xx_status = DM75xx_PCLK_Stop(brd);
		DM75xx_Exit_On_Error(brd, dm75xx_status, "DM75xx_PCLK_Stop");
		
		/* Read out data from the FIFO */
		fprintf(logfile_pd, "Readout data from the FIFO\n");
		i = 0;
		do {
			
			/* Reading the FIFO */
			dm75xx_status = DM75xx_ADC_FIFO_Read(brd, &data);
			DM75xx_Exit_On_Error(brd, dm75xx_status,
					     "DM75xx_ADC_FIFO_Read");
			//fprintf(logfile_pd, "%2.2f\n",
			//	((DM75xx_ADC_ANALOG_DATA(data) / 4096.) * 10));
			acq_output->val[i]=((DM75xx_ADC_ANALOG_DATA(data) / 4096.) * 10);
		
			i++;
			
			/* Check the FIFO status each time */
			dm75xx_status = DM75xx_FIFO_Get_Status(brd, &data);
			DM75xx_Exit_On_Error(brd, dm75xx_status,
					     "DM75xx_FIFO_Get_Status");
		}
		while (data & DM75xx_FIFO_ADC_NOT_EMPTY);
		/* Print how many samples were received */
		fprintf(logfile_pd, "Received %d samples\n", i);
		
		/* Save FIFO output to a file */	
		FILE * file = fopen(fname,"wb");
		if (file != NULL){
			fwrite(acq_output, sizeof(struct acq), 1, file);
			fclose(file);
		}
		free(acq_output);
		sleep(5);
	}
	
	/* Program clean up */
	fprintf(logfile_pd, "Acquisition completed\n");

	/* Reset the brd */
	fprintf(logfile_pd, "Resetting the brd\n");
	dm75xx_status = DM75xx_Board_Reset(brd);
	DM75xx_Exit_On_Error(brd, dm75xx_status, "DM75xx_Board_Reset");
	
	/* Close the device */
	fprintf(logfile_pd, "Closing...\n");
	dm75xx_status = DM75xx_Board_Close(brd);
	DM75xx_Exit_On_Error(brd, dm75xx_status, "DM75xx_Board_Close");
	exit(EXIT_SUCCESS);

	fclose(logfile_pd);
}
