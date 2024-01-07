#include "record.h"
#include <errno.h>
#include <inttypes.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define MAX_FILENAME_SIZE 256
#define MAX_TASK_SIZE 256

int write_record(int id, struct record *record) {
	char filename[MAX_FILENAME_SIZE] = {0};
	snprintf(filename, (MAX_FILENAME_SIZE-1), "record_%d", id);

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

struct record *read_record(char *filename) {

	
	FILE *fptr = fopen(filename, "rb");
	struct record *record = (struct record *) malloc(sizeof(struct record));

	fread(&record->finish, sizeof(record->finish), 1, fptr);

	fread(&record->priority, sizeof(record->priority), 1, fptr);

	fread(&record->time_to_do, sizeof(record->time_to_do), 1, fptr);

	char *buffer = (char *) malloc(sizeof(char) * MAX_TASK_SIZE);
	for (int i=0; i<MAX_TASK_SIZE;i++) {
		buffer[i] = 0;
	}
	int i=0;
	fread(buffer, sizeof(char), 1, fptr);
	while(i < MAX_TASK_SIZE && buffer[i] != 0) {
		i++;
		fread(buffer+i, sizeof(char), 1, fptr);
	}
	record->task = buffer;
	return record;
}
int read_record_ptr(char *filename, struct record* record) {
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
				read_record_ptr(buffer, records+i);
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
