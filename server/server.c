#include <stdio.h>
#include <dirent.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include "../record.h"		
#ifndef OP_CODES_TABLE
#define OP_CODES_TABLE {write_record_to_db}
#endif
#ifndef OP_CODES_TABLE_LEN
#define OP_CODES_TABLE_LEN 1	// NEEDS TO BE UPDATED
#endif


int write_record_to_db(char* buffer) {
	char* filename = (char*) malloc(sizeof(char) * (MAX_FILENAME_SIZE+strlen(DB_ROOT)));

	int i=0;
	snprintf(filename, MAX_FILENAME_SIZE, "%srecord_%d",DB_ROOT, i);
	while (access(filename, F_OK) == 0) {
		i++;
		snprintf(filename, MAX_FILENAME_SIZE, "%srecord_%d",DB_ROOT, i);
	}

	struct record record;
	read_record_ptr_to_ptr(buffer+sizeof(char), &record);
	if (write_record_filename(filename, &record) == -1) {
		perror("write_record_to_db");
		return -1;
	}
	return 0;
}

int priority_comp(const void * elem1, const void * elem2) {
	int i = (*((struct record*)elem1)).priority;
	int j = (*((struct record*)elem2)).priority;
	if (i > j) return 1;
	if (i < j) return -1;
	return 0;
}

int time_comp(const void * elem1, const void * elem2) {
	time_t i = (*((struct record*)elem1)).finish;;
	time_t j = (*((struct record*)elem2)).finish;
	if (i > j) return 1;
	if (i < j) return -1;
	return 0;
}

void* handle_client(void* arg) {
	int client_fd = *((int*)arg);
	int max_records = 16;
	char* request = (char *) malloc(256);	// assumes that char is 1 byte
	recv(client_fd, request, 256, 0);
	unsigned char op_code = request[0];
	if (op_code >= OP_CODES_TABLE_LEN) {
		printf("invalid op_code: %c\n", op_code);
		return 0;
	}
	
	int (*op_code_table[OP_CODES_TABLE_LEN]) (char*)= OP_CODES_TABLE;
	op_code_table[op_code](request);

	return 0;
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
