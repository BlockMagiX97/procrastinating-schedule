#include "../record.h"
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define PORT 8081

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
	int opcode;
	int msg_len;
	char buffer[MAX_RECORD_SIZE+1];


	if (argc < 2) {
		print_usage();
		return -1;
	}
	if (argv[1][0] == 'd') 
		opcode = 2;
	else if (argv[1][0] == 'g')
		opcode = 1;
	else if (argv[1][0] == 'n') {
		opcode = 0;
	} else {
		print_usage();
		return -1;
	}
	

	if (opcode == 2) {
		if (argc < 3) {
			print_usage();
			return -1;
		}

		msg_len = sizeof(char)+sizeof(int);
		*((int*)(buffer+1)) = atoi(argv[2]);
	}
	if (opcode == 1) {
		msg_len = sizeof(char);
	}
	if (opcode == 0) {
		if (argc < 6) {
			print_usage();
			return -1;
		}
		struct record record;
		record.id = 0;
		record.start_time = 0;
		record.finish = atoi(argv[2]);
		record.time_to_do = atoi(argv[3]);
		record.priority = atoi(argv[4]);
		record.task = argv[5];
		write_record_to_buffer(&record, buffer+1); 
		msg_len = MAX_RECORD_SIZE + sizeof(char);


		
	}

	buffer[0] = opcode;
	send(sockfd, buffer, msg_len, 0);

	int lenght;
	recv(sockfd, &lenght, sizeof(int), 0);
	char *response = malloc(lenght);
	recv(sockfd, response, lenght, 0);

	if(opcode == 2) {
		int exit_code = *((int*)(response));
		if (exit_code == 0) {
			printf("SUCCESS\n");
			return 0;
		}
		printf("ERROR\n");
		return -1;
	}

	if (opcode == 1) {
		int num_records = *((int*)(response));
		struct record *records = malloc(sizeof(struct record) * num_records);
		int offset=0;

		for (int i=0;i<num_records;i++) {
			offset += read_record_buf_to_ptr(response+offset+sizeof(int), records+i);
		}
		for (int i=0;i<num_records;i++) {
			printf_record(records+i);
		}

	}
	if(opcode == 0) {
		int exit_code = *((int*)(response));
		if (exit_code == 0) {
			printf("SUCCESS\n");
			return 0;
		}
		printf("ERROR\n");
		return -1;
	}

	return 0;
}
