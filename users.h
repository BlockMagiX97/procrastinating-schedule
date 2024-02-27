#include <argon2.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

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

int write_user_to_fd(int fd, user_entry* user);

// INTERNAL USE ONLY
int write_user_ptr_ptr(user_entry* src, user_entry_network* dest);
// INTERNAL USE ONLY
int read_user_ptr_ptr(user_entry_network* src, uint8_t* dynamic_src, user_entry* dest);

int read_user_from_fd(int fd, user_entry* dest);
int print_user(const user_entry* user);
