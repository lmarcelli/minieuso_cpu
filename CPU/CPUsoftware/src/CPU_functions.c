#include "globals.h"

/* mount USB */
void mount_usb(char * usb_port)
{
	if (access(usb_port, F_OK) != -1) 
	{
		if (mount(usb_port, "/media/usb", MNT_UNION, "")) 
		{
			if (errno == EBUSY) 
			{
		    	tlog("ERROR","USB Mountpoint busy");
		    	printf("USB mountpoint busy");
		    } else 
		    {	
		    	tlog("ERROR","Mount error");
		    	printf("Mount error: %s", strerror(errno));
		    }
		} 
		else 
		{
		    tlog("INFO","Mount successful");
		}
	} 
	else 
	{
		tlog("ERROR","No USB connected");
	}
}

/* check IP address connection */
int check_IP_com(char * ip_address)
{
    printf("start");
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port   = htons(65432);  // Could be anything
    inet_pton(AF_INET, ip_address, &sin.sin_addr);

    if (connect(sockfd, (struct sockaddr *) &sin, sizeof(sin)) == -1)
    {
        printf("Error connecting: %d (%s)\n", errno, strerror(errno));
    }

    sin.sin_family = AF_INET;
    sin.sin_port   = htons(65432);  // Could be anything
    inet_pton(AF_INET, "192.168.0.9", &sin.sin_addr);

    if (connect(sockfd, (struct sockaddr *) &sin, sizeof(sin)) == -1)
    {
        printf("Error connecting 192.168.0.9: %d (%s)\n", errno, strerror(errno));
    }
    return 0;
}
