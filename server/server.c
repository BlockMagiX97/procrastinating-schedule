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
#include <sys/fcntl.h> // mkdirat
#include "../record.h"		
#include "../users.h"
#include "../network.h"
// Server config
#define PORT 9991
#define DB_ROOT "/var/lib/procrast_data/"

// Cryptografy config
#define MEMORY (1<<16)
#define NUM_ITER 2
#define PARARELL 1
#define VERSION ARGON2_VERSION_13
#define SALT_LEN 64
#define HASH_LEN 128


void* handle_client(void* arg) {
	int client_fd = *((int*)arg);

	packet* auth_packet = read_packet(client_fd);
	// authentication
	uint8_t new_user;
	read_bytes_packet(auth_packet, &new_user, 1);

	if (new_user == 0) {
		// create new user


		user_entry response;
		read_user(auth_packet, &response);

		int root_dir = open(DB_ROOT,O_DIRECTORY);
		print_user(&response);
		if (root_dir < 0) {
			perror("db_root not open");
			goto end_point;
		}
		if (mkdirat(root_dir, response.username, 0700) < 0) {
			perror("cannot make dir");
			close(root_dir);
			goto end_point;
		}
		int user_root = openat(root_dir, response.username, O_DIRECTORY);
		if (user_root < 0) {
			perror("user_root not open");
			close(root_dir);
			goto end_point;
		}
		int user_file = openat(user_root, "info", O_CREAT | O_RDWR, 0666);
		if (user_file < 0) {
			perror("user_info can not open");
			close(root_dir);
			close(user_root);
			goto end_point;
		}
		write_user_to_fd(user_file, &response);
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
