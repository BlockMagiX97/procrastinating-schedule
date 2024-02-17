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
#include "../record.h"		
#ifndef OP_CODES_TABLE
#define OP_CODES_TABLE {write_record_to_db, get_all_sorted, remove_record}
#endif
#ifndef OP_CODES_TABLE_LEN
#define OP_CODES_TABLE_LEN 3	// NEEDS TO BE UPDATED
#endif

int priority_comp(const void * elem1, const void * elem2) {
	int i = (*((struct record*)elem1)).priority;
	int j = (*((struct record*)elem2)).priority;
	if (i > j) return 1;
	if (i < j) return -1;
	return 0;
}

int time_comp(const void * elem1, const void * elem2) {
	time_t i = ((struct record*)elem1)->finish;
	time_t j = ((struct record*)elem2)->finish;
	if (i > j) return 1;
	if (i < j) return -1;
	return 0;
}
int remove_record(int clientfd, char *buffer) {
	int record_id = *((int*)(buffer+1));
	int result = 0;
	char filename[MAX_FILENAME_SIZE+128] = {0};
	snprintf(filename, MAX_FILENAME_SIZE+128 - 1, "%srecord_%d", DB_ROOT, record_id);
	if(remove(filename) != 0) {
		result = -1;
	}
	int tmp = sizeof(int);
	if (send(clientfd, &tmp, sizeof(int), 0) != sizeof(int)) {
		return 1;
	}
	if (send(clientfd, &result, sizeof(int), 0) != sizeof(int)) {
		result = 1;
	}
	return result;
	
	
}
// if returns -1 error happenned , 1 means error and client does not know it
int get_all_sorted(int clientfd, char *buffer) {
	int file_count = 0;
	DIR * dirp;
	struct dirent * entry;

	dirp = opendir(DB_ROOT); /* There should be error handling after this */
	if (dirp == NULL) {
		perror("cant open database: ");
		return -1;
	}
	while ((entry = readdir(dirp)) != NULL) {
		if (entry->d_type == DT_REG) { /* If the entry is a regular file */
			file_count++;
		}
	}
	closedir(dirp);
	struct record * records = get_all(file_count);

	qsort(records, file_count, sizeof(struct record), time_comp);

	int start=0;
	int i;
	time_t finish=0;
	for (i=0; i<file_count;i++) {
		if(finish != records[i].finish) {
			qsort(records+start, i - start, sizeof(struct record), priority_comp);
			// use finish as placeholder for current start_time
			for (int j=i-1; j >= start; j--) {
				// TODO: add checks for int overflows
				finish -= records[j].time_to_do;
				records[j].start_time = finish; 
			}

			finish = records[i].finish;
			start = i;
		}
		
	}
	qsort(records+start, i - start, sizeof(struct record), priority_comp);

	for (int j=i-1; j >= start; j--) {
		// TODO: add checks for int overflows
		finish -= records[j].time_to_do;
		records[j].start_time = finish; 
	}

	char *out_buffer = malloc(sizeof(int) + MAX_RECORD_SIZE * file_count);
	*((int*)(out_buffer)) = file_count;
	int offset = 0;
	
	for (int i=0;i<file_count;i++) {
		offset += write_record_to_buffer(records+i, out_buffer+sizeof(int)+offset);
	}
	int tmp = sizeof(int) + offset;
	if (send(clientfd, &tmp, sizeof(int), 0) != sizeof(int)) {
		return 1;
	}
	if (send(clientfd, out_buffer, tmp, 0) != tmp) {
		return 1;
	}
	return 0;
}
// if returns -1 error happenned , 1 means error and client does not know it
int write_record_to_db(int clientfd, char* buffer) {
	int result = 0;
	char* filename = (char*) malloc(sizeof(char) * (MAX_FILENAME_SIZE+strlen(DB_ROOT)));

	// records start at 1 becouse error happens when id == 0
	int i=1;
	snprintf(filename, MAX_FILENAME_SIZE, "%srecord_%d",DB_ROOT, i);
	while (access(filename, F_OK) == 0) {
		i++;
		snprintf(filename, MAX_FILENAME_SIZE, "%srecord_%d",DB_ROOT, i);
	}

	struct record record;
	read_record_buf_to_ptr(buffer+sizeof(char), &record);
	if (write_record_filename(filename, &record) == -1) {
		perror("write_record_to_db");
		result = -1;
		goto end_sequence;
	} else {
		result = record.id;
	}
end_sequence:

	int tmp= sizeof(int);
	if (send(clientfd, &tmp, sizeof(int), 0) != sizeof(int)) {
		return -1;
	}
	if (send(clientfd, &result, tmp, 0) != tmp) {
		result = 1;
	}
	return result;
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
	
	int (*op_code_table[OP_CODES_TABLE_LEN]) (int, char*)= OP_CODES_TABLE;
	int result = op_code_table[op_code](client_fd, request);
	printf("exit code of operation %d is %d\n", op_code, result);


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
