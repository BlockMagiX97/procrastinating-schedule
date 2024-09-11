#include "protocol.h"
#include <stdio.h>
#include <argon2.h>
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h> // mkdirat
// Server config
#define PORT 9991


void* handle_client(void* arg) {
	int client_fd = *((int*)arg);

	struct format_mask_t* mask = malloc_mask();
	if (mask == NULL) {
		perror("mask");
		return NULL;
	}
	
	int32_t total_struct = generate_mask_from_client(client_fd, mask);
	if (total_struct < 0) {
		printf("fuckers\n");
		return NULL;
	}
	

	return NULL;
}


int main(int argc, char** argv) {
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		perror("socket");
		return -1;
	}

	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);
	int address_len = sizeof(address);
	if (bind(sockfd, (struct sockaddr*) &address, address_len) == -1) {
		perror("bind");
		return -1;
	}  

	if (listen(sockfd, 3) == -1) {
		perror("listen");
		return -1;
	}

	while(1) {
		struct sockaddr_in client_addr;
		int cilent_addr_len = sizeof(client_addr);
		int* cilent_fd = malloc(sizeof(int));
		if ((*cilent_fd = accept(sockfd, &client_addr, &cilent_addr_len)) == -1 ) {
			perror("accept");
			return -1;
		}
	
		pthread_t thread_id;
		pthread_create(&thread_id, NULL, handle_client, (int*)cilent_fd);
		pthread_detach(thread_id);
	}
}
