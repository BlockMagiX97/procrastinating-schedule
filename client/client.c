#include "../pre.c"
#include <unistd.h>
#include <argon2.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define PORT 9991

int main(int argc, char**argv) {
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = INADDR_ANY;
	serveraddr.sin_port = htons(PORT);
	connect(sockfd, &serveraddr, sizeof(serveraddr));

	struct record_t a;
	a.owner = 10;
	a.permission = 11;
	a.group = 2;
	a.task_len = 3;
	a.task = "max";

	write_record_t(sockfd, &a, 0);

	close(sockfd);
	
	return 0;

	
}
