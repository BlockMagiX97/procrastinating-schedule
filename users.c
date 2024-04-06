#include "users.h"

int free_user(user_entry* user) {
	free(user->hash);
	free(user->salt);
	free(user->username);
	return 0;
};

int print_user(const user_entry* user) {
	printf("memory: %u\n num_iter: %u\npararell: %u\nversion: %u\nusername(%d): %s\nsalt_len: %d\nhash_len: %d\n\n", user->memory, user->num_iter, user->pararell, user->version, user->username_len, user->username, user->salt_len, user->hash_len);
	return 0;
}

int write_user(packet* pack, user_entry* src) {
	if ((pack->data_len-pack->offset) < (sizeof(user_entry_network)+src->hash_len+src->salt_len+src->username_len)) {
		return 1;
	}
	user_entry_network* dest = (user_entry_network*)(pack->data+pack->offset);
	dest->memory = src->memory;
	dest->num_iter = src->num_iter;
	dest->pararell = src->pararell;
	dest->version = src->version;
	dest->username_len = src->username_len;
	dest->salt_len = src->salt_len;
	dest->hash_len = src->hash_len;

	pack->offset += sizeof(user_entry_network);

	if (src->username != 0) {
		memcpy(pack->data+pack->offset, src->username, src->username_len);
		pack->offset+=src->username_len;
	}
	if (src->salt != 0) {
		memcpy(pack->data+pack->offset, src->salt, src->salt_len);
		pack->offset+=src->salt_len;
	}
	if (src->hash != 0) {
		memcpy(pack->data+pack->offset, src->hash, src->hash_len);
		pack->offset += src->hash_len;
	}

	return 0;

}
int read_user(packet* pack, user_entry* dest) {
	if ((pack->data_len-pack->offset) < sizeof(user_entry_network)) {
		return 1;
	} 
	user_entry_network* src = (user_entry_network*)(pack->data+pack->offset);
	dest->memory = src->memory;
	dest->num_iter = src->num_iter;
	dest->pararell = src->pararell;
	dest->version = src->version;
	dest->username_len = src->username_len;
	dest->salt_len = src->salt_len;
	dest->hash_len = src->hash_len;

	pack->offset += sizeof(user_entry_network);

	if ((pack->data_len-pack->offset) < (dest->username_len+dest->salt_len+dest->hash_len)) {
		return 1;
	}
	uint8_t* username = (uint8_t*)malloc(src->username_len+1);
	memcpy(username, pack->data+pack->offset, src->username_len);
	username[src->username_len] = '\0';
	pack->offset += src->username_len;

	uint8_t* salt = (uint8_t*)malloc(src->salt_len);
	memcpy(salt, pack->data+pack->offset, src->salt_len);
	pack->offset += src->salt_len;

	uint8_t* hash = (uint8_t*)malloc(src->hash_len);
	memcpy(hash, pack->data+pack->offset, src->hash_len);
	pack->offset  += src->hash_len;

	dest->username = (char*)username;
	dest->salt = salt;
	dest->hash = hash;

	return 0;
}
