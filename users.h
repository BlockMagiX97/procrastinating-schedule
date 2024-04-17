#include <argon2.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "network.h"

#ifndef USER_ENTRYS
#define USER_ENTRYS
typedef struct {
	uint32_t memory;
	uint32_t num_iter;
	uint32_t pararell;
	uint32_t version;

	uint8_t username_len;
	uint8_t salt_len;
	uint8_t hash_len;
	// there is data here of size hash_len + username_len + salt_len
	// enteries are placed in this order:
	// username
	// salt
	// hash
} __attribute__((packed)) user_entry_network;

typedef struct {
	uint32_t memory;
	uint32_t num_iter;
	uint32_t pararell;
	uint32_t version;

	uint8_t username_len;
	char* username;
	uint8_t salt_len;
	uint8_t* salt;
	uint8_t hash_len;
	uint8_t* hash;

} user_entry;
#endif

int write_user(packet* pack, user_entry* src);
int write_user_to_fd(int fd, user_entry* src);
int read_user(packet* pack, user_entry* dest);
int read_user_from_fd(int fd, user_entry* dest);

int free_user(user_entry* user);
int print_user(const user_entry* user);
