#include "users.h"

int write_user_ptr_ptr(user_entry* src, user_entry_network* dest) {
	dest->memory = src->memory;
	dest->num_iter = src->num_iter;
	dest->pararell = src->pararell;
	dest->version = src->version;
	dest->username_len = src->username_len;
	dest->salt_len = src->salt_len;
	dest->hash_len = src->hash_len;

	uint8_t* dest_end = ((uint8_t*)(dest))+sizeof(user_entry_network);
	memcpy(dest_end, src->username, src->username_len);
	dest_end+=src->username_len;
	memcpy(dest_end, src->salt, src->salt_len);
	dest_end+=src->salt_len;
	memcpy(dest_end, src->hash, src->hash_len);
	dest_end += src->hash_len;
	return 0;
	
}
int write_user_to_fd(int fd, user_entry* user) {
	size_t user_entry_net_len = sizeof(user_entry_network)+user->username_len+user->salt_len+user->hash_len;
	uint8_t* buffer = (uint8_t*)malloc(user_entry_net_len);
	write_user_ptr_ptr(user, (user_entry_network*)buffer);
	
	write(fd, buffer, user_entry_net_len);
	free(buffer);
	return 0;
	
}
int read_user_ptr_ptr(user_entry_network* src, uint8_t* dynamic_src, user_entry* dest) {
	dest->memory = src->memory;
	dest->num_iter = src->num_iter;
	dest->pararell = src->pararell;
	dest->version = src->version;
	dest->username_len = src->username_len;
	dest->salt_len = src->salt_len;
	dest->hash_len = src->hash_len;

	uint8_t* username = (uint8_t*)malloc(src->username_len+1);
	memcpy(username, dynamic_src, src->username_len);
	username[src->username_len] = '\0';
	dynamic_src += src->username_len;

	uint8_t* salt = (uint8_t*)malloc(src->salt_len);
	memcpy(salt, dynamic_src, src->salt_len);
	dynamic_src += src->salt_len;

	uint8_t* hash = (uint8_t*)malloc(src->hash_len);
	memcpy(hash, dynamic_src, src->hash_len);
	dynamic_src += src->hash_len;

	dest->username = (char*)username;
	dest->salt = salt;
	dest->hash = hash;


	return 0;
}
int read_user_from_fd(int fd, user_entry* dest) {
	user_entry_network in_user_entry;
	if(read(fd, &in_user_entry, sizeof(user_entry_network)) != sizeof(user_entry_network)) {
		return -1;
	}
	size_t dynamic_in_user_len = in_user_entry.username_len+in_user_entry.salt_len+in_user_entry.hash_len;
	uint8_t* dynamic_in_user_buffer = (uint8_t*)malloc(dynamic_in_user_len);
	if(read(fd, dynamic_in_user_buffer, dynamic_in_user_len) != dynamic_in_user_len ) {
		return -1;
	}
	read_user_ptr_ptr(&in_user_entry, dynamic_in_user_buffer, dest);
	return 0;

}
int print_user(const user_entry* user) {
	printf("memory: %u\n num_iter: %u\npararell: %u\nversion: %u\nusername(%d): %s\nsalt_len: %d\nhash_len: %d\n\n", user->memory, user->num_iter, user->pararell, user->version, user->username_len, user->username, user->salt_len, user->hash_len);
	return 0;
}
