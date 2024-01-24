#include "../record.h"
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define PORT 4242

void print_usage() {
	printf("usage\n");
}
int main(int argc, char**argv) {
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = INADDR_ANY;
	serveraddr.sin_port = htons(PORT);
	connect(sockfd, &serveraddr, sizeof(serveraddr));
	uint8_t first_packet[2];
	first_packet[0] = 1;
	first_packet[1] = 0;
	send(sockfd, first_packet, 2, 0);
	uint32_t num_records;
	recv(sockfd, &num_records, sizeof(uint32_t), 0);
	record* records = (record*)malloc(sizeof(record)*num_records);
	for (int i=0; i<num_records;i++) {
		read_record_from_fd(records+i, sockfd);
		print_record(records+i);
	}


	
	return 0;

	
}
