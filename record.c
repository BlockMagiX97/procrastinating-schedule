#include "record.h"

int read_record_ptr_to_ptr(record_network* src, record* dest, char* task) {
	dest->id = src->id;
	dest->start_time = src->start_time;
	dest->time_to_do = src->time_to_do;
	dest->finish_time = src->finish_time;
	dest->priority = src->priority;
	dest->task_lenght = src->task_lenght;
	
	dest->task = task;

	return 0;
}

int write_record_ptr_to_ptr(record* src, record_network* dest) {
	dest->id = src->id;
	dest->start_time = src->start_time;
	dest->time_to_do = src->time_to_do;
	dest->finish_time = src->finish_time;
	dest->priority = src->priority;

	dest->task_lenght = src->task_lenght;
	memcpy(((uint8_t*)(&(dest->task_lenght)))+sizeof(dest->task_lenght), src->task, src->task_lenght);

	return 0;
}

int write_record_to_fd(record* src, int fd) {
	size_t task_lenght = src->task_lenght;
	size_t total_record_size = sizeof(record_network)+task_lenght;
	uint8_t* record_buffer = (uint8_t*) malloc(total_record_size);
	// warning is expected since func is accessing task_lenght bytes beyond
	write_record_ptr_to_ptr(src, (record_network*)record_buffer);

	
	if (write(fd, record_buffer, total_record_size) != total_record_size) {
		return -1;
	};

	free(record_buffer);
	return 0;
}
int read_record_from_fd(record* dest, int fd) {
	record_network in_record;;
	if (read(fd, &in_record, sizeof(record_network)) != sizeof(record_network)) {
		perror("read failed: ");
		return -1;
	}
	char* task_buffer = (char*)malloc(in_record.task_lenght+1);
	if (read(fd, task_buffer, in_record.task_lenght) != in_record.task_lenght) {
		perror("read failed: ");
		return -1;
	}
	
	task_buffer[in_record.task_lenght] = '\0';

	read_record_ptr_to_ptr(&in_record, dest, task_buffer);
	return 0;

}
int print_record(const record* dest) {
	printf("id: %u\n", dest->id);
	printf("start_time: %ld\n", dest->start_time);
	printf("time_to_do: %ld\n", dest->time_to_do);
	printf("finish_time: %ld\n", dest->finish_time);
	printf("priority: %hu\n", dest->priority);
	printf("task: %s\n", dest->task);
	return 0;
}
