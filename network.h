#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>


typedef struct {
	uint32_t data_len;
	uint8_t* data;
	uint32_t offset;
} packet;

packet* read_packet(int fd);

// 0 means success
int write_packet(int fd, packet* pack);

void free_packet(packet* pack);
