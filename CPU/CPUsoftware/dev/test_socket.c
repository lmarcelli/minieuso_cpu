#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h> 
#include <stdio.h>

int main() {

	int status;
	struct addrinfo hints;
	struct addrinfo *res;  // will point to the results
	char *msg = "acq shot";
	char *buf;
	int len, bytes_sent, bytes_recv;
	int sockfd;

	memset(&hints, 0, sizeof hints); // make sure the struct is empty
	hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
	

	// get ready to connect
	printf("get ready to connect\n");
	status = getaddrinfo("192.16.7.10", "23", &hints, &res);
	// servinfo now points to a linked list of 1 or more struct addrinfos
	// etc.

	printf("make a socket\n");
	//make a socket
	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	printf("bind to port\n");
	//bind to port (not stricly necessary as we only care about the remote port)
	bind(sockfd, res->ai_addr, res->ai_addrlen);

	printf("connect\n");
	//connect
	connect(sockfd, res->ai_addr, res->ai_addrlen);

	printf("send msg\n");
	//send msg
	len = strlen(msg);
	bytes_sent = send(sockfd, msg, len, 0);


	printf("recv buf:\n");
	bytes_recv = recv(sockfd, buf, len, 0);
	printf("%s", buf);

	//close
	close(sockfd);

	return 0;
}