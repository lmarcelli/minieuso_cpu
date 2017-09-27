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

int main()
{
    int sockfd, portno, n, i;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];

    /* set up the telnet connection */
    portno = 23;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    server = gethostbyname("192.168.7.10");
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
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
    printf("Connected to telnet server\n");

    /* PDM commands */
    /* check the status */
    bzero(buffer,256);
    strncpy(buffer, "hvps status gpio\r", sizeof(buffer));
    printf("%s\n",buffer);
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
         error("ERROR writing to socket");
    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0) 
         error("ERROR reading from socket");
    printf("%s\n",buffer);

    /* set the cathode voltage */
    bzero(buffer,256);
    strncpy(buffer, "hvps cathode 3 3 3 3 3 3 3 3 3\r", sizeof(buffer));
    printf("%s\n",buffer);
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
         error("ERROR writing to socket");
    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0) 
         error("ERROR reading from socket");
    printf("%s\n",buffer);

    /* turn on the HV */ 
    bzero(buffer,256);
    strncpy(buffer, "hvps turnon 1 1 1 1 1 1 1 1 1\r", sizeof(buffer));
    printf("%s\n",buffer);
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
         error("ERROR writing to socket");
    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0) 
         error("ERROR reading from socket");
    printf("%s\n",buffer);
    
    /* set the dynode voltage */
    bzero(buffer,256);
    strncpy(buffer, "hvps setdac 3500 3500 3500 0 0 0 0 0 0\r", sizeof(buffer));
    printf("%s\n",buffer);
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
         error("ERROR writing to socket");
    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0) 
         error("ERROR reading from socket");
    printf("%s\n",buffer);

    /* check the status */
    bzero(buffer,256);
    strncpy(buffer, "hvps status gpio\r", sizeof(buffer));
    printf("%s\n",buffer);
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
         error("ERROR writing to socket");
    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0) 
         error("ERROR reading from socket");
    printf("%s\n",buffer);
    // CHECK STATUS

    /* loop over this */
    for (i=0; i<10; i++){
        /* take a frame */
        bzero(buffer,256);
        strncpy(buffer, "acq shot\r", sizeof(buffer));
        printf("%s\n",buffer);
        n = write(sockfd,buffer,strlen(buffer));
        if (n < 0) 
             error("ERROR writing to socket");
        bzero(buffer,256);
        n = read(sockfd,buffer,255);
        if (n < 0) 
             error("ERROR reading from socket");
        printf("%s\n",buffer);
        sleep(1);

        /* take an S-curve */
        bzero(buffer,256);
        strncpy(buffer, "acq sc_pdm\r", sizeof(buffer));
        printf("%s\n",buffer);
        n = write(sockfd,buffer,strlen(buffer));
        if (n < 0) 
             error("ERROR writing to socket");
        bzero(buffer,256);
        n = read(sockfd,buffer,255);
        if (n < 0) 
             error("ERROR reading from socket");
        printf("%s\n",buffer);
        sleep(3);

        /* check the status */
        bzero(buffer,256);
        strncpy(buffer, "hvps status gpio\r", sizeof(buffer));
        printf("%s\n",buffer);
        n = write(sockfd,buffer,strlen(buffer));
        if (n < 0) 
             error("ERROR writing to socket");
        bzero(buffer,256);
        n = read(sockfd,buffer,255);
        if (n < 0) 
             error("ERROR reading from socket");
        printf("%s\n",buffer);
        // CHECK STATUS AND DO SOMETHING IF WRONG
    }

    close(sockfd);
    return 0;
}