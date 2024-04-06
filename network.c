#include "network.h"

packet* read_packet(int fd) {
	packet* pack = (packet*) malloc(sizeof(packet));
	if (pack == NULL) {
		perror("malloc");
		return NULL;
	}
	if (read(fd, &(pack->data_len),4) != 4) {
		perror("read_packet() data_len");
		free(pack);
		return NULL;
	}
	uint8_t* buffer = (uint8_t*)malloc(pack->data_len);
	if (buffer == NULL) {
		perror("malloc");
		free(pack);
		return NULL;
	}
	if (read(fd, buffer, pack->data_len) != pack->data_len) {
		perror("read_packet() data");
		free(buffer);
		free(pack);
		return NULL;
	}
	pack->data = buffer;
	pack->offset = 0;
	return pack;
}
// 0 means success
int write_packet(int fd, packet* pack) {
	if (write(fd, &(pack->data_len), 4) != 4) {
		perror("write_packet() data_len");
		return 1;
	}
	if (write(fd, pack->data, pack->data_len) != pack->data_len) {
		perror("write_packet() data");
		return 1;
	}
	return 0;
}
void free_packet(packet* pack) {
	free(pack->data);
	free(pack);
}
