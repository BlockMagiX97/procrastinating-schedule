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
		free(arg);
		return NULL;
	}
	
	int32_t total_struct = generate_mask_from_client(client_fd, mask);

	printf("total_struct: %d\n", total_struct);
	for (int i=0;i<mask->num_of_structs;i++) {
		printf("%s\n", global_format.struct_info[i].identifier);
		for (int j=0;j<mask->struct_mask[i].num_of_fields;j++) {
			printf("    %s: %d\n", global_format.struct_info[i].field_info[j].identifier, mask->struct_mask[i].field_mask[j]);
		}
	}
	if (send_data(client_fd,&total_struct,4) != 4) {
		perror("error sending num_of_structs");
		free(arg);
		free_mask(mask);
		return NULL;
	}
	if (total_struct < 0) {
		printf("generating mask failed: %d\n", total_struct);
		free(arg);
		free_mask(mask);
		return NULL;
	}
	for (int i=0;i<)
	
	free_mask(mask);
	free(arg);

	return NULL;
}


int main(int argc, char** argv) {
	generate_global_format();
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
