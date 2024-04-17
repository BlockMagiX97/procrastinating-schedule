#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "network.h"

#ifndef RECORDS
#define RECORDS
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
#endif

int free_record(record* rec);
int print_record(const record* dest);
int read_record(packet*pack, record* dest);
int write_record(packet* pack, record* src);

