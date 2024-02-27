#include <stdio.h>
#include <argon2.h>
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
#include <errno.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "../record.h"		
#include "../users.h"
#ifndef OP_CODES_TABLE
#define OP_CODES_TABLE {&new_record, &get_all_sorted, &delete_record}
#endif
#ifndef OP_CODES_TABLE_LEN
#define OP_CODES_TABLE_LEN 3
#endif
#define PORT 4242
#define DB_ROOT "/var/lib/procrast_data/" // NEEDS to have "/" at the end
#define ENOUGH ((CHAR_BIT * sizeof(uint16_t) - 1) / 3 + 2) // MUST be of the same type as record.task_lenght 

typedef int (*function_oper)(int); 

// CRYPTOGRAFY
#define SALT_LEN 32
#define HASH_LEN 32

int sort_records(const void* a, const void* b) {
	record* e1 = (record*)a;
	record* e2 = (record*)b;
	if (e1->finish_time > e2->finish_time) return 1;
	if (e1->finish_time < e2->finish_time) return -1;
	if (e1->priority > e2->priority) return 1;
	if (e1->priority < e2->priority) return -1;
	return 0;
}
int delete_record(int client_fd) {
	size_t db_len = strlen(DB_ROOT);
	char* buffer = (char*)malloc(db_len+ENOUGH+1);

	uint32_t id;
	read(client_fd, &id, sizeof(id));
	snprintf(buffer, db_len+ENOUGH, "%s%d", DB_ROOT, id);
	return remove(buffer);
	

	
}
int get_all_sorted(int client_fd) {
	uint32_t file_count=0;

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
	for (int j=0;j<file_count;j++) {
		free(records[j].task);
	}
	free(records);

	return 0; 
}
// should be free of memory leaks
int new_record(int client_fd){
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
// TODO: fix memory leaks
void* handle_client(void* arg) {
	int client_fd = *((int*)arg);

	// authentication
	uint8_t new_user;
	read(client_fd, &new_user, sizeof(uint8_t));
	if (new_user == 0) {
		// create new user
		user_entry info;
		info.memory = (1<<16);
		info.num_iter = 2;
		info.pararell = 1;
		info.version = ARGON2_VERSION_13;

		info.salt_len = SALT_LEN;

		uint8_t salt_buffer[SALT_LEN];
		FILE* urandom = fopen("/dev/urandom","r");
		if (fread(salt_buffer, sizeof(uint8_t), SALT_LEN, urandom) != SALT_LEN) {
			perror("fread /dev/urandom");
			goto end_point;
		}
		fclose(urandom);
		info.salt = salt_buffer;

		info.hash_len = HASH_LEN;

		write_user_to_fd(client_fd, &info);

		user_entry response;
		read_user_from_fd(client_fd, &response);

		int root_dir = open(DB_ROOT,O_DIRECTORY);
		print_user(&response);
		if (root_dir < 0) {
			perror("db_root not open");
			goto end_point;
		}
		if (mkdirat(root_dir, response.username, 0700) < 0) {
			perror("cannot make dir");
			goto end_point;
		}
		int user_root = openat(root_dir, response.username, O_DIRECTORY);
		if (user_root < 0) {
			perror("user_root not open");
			goto end_point;
		}
		int user_file = openat(user_root, "info", O_CREAT | O_RDWR, 0666);
		if (user_file < 0) {
			perror("user_info can not open");
			goto end_point;
		}
		write_user_to_fd(user_file, &response);
		perror("write_user");
		close(root_dir);
		close(user_root);
		close(user_file);
	} else {
		// authenticate user
		
		uint8_t username_len;
		read(client_fd, &username_len, sizeof(uint8_t));
		char* username = (char*)malloc(username_len+1);
		read(client_fd, username, username_len);
		username[username_len] = '\0';

		int root_dir = open(DB_ROOT,O_DIRECTORY);
		if (root_dir < 0) {
			perror("db_root not open");
			goto end_point;
		}
		int user_root = openat(root_dir, username, O_DIRECTORY);
		if (user_root < 0) {
			perror("user_root not open");
			goto end_point;
		}
		int user_info_fd = openat(user_root, "info", O_RDONLY);
		if (user_info_fd < 0) {
			perror("user_info can not open");
			goto end_point;
		}

		user_entry info;
		read_user_from_fd(user_info_fd, &info);
		bzero(info.hash, info.hash_len);
		write_user_to_fd(client_fd, &info);

		user_entry response;
		read_user_from_fd(client_fd, &response);

		// refresh hash from storage
		close(user_info_fd);
		// refresh seek
		user_info_fd = openat(user_root, "info", O_RDONLY);
		read_user_from_fd(user_info_fd, &info);

		if (memcmp(info.hash, response.hash, info.hash_len) != 0) {
			printf("user %s failed authentication\n", username);
			goto end_point;
		}
		printf("success\n");
		free(username);



	}
	end_point:
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
