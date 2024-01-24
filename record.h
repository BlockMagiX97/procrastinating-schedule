#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

typedef struct {
	uint32_t id;
	time_t start_time;
	time_t time_to_do;
	time_t finish_time;
	uint16_t priority;
	uint16_t task_lenght; // for easier buffer size prediction
	char* task; // MUST include null byte
} record;

// basicaly we first define static lenght and then we add dynamic elements to memory directly after last element 
typedef struct {
	uint32_t id;
	time_t start_time;
	time_t time_to_do;
	time_t finish_time;
	uint16_t priority;
	uint16_t task_lenght;
	// task is located here and is task_lenght bytes long !!!null byte not included!!!
} __attribute__((packed)) record_network;

typedef struct {
	uint8_t opcode;
} __attribute__((packed)) packet_request;

typedef struct {
	uint8_t result;
	uint32_t lenght_payload;
} __attribute__((packed)) packet_response;

int print_record(const record* dest);
int read_record_ptr_to_ptr(record_network* src, record* dest, char* task);
int read_record_from_fd(record* dest, int fd);
int write_record_ptr_to_ptr(record* src, record_network* dest);
int write_record_to_fd(record* src, int fd);

