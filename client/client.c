#include "../record.h"
#include "../users.h"
#include <argon2.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define PORT 4242

void print_usage() {
	printf("usage\n");
}
int remove_chars(char* str, int strlen_s, char c) {
	int i=0;
	int j=0;
	while ((i<strlen_s) && (str[i] != '\0')) {
		if (str[i] != c) {
			str[j] = str[i];
			j++;
		}
		i++;
	}
	str[j] = str[i];
	return j;
}
int create_new_user_client(int sockfd, char* username, char* passwd) {
	uint8_t is_new_user  = 0;
	send(sockfd, &is_new_user, 1, 0);


	user_entry info;
	read_user_from_fd(sockfd, &info);
	uint8_t* hash = (uint8_t*)malloc(info.hash_len);
	argon2_context context = {
		hash,
		info.hash_len,
		passwd,
		strlen(passwd),
		info.salt,
		info.salt_len,
		NULL, 0,
		NULL, 0,
		info.num_iter,
		info.memory,
		info.pararell,
		info.pararell,
			info.version,
		NULL,NULL,
		ARGON2_DEFAULT_FLAGS
	};
	int rc = argon2id_ctx(&context);
	if (rc != ARGON2_OK) {
		printf("Error: %s\n", argon2_error_message(rc));
		return 1;
	}
	info.hash = hash;
	info.username = username;
	info.username_len = strlen(username);
	write_user_to_fd(sockfd, &info);
	return 0;
}
int authenticate_user_client(int sockfd, char* username, char* passwd) {
	uint8_t is_new_user = 1;
	send(sockfd, &is_new_user, 1, 0);


	uint8_t username_len = strlen(username);
	write(sockfd, &username_len, 1);
	write(sockfd, username, username_len);

	user_entry info;
	read_user_from_fd(sockfd, &info);
	print_user(&info);
	uint8_t* hash = (uint8_t*)malloc(info.hash_len);
	argon2_context context = {
		hash,
		info.hash_len,
		passwd,
		strlen(passwd),
		info.salt,
		info.salt_len,
		NULL, 0,
		NULL, 0,
		info.num_iter,
		info.memory,
		info.pararell,
		info.pararell,
		info.version,
		NULL,NULL,
		ARGON2_DEFAULT_FLAGS
	};
	int rc = argon2id_ctx(&context);
	if (rc != ARGON2_OK) {
		printf("Error: %s\n", argon2_error_message(rc));
		return 1;
	}
	info.hash = hash;
	write_user_to_fd(sockfd, &info);

	uint8_t op_ret = 1;
	recv(sockfd, &op_ret, 1, 0);

	return op_ret;
}
int new_record_client(int sockfd, const record* new_record) {
	uint8_t op_code = 0;
	send(sockfd, &op_code, 1,0);
	write_record_to_fd(new_record, sockfd);

	uint8_t op_ret = 1;
	recv(sockfd, &op_ret, 1, 0);

	return op_ret;
}
record* get_all_rec_client(int sockfd, uint32_t* record_count) {
	uint8_t op_code = 1;
	send(sockfd, &op_code, 1, 0);

	read(sockfd, record_count, sizeof(record_count));

	printf("rec_num: %u\n", *record_count);

	record* records = malloc((*record_count)*sizeof(record));
	for (int i=0; i<(*record_count);i++) {
		read_record_from_fd(records+i, sockfd);
		printf("exec\n");
	}
	
	return records;
}

int main(int argc, char**argv) {
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = INADDR_ANY;
	serveraddr.sin_port = htons(PORT);
	connect(sockfd, &serveraddr, sizeof(serveraddr));

	char new_user_buff[10];
	printf("New user (y, n): ");
	fgets(new_user_buff, 10, stdin);
	uint8_t new_user = new_user_buff[0] == 'y' ? 0 : 1; 

	char username[256];
	printf("Username: ");
	fgets(username, 256, stdin);

	char passwd[256];
	printf("Password: ");
	fgets(passwd, 256, stdin);

	remove_chars(username, strlen(username), '\n'); 
	remove_chars(passwd, strlen(passwd), '\n'); 

	if (new_user == 0) { 
		create_new_user_client(sockfd, username, passwd);
	
	} else {
		authenticate_user_client(sockfd, username, passwd);
	}

	char operation[25];
	printf("operation (n, d, g): ");
	fgets(operation, 25, stdin);

	if (operation[0] == 'n') {
		record tmp_rec;

		char task[256];
		printf("task: ");
		fgets(task, 256, stdin);
		tmp_rec.task = task;

		char priority[25];
		printf("priority: ");
		fgets(priority, 25, stdin);
		tmp_rec.priority = atoi(priority);

		char finish_time[101];
		printf("finish time  : ");
		fgets(finish_time, 101, stdin);
		tmp_rec.finish_time = atoi(finish_time);
		
		char time_to_do[101];
		printf("time to do: ");
		fgets(time_to_do, 101, stdin);
		tmp_rec.time_to_do = atoi(time_to_do);

		tmp_rec.task_lenght = strlen(task);
		tmp_rec.id = 0;
		tmp_rec.start_time = 0;

		new_record_client(sockfd, &tmp_rec);

	} else if (operation[0] == 'g') {
		uint32_t record_count;
		record* records = get_all_rec_client(sockfd, &record_count);
				for (int i=0;i<record_count;i++) {
			print_record(records+i);
			printf("\n");
		}
	}else if (operation[0] == 'd') {
		uint8_t op_code = 2;
		send(sockfd, &op_code, 1, 0);
		char buffer[100];
		fgets(buffer, 100, stdin);
		uint32_t id = atoi(buffer);
		send(sockfd, &id, sizeof(uint32_t), 0);
		
	}
	

	close(sockfd);
	
	return 0;

	
}
