#include "record.h"

int free_record(record* rec) {
	free(rec->task);
	free(rec);
	return 0;
}
int read_record(packet* pack, record* dest) {
	if ((pack->data_len - pack->offset) < sizeof(record_network)) {
		return 1;
	}
	record_network* tmp = (record_network*)(pack->data+pack->offset);
	dest->id = tmp->id;
	dest->finish_time = tmp->finish_time;
	dest->time_to_do = tmp->time_to_do;
	dest->start_time = tmp->start_time;
	dest->priority = tmp->priority;
	dest->task_lenght = tmp->task_lenght;

	pack->offset += sizeof(record_network);

	if ((pack->data_len - pack->offset) < dest->task_lenght) {
		pack->offset -= tmp->task_lenght;
		return 1;
	}
	char* buffer = (char*)malloc(dest->task_lenght+1);
	memcpy(buffer, pack->data+pack->offset, dest->task_lenght);
	buffer[dest->task_lenght] = '\0';
	pack->offset += dest->task_lenght;
	dest->task = buffer;
	return 0;
}
int write_record(packet* pack, record* src) {
	if ((pack->data_len-pack->offset) < (sizeof(record_network)+src->task_lenght)) {
		return 1;
	}
	record_network* dest = (record_network*)(pack->data+pack->offset);
	dest->id = src->id;
	dest->finish_time = src->finish_time;
	dest->time_to_do = src->time_to_do;
	dest->start_time = src->start_time;
	dest->priority = src->priority;
	dest->task_lenght = src->task_lenght;
	pack->offset += sizeof(record_network);
	// buffer overflow checked at the top of the function
	memcpy(pack->data+pack->offset, src->task, dest->task_lenght);
	pack->offset += dest->task_lenght;
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
