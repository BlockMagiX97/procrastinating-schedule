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
#include <limits.h>
#include <sys/fcntl.h>
#include "../record.h"		
#ifndef OP_CODES_TABLE
#define OP_CODES_TABLE {&new_record, &get_all_sorted}
#endif
#ifndef OP_CODES_TABLE_LEN
#define OP_CODES_TABLE_LEN 2
#endif
#define PORT 4242
#define DB_ROOT "/var/lib/procrast_data/" // NEEDS to have "/" at the end
#define ENOUGH ((CHAR_BIT * sizeof(uint16_t) - 1) / 3 + 2) // MUST be of the same type as record.task_lenght 

typedef int (*function_oper)(int, uint8_t); 

int sort_records(const void* a, const void* b) {
	record* e1 = (record*)a;
	record* e2 = (record*)b;
	if (e1->finish_time > e2->finish_time) return 1;
	if (e1->finish_time < e2->finish_time) return -1;
	if (e1->priority > e2->priority) return 1;
	if (e1->priority < e2->priority) return -1;
	return 0;
}
int get_all_sorted(int client_fd, uint8_t request_lenght) {
	uint32_t file_count=0;
	size_t db_len = strlen(DB_ROOT);

	struct dirent *de;  // Pointer for directory entry 
  
	DIR *dr = opendir(DB_ROOT); 
  
	if (dr == NULL) { 
		printf("Could not open current directory" ); 
		return 0; 
	} 
	while ((de = readdir(dr)) != NULL) {
		if (de->d_type == DT_REG) {
			file_count++;
		}
	}
	closedir(dr);

	record* records = (record*)malloc(sizeof(record)*file_count);

	dr = opendir(DB_ROOT); 
  
	if (dr == NULL) { 
		printf("Could not open current directory" ); 
		return 0; 
	}
	int filefd;
	int i=0;
	uint64_t total_size = 0;
	while ((de = readdir(dr)) != NULL) {
		if (de->d_type == DT_REG) {
			filefd = openat(dirfd(dr), de->d_name, O_RDONLY);
			if (filefd == -1) {
				perror("file at get_all_sorted");
				return -1;
			}
			read_record_from_fd(records+i, filefd);
			records[i].id = atoi(de->d_name);
			total_size += sizeof(record_network)+records[i].task_lenght;
			close(filefd);
			i++;
			
		}
	}
	closedir(dr);

	qsort(records, file_count, sizeof(record), &sort_records);

	write(client_fd, &file_count, sizeof(typeof(file_count)));

	time_t finish;
	time_t start;
	start = records[file_count-1].finish_time;
	finish = records[file_count-1].finish_time;
	for (int j=file_count-1; j>=0; j--) {
		if(finish != records[j].finish_time) {
			start = records[j].finish_time;	
			finish = records[j].finish_time;
		}
		start -= records[j].time_to_do;
		records[j].start_time = start;
	}
	

	for (int j=0;j<file_count;j++) {
		write_record_to_fd(records+j, client_fd);
	}

	return 0; 
}
// should be free of memory leaks
// request_lenght is not used
int new_record(int client_fd, uint8_t request_lenght){
	record tmp_rec;
	read_record_from_fd(&tmp_rec, client_fd);
	print_record(&tmp_rec);
	uint32_t i=0;
	size_t db_len = strlen(DB_ROOT);
	char* buffer = (char*)malloc(db_len+ENOUGH+1);

	do {
		snprintf(buffer,db_len+ENOUGH, "%s%d",DB_ROOT, i);
		buffer[db_len+ENOUGH] = '\0'; 
		i++;
		printf("here\n");
		
	} while (access(buffer, F_OK) == 0);

	int record_filefd = open(buffer, O_WRONLY | O_CREAT, 0666);
	free(buffer);

	write_record_to_fd(&tmp_rec, record_filefd);
	free(tmp_rec.task);

	close(record_filefd);
	return 0;
}

void* handle_client(void* arg) {
	int client_fd = *((int*)arg);

	uint8_t first_packet[2];	// two chars first is op_code second is lenght of next packet
	if (recv(client_fd, first_packet, 2, 0) != 2) {
		printf("client didnt send correct packet\n");
		goto return_point;
	}

	uint8_t op_code = first_packet[0];
	if(op_code >= OP_CODES_TABLE_LEN) {
		goto return_point;
	}
	uint8_t request_lenght = first_packet[1];

	function_oper op_codes[OP_CODES_TABLE_LEN] = OP_CODES_TABLE;
	int result = op_codes[op_code](client_fd, request_lenght);

	printf("operation: %u, result: %d\n", op_code, result);

return_point:
	printf("client served\n");
	close(client_fd);
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
