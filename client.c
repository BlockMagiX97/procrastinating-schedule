#include <unistd.h>
#include <argon2.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "protocol.h"
#define PORT 9991

int main(int argc, char**argv) {
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = INADDR_ANY;
	serveraddr.sin_port = htons(PORT);
	connect(sockfd, &serveraddr, sizeof(serveraddr));


	generate_global_format();
	if (send_format_to_server(sockfd) < 0) {
		printf("FUCKKKKK\n");
	}


	close(sockfd);
	
	return 0;

	
}
