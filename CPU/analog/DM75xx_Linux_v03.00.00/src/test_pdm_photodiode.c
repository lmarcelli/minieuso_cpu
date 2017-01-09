/*
F. Capel

Program to test acquisition of the pdm and the photodiode sequentially when
debugging the power consumption.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <signal.h>
#include <stdint.h>

#include "dm75xx_library.h"

#define CHANNELS 16
#define BURST_RATE 1000000
#define PACER_RATE 100000

struct acq {
    float val[1024];
};

void myerror(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    /* pdm */
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];

    /* photodiode */
    DM75xx_Board_Descriptor *brd;
    DM75xx_Error dm75xx_status;
    dm75xx_cgt_entry_t cgt[CHANNELS];
    int i, k, k_max = atoi(argv[1]);
    float actR;
    uint16_t data = 0x0000;

    unsigned long int minor_number = 0;
    struct acq *acq_output = malloc(sizeof(struct acq));

    /* open the log files */
    FILE * logfile_pd = fopen("/home/minieusouser/log/test_photodiode_log.log","w");
    if (logfile_pd == NULL) {
        fprintf(stderr, "ERROR, log file not opened\n");
    }

    FILE * logfile_pdm = fopen("/home/minieusouser/log/test_pdm_log.log","w");
    if (logfile_pdm == NULL) {
        fprintf(stderr, "ERROR, log file not opened\n");
    }

    /*********************************/
    /* PDM SETUP */
    /* set up the telnet connection */
    portno = 23;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        myerror("ERROR opening socket");

    server = gethostbyname("192.168.7.10");
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        myerror("ERROR connecting");
    
    fprintf(logfile_pdm,"Connected to telnet server\n");

    /*********************************/  
    /* PHOTODIODE SETUP */
    fprintf(logfile_pd, "\nProgram to test the photodiode acquisition\n");
    
    /* Device initialisation */
    dm75xx_status = DM75xx_Board_Open(minor_number, &brd);
    DM75xx_Exit_On_Error(brd, dm75xx_status, "DM75xx_Board_Open");
    fprintf(logfile_pd, "Initialising the brd\n");
    dm75xx_status = DM75xx_Board_Init(brd);
    DM75xx_Exit_On_Error(brd, dm75xx_status, "DM75xx_Board_Init");

    /*********************************/
    /* PDM SWITCH ON */
    /* PDM commands */
    /* check the status */
    bzero(buffer, 256);
    strncpy(buffer, "hvps status gpio\r", sizeof(buffer));
    fprintf(logfile_pdm,"%s\n",buffer);
    n = write(sockfd, buffer, strlen(buffer));
    if (n < 0) 
         myerror("ERROR writing to socket");
    bzero(buffer, 256);
    n = read(sockfd, buffer, 255);
    if (n < 0) 
         myerror("ERROR reading from socket");
    fprintf(logfile_pdm, "%s\n", buffer);

    /* set the cathode voltage */
    bzero(buffer,256);
    strncpy(buffer, "hvps cathode 3 3 3 3 3 3 3 3 3\r", sizeof(buffer));
    fprintf(logfile_pdm, "%s\n", buffer);
    n = write(sockfd, buffer, strlen(buffer));
    if (n < 0) 
         myerror("ERROR writing to socket");
    bzero(buffer, 256);
    n = read(sockfd, buffer, 255);
    if (n < 0) 
         myerror("ERROR reading from socket");
    fprintf(logfile_pdm, "%s\n", buffer);

    /* turn on the HV */ 
    bzero(buffer,256);
    strncpy(buffer, "hvps turnon 1 1 1 1 1 1 1 1 1\r", sizeof(buffer));
    fprintf(logfile_pdm, "%s\n", buffer);
    n = write(sockfd, buffer, strlen(buffer));
    if (n < 0) 
         myerror("ERROR writing to socket");
    bzero(buffer, 256);
    n = read(sockfd, buffer, 255);
    if (n < 0) 
         myerror("ERROR reading from socket");
    fprintf(logfile_pdm, "%s\n", buffer);
    
    /* set the dynode voltage */
    bzero(buffer, 256);
    strncpy(buffer, "hvps setdac 3500 3500 3500 0 0 0 0 0 0\r", sizeof(buffer));
    fprintf(logfile_pdm, "%s\n", buffer);
    n = write(sockfd, buffer, strlen(buffer));
    if (n < 0) 
         myerror("ERROR writing to socket");
    bzero(buffer, 256);
    n = read(sockfd, buffer, 255);
    if (n < 0) 
         myerror("ERROR reading from socket");
    fprintf(logfile_pdm, "%s\n", buffer);

    /* check the status */
    bzero(buffer, 256);
    strncpy(buffer, "hvps status gpio\r", sizeof(buffer));
    fprintf(logfile_pdm,"%s\n", buffer);
    n = write(sockfd, buffer, strlen(buffer));
    if (n < 0) 
         myerror("ERROR writing to socket");
    bzero(buffer, 256);
    n = read(sockfd, buffer, 255);
    if (n < 0) 
         myerror("ERROR reading from socket");
    fprintf(logfile_pdm, "%s\n", buffer);

	sleep(2);

    /*********************************/
    /* LOOP WITH PDM AND PHOTODIODE */

    /* loop over acquisition */
    for (k=0; k<k_max; k++){

        /* take a frame */
        bzero(buffer,256);
        strncpy(buffer, "acq shot\r", sizeof(buffer));
        fprintf(logfile_pdm, "%s\n", buffer);
        n = write(sockfd, buffer, strlen(buffer));
        if (n < 0) 
             myerror("ERROR writing to socket");
        bzero(buffer, 256);
        n = read(sockfd, buffer, 255);
        if (n < 0) 
             myerror("ERROR reading from socket");
        fprintf(logfile_pdm, "%s\n", buffer);
	    
        sleep(3);
        
        /* take an S-curve */
        bzero(buffer,256);
        strncpy(buffer, "acq sc_pdm\r", sizeof(buffer));
        fprintf(logfile_pdm, "%s\n", buffer);
        n = write(sockfd, buffer, strlen(buffer));
        if (n < 0) 
             myerror("ERROR writing to socket");
        bzero(buffer, 256);
        n = read(sockfd, buffer, 255);
        if (n < 0) 
             myerror("ERROR reading from socket");
        fprintf(logfile_pdm, "%s\n", buffer);
	    
        sleep(10);

        /* check the status */
        bzero(buffer,256);
        strncpy(buffer, "hvps status gpio\r", sizeof(buffer));
        fprintf(logfile_pdm, "%s\n", buffer);
        n = write(sockfd, buffer, strlen(buffer));
        if (n < 0) 
             myerror("ERROR writing to socket");
        bzero(buffer,256);
        n = read(sockfd,buffer,255);
        if (n < 0) 
             myerror("ERROR reading from socket");
        fprintf(logfile_pdm, "%s\n", buffer);
        /* could read in and check the status here */

        char fname[64];
        snprintf(fname, sizeof(char) * 64, "/home/minieusouser/DATA/output%i.dat", k);  
        
        /* Clear ADC FIFO */
        fprintf(logfile_pd, "Cearing the FIFO\n");
        dm75xx_status = DM75xx_ADC_Clear(brd);
        DM75xx_Exit_On_Error(brd, dm75xx_status, "DM75xx_Clear_AD_FIFO");
        
        /* Check FIFO Status */
        dm75xx_status = DM75xx_FIFO_Get_Status(brd, &data);
        DM75xx_Exit_On_Error(brd, dm75xx_status, "DM75xx_FIFO_Get_Status");
        fprintf(logfile_pd, "FIFO Status: 0x%4x\n", data);
        
        /* Enable Channel Gain Table */
        fprintf(logfile_pd, "Enabling channel gain table\n");
        dm75xx_status = DM75xx_CGT_Enable(brd, 0xFF);
        DM75xx_Exit_On_Error(brd, dm75xx_status, "DM75xx_CGT_Enable");
        
        /* Create CGT Entry */
        fprintf(logfile_pd, "Set the chanel gain table\n");
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
        fprintf(logfile_pd, "Setting up the clocks\n");
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
        fprintf(logfile_pd, "starting the pacer clock\n");
        dm75xx_status = DM75xx_PCLK_Start(brd);
        DM75xx_Exit_On_Error(brd, dm75xx_status, "DM75xx_PCLK_Start");

        fprintf(logfile_pd, "Collecting the data until the FIFO is full\n");
        do {
            /* Check FIFO Status */
            dm75xx_status = DM75xx_FIFO_Get_Status(brd, &data);
            DM75xx_Exit_On_Error(brd, dm75xx_status,
                         "DM75xx_FIFO_Get_Status");
        } while (data & DM75xx_FIFO_ADC_NOT_FULL);
        
        /* Stop the pacer clock */
        fprintf(logfile_pd, "Stopping the pacer clock\n");
        dm75xx_status = DM75xx_PCLK_Stop(brd);
        DM75xx_Exit_On_Error(brd, dm75xx_status, "DM75xx_PCLK_Stop");
        
        /* Read out samples */
        fprintf(logfile_pd, "Reading the FIFO\n");
        i = 0;
        do {
            
            /* Read AD FIFO */
            dm75xx_status = DM75xx_ADC_FIFO_Read(brd, &data);
            DM75xx_Exit_On_Error(brd, dm75xx_status,
                         "DM75xx_ADC_FIFO_Read");
            acq_output->val[i]=((DM75xx_ADC_ANALOG_DATA(data) / 4096.) * 10);
            
            i++;
            
            /* Read the FIFO Status */
            dm75xx_status = DM75xx_FIFO_Get_Status(brd, &data);
            DM75xx_Exit_On_Error(brd, dm75xx_status,
                         "DM75xx_FIFO_Get_Status");
        }
        while (data & DM75xx_FIFO_ADC_NOT_EMPTY);
        /* Print how many samples were received */
        fprintf(logfile_pd, "Received %d samples\n", i);
        
        /* Save samples to a file */    
        FILE * file = fopen(fname,"wb");
        if (file != NULL){
            fwrite(acq_output, sizeof(struct acq), 1, file);
            fclose(file);
        }

        free(acq_output);
        sleep(3);
    }

    /****************************************/
    /* PROGRAM CLEAN UP */
    /* Reset the board */
    fprintf(logfile_pd, "Restting the brd\n");
    dm75xx_status = DM75xx_Board_Reset(brd);
    DM75xx_Exit_On_Error(brd, dm75xx_status, "DM75xx_Board_Reset");
    
    /* Close the dm75xx device */
    fprintf(logfile_pd, "Clsoing the device\n");
    dm75xx_status = DM75xx_Board_Close(brd);
    DM75xx_Exit_On_Error(brd, dm75xx_status, "DM75xx_Board_Close");
    exit(EXIT_SUCCESS);

    fprintf(logfile_pdm, "Exiting...");
    fclose(logfile_pdm);
    fclose(logfile_pd);
    close(sockfd);
    return 0;
}
