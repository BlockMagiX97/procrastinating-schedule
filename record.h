#include <time.h>
#ifndef PORT
#define PORT 8081
#endif
#ifndef DB_ROOT
#define DB_ROOT "/var/lib/procrast_data/"
#endif
#ifndef MAX_TASK_SIZE
#define MAX_TASK_SIZE 256
#endif

#ifndef MAX_FILENAME_SIZE
#define MAX_FILENAME_SIZE 256
#endif
#ifndef MAX_RECORD_SIZE
#define MAX_RECORD_SIZE sizeof(time_t) * 3 + sizeof(int)*2 + MAX_TASK_SIZE
#endif


// invalid if priority == INT_MIN
struct record {
	int id;
	time_t start_time;
	time_t finish;
	time_t time_to_do;
	int priority;
	char *task;
};

// returns size of buffer when succesfull otherwise -1
// puts read record into param record
int read_record_file_to_ptr(char *filename, struct record* record); 
// returns size of buffer when succesfull otherwise -1
// reads record from binary (buffer) and puts it into formated record (record)
int read_record_buf_to_ptr(char* buffer, struct record* record);
// returns -1 when it fails, 0 when not
int write_record_filename(char * filename, struct record *record); 
// returns -1 when it fails, 0 otherwise
// write record to binary from formated record (record)
int write_record_to_buffer(struct record* record, char *dest_buffer); 
// prints record
int printf_record(struct record * record); 

struct record *get_all(int max_records);
