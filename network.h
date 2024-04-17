#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>


#ifndef PACKET
#define PACKET
typedef struct {
	uint32_t data_len;
	uint8_t* data;
	uint32_t offset;
} packet;
#endif

packet* read_packet(int fd);

// 0 means success
int write_packet(int fd, packet* pack);

int read_bytes_packet(packet* pack, void* dest, size_t n);

packet* create_packet(uint32_t data_len);

void free_packet(packet* pack);
