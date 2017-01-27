#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n, acq_max = atoi(argv[1]);
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];

    /* open the log file */
    FILE * logfile = fopen("/home/minieusouser/log/test_pdm_log.log","w");
    if (logfile == NULL) {
        fprintf(stderr, "ERROR, log file not opened\n");
    }

    /* set up the telnet connection */
    portno = 23;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

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
        error("ERROR connecting");
    
    fprintf(logfile,"Connected to telnet server\n");

    /* PDM commands */
    /* check the status */
    bzero(buffer, 256);
    strncpy(buffer, "instrument status\r", sizeof(buffer));
    fprintf(logfile,"%s\n",buffer);
    n = write(sockfd, buffer, strlen(buffer));
    if (n < 0) 
         error("ERROR writing to socket");
    bzero(buffer, 256);
    n = read(sockfd, buffer, 255);
    if (n < 0) 
         error("ERROR reading from socket");
    fprintf(logfile, "%s\n", buffer);

    /* set to acquisition mode one */
    bzero(buffer,256);
    strncpy(buffer, "instrument mode 1\r", sizeof(buffer));
    fprintf(logfile, "%s\n", buffer);
    n = write(sockfd, buffer, strlen(buffer));
    if (n < 0) 
         error("ERROR writing to socket");
    bzero(buffer, 256);
    n = read(sockfd, buffer, 255);
    if (n < 0) 
         error("ERROR reading from socket");
    fprintf(logfile, "%s\n", buffer);

    /* start the aquisition */ 
    bzero(buffer,256);
    strncpy(buffer, "instrument start\r", sizeof(buffer));
    fprintf(logfile, "%s\n", buffer);
    n = write(sockfd, buffer, strlen(buffer));
    if (n < 0) 
         error("ERROR writing to socket");
    bzero(buffer, 256);
    n = read(sockfd, buffer, 255);
    if (n < 0) 
         error("ERROR reading from socket");
    fprintf(logfile, "%s\n", buffer);
    
    /* sleep for a multiple of 5s */
	sleep(acq_max*5);

    /* stop the aquisition */ 
    bzero(buffer,256);
    strncpy(buffer, "instrument stop\r", sizeof(buffer));
    fprintf(logfile, "%s\n", buffer);
    n = write(sockfd, buffer, strlen(buffer));
    if (n < 0) 
         error("ERROR writing to socket");
    bzero(buffer, 256);
    n = read(sockfd, buffer, 255);
    if (n < 0) 
         error("ERROR reading from socket");
    fprintf(logfile, "%s\n", buffer);


    fprintf(logfile, "Exiting...");
    fclose(logfile);
    close(sockfd);
    return 0;
}
