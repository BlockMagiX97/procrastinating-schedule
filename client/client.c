#include "../record.h"
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define PORT 8081

int main() {
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = INADDR_ANY;
	serveraddr.sin_port = htons(PORT);
	connect(sockfd, &serveraddr, sizeof(serveraddr));
	char buffer[256] = {0,0,0,0,0,0,0,0,0,0,0,0,0,'A',0,0,0,0,0,0,'a','a','a',0};


	
	send(sockfd, buffer, 256, 0);
	return 0;


}
