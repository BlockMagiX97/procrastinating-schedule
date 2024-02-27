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
int main(int argc, char**argv) {
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = INADDR_ANY;
	serveraddr.sin_port = htons(PORT);
	connect(sockfd, &serveraddr, sizeof(serveraddr));

	uint8_t new_user = argc-1;
	send(sockfd, &new_user, 1, 0);

	if (new_user == 0) {
		user_entry info;
		char passwd[256];
		char username[256];
		printf("Enter username: ");
		scanf("%255s", username);
		printf("Enter password: ");
		scanf("%255s", passwd);
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
			exit(1);
		}
		info.hash = hash;
		info.username = username;
		info.username_len = strlen(username);
		write_user_to_fd(sockfd, &info);
		int file = open("user_debug", O_RDWR | O_CREAT, 0666);
		write_user_to_fd(file, &info);
		close(file);

	} else {
		char passwd[256];
		char username[256];
		printf("Enter username: ");
		scanf("%255s", username);
		printf("Enter password: ");
		scanf("%255s", passwd);

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
			exit(1);
		}
		info.hash = hash;

		write_user_to_fd(sockfd, &info);
	}


	close(sockfd);
	
	return 0;

	
}
