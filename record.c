#include "record.h"
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

int printf_record(struct record * record) {
	printf("id: %d\n", record->id);
	printf("time_to_do: %ld\n", record->time_to_do);
	printf("start_time: %ld\n", record->start_time);
	printf("task: %s\n", record->task);
	printf("finish: %ld\n", record->finish);
	printf("priority: %d\n\n", record->priority);
	return 0;
}


int write_record_filename(char * filename, struct record *record) {
	FILE *fptr = fopen(filename, "wb"); 
	if (!fptr) {
		perror("fileopen");
		return -1;
	}
	if (fwrite(&record->id, sizeof(record->id), 1, fptr) != 1) {
		printf("error writing id");
	}

	if (fwrite(&record->start_time, sizeof(record->start_time), 1, fptr) != 1) {
		printf("error writing finish time");
	}

	if (fwrite(&record->finish, sizeof(record->finish), 1, fptr) != 1) {
		printf("error writing finish time");
	}

	if (fwrite(&record->priority, sizeof(record->priority), 1, fptr) != 1) {
		printf("error writing priority");
	}

	if (fwrite(&record->time_to_do, sizeof(record->time_to_do), 1, fptr) != 1) {
		printf("error writing time to do");
	}
	int i=0;
	while (record->task[i] != '\0' && i<MAX_TASK_SIZE) {
		if (fwrite(&(record->task[i]), sizeof(char), 1, fptr) != 1) {
			printf("error writing task");
		}

		i++;
	}
	fclose(fptr);
	return 0;
}

int read_record_file_to_ptr(char *filename, struct record* record) {
	FILE *fptr = fopen(filename, "rb");
	
	if (fptr == NULL) {
		perror("cant open file: ");
		return -1;
	}
	char buffer[MAX_RECORD_SIZE] = {0};
	fread(buffer, MAX_RECORD_SIZE, sizeof(char), fptr);

	return read_record_buf_to_ptr(buffer, record);
}

int read_record_buf_to_ptr(char *src_buffer, struct record* record) {
	int i=0;
	memcpy(&(record->id), src_buffer+i, sizeof(record->id));
	i+=sizeof(record->id);
	memcpy(&(record->start_time), src_buffer+i, sizeof(record->start_time));
	i+=sizeof(record->start_time);
	memcpy(&(record->finish), src_buffer+i, sizeof(record->finish));
	i+=sizeof(record->finish);
	memcpy(&(record->priority), src_buffer+i, sizeof(record->priority));
	i+=sizeof(record->priority);
	memcpy(&(record->time_to_do), src_buffer+i, sizeof(record->time_to_do));
	i+=sizeof(record->time_to_do);

	char *task_buffer = (char*) malloc(sizeof(char)*MAX_TASK_SIZE);
	bzero(task_buffer, sizeof(char)*MAX_TASK_SIZE);
	task_buffer[0]=src_buffer[i];
	int j=0;
	// MAX_TASK_SIZE-1 to guarantee null termination
	for(j=0;j<MAX_TASK_SIZE-1 && src_buffer[i+j] != '\0';j++) {
		task_buffer[j] = src_buffer[i+j];
	}
	i+=j;
	record->task = task_buffer;
	return i-1;
}
int write_record_to_buffer(struct record* record, char *dest_buffer) {
	int i=0;
	memcpy(dest_buffer+i, &(record->id), sizeof(record->id));
	i += sizeof(record->id);
	memcpy(dest_buffer+i, &(record->start_time), sizeof(record->start_time));
	i += sizeof(record->start_time);
	memcpy(dest_buffer+i, &(record->finish), sizeof(record->finish));
	i += sizeof(record->finish);
	memcpy(dest_buffer+i, &(record->priority), sizeof(record->priority));
	i += sizeof(record->priority);
	memcpy(dest_buffer+i, &(record->time_to_do), sizeof(record->time_to_do));
	i += sizeof(record->time_to_do);

	strncpy(dest_buffer+i, record->task, MAX_TASK_SIZE);
	(dest_buffer+i)[MAX_TASK_SIZE-1] = '\0';
	return i+strlen(dest_buffer+i);
}

struct record *get_all(int max_records) {
	DIR *dir;
	struct record* records = malloc(sizeof(struct record) * max_records);

	struct record invalid_record;
	invalid_record.id = 0;
	invalid_record.start_time = 0;
	invalid_record.finish = 0;
	invalid_record.priority = INT_MIN;
	invalid_record.time_to_do = 0;
	invalid_record.task = 0;

	for(int i=0;i<max_records;i++) {
		records[i] = invalid_record;
	}

	struct dirent *ent;
	char buffer[MAX_FILENAME_SIZE + 128] = {0};
	// starting from 1 becouse id = 0 breaks client side and i cant fix it
	int i=1;
	int true_records = 0;
	snprintf(buffer, MAX_FILENAME_SIZE+128, "%srecord_%d",DB_ROOT, i);
	while (true_records < max_records) { 
		if (access(buffer, F_OK) == 0) {
			read_record_file_to_ptr(buffer, records+true_records);
			records[true_records].id = i;
			true_records++;

		}
		i++;
		snprintf(buffer, MAX_FILENAME_SIZE +128, "%srecord_%d",DB_ROOT, i);
	}
	return records;
}
