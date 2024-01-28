#include "record.h"
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

int write_record_filename(char * filename, struct record *record) {
	FILE *fptr = fopen(filename, "wb"); 
	if (!fptr) {
		perror("fileopen");
		return -1;
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
	
	if(fptr == NULL) {
		perror("fopen read_record_ptr");
		return -1;
	}

	if (fread(&record->finish, sizeof(record->finish), 1, fptr) != 1) {
		perror("fread finish");
		return -1;
	}

	if (fread(&record->priority, sizeof(record->priority), 1, fptr) != 1) { 
		perror("fread priority");
		return -1;
	}

	if (fread(&record->time_to_do, sizeof(record->time_to_do), 1, fptr) != 1) {
		perror("fread time_to_do");
		return -1;
	}

	char *buffer = (char *) malloc(sizeof(char) * MAX_TASK_SIZE);
	for (int i=0; i<MAX_TASK_SIZE;i++) {
		buffer[i] = 0;
	}
	int i=0;
	fread(buffer, sizeof(char), 1, fptr);
	while(i < MAX_TASK_SIZE && buffer[i] != 0) {
		i++;
		if (fread(buffer+i, sizeof(char), 1, fptr) != 1) {
			perror("fread task");
		}
	}
	record->task = buffer;
	return 0;
}
int read_record_ptr_to_ptr(char *buffer, struct record* record) {
	int i=0;
	memcpy(&(record->finish), buffer+i, sizeof(record->finish));
	i+=sizeof(record->finish);
	memcpy(&(record->priority), buffer+i, sizeof(record->priority));
	i+=sizeof(record->priority);
	memcpy(&(record->time_to_do), buffer+i, sizeof(record->time_to_do));
	i+=sizeof(record->time_to_do);

	char *task_buffer = (char*) malloc(sizeof(char)*MAX_TASK_SIZE);
	bzero(task_buffer, sizeof(char)*MAX_TASK_SIZE);
	task_buffer[0]=buffer[i];
	int j=0;
	// MAX_TASK_SIZE-1 to guarantee null termination
	for(j=0;j<MAX_TASK_SIZE-1 && buffer[i+j] != '\0';j++) {
		task_buffer[j] = buffer[i+j];
	}
	i+=j;
	record->task = task_buffer;
	return 0;
}

struct record *get_all(int max_records) {
	DIR *dir;
	struct record* records = malloc(sizeof(struct record) * max_records);

	struct record invalid_record;
	invalid_record.finish = 0;
	invalid_record.priority = INT_MIN;
	invalid_record.time_to_do = 0;
	invalid_record.task = 0;

	for(int i=0;i<max_records;i++) {
		records[i] = invalid_record;
	}

	struct dirent *ent;
	char buffer[MAX_FILENAME_SIZE + 16] = {0};
	// iterates over all files in current working directory
	if ((dir = opendir ("./records")) != NULL) {
		int i=0;
		while ((ent = readdir (dir)) != NULL && i < max_records) {
			if (ent->d_type != DT_DIR) {
				snprintf(buffer, MAX_FILENAME_SIZE + 16 , "./records/%s", ent->d_name);
				read_record_file_to_ptr(buffer, records+i);
				i++;
			}
		}
		closedir (dir);
	} else {
		/* could not open directory */
		perror ("");
		return NULL;
	}	
	return records;
}
