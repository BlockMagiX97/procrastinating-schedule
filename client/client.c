#include "../pre.c"
#include <unistd.h>
#include <argon2.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define PORT 9991

int main(int argc, char**argv) {
	struct sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = INADDR_ANY;
	serveraddr.sin_port = htons(PORT);

	struct record_t a;
	a.owner = 10;
	a.permission = 11;
	a.group = 2;
	a.task_len = 3;
	a.task = "max";

	struct record_t b;

	uint8_t * buffer = malloc(size_record_t(&a));
	serialize_record_t(buffer, &a, size_record_t(&a));
	deserialize_record_t(&b, buffer, size_record_t(&a));

	printf("ser_size = %lu\n", size_record_t(&a));
	printf("owner: %u\n", b.owner);
	printf("permisions: %u\n", b.permission);
	printf("group: %u\n", b.group);
	printf("task[%u]: %s\n", b.task_len, b.task);


	
	return 0;

	
}
