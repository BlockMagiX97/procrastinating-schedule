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


// invalid if priority == INT_MIN
struct record {
	time_t finish;
	time_t time_to_do;
	int priority;
	char *task;
};

// returns 0 when succesfull otherwise -1
// puts read record into param record
int read_record_file_to_ptr(char *filename, struct record* record); 
// returns 0 when succesfull otherwise -1
// reads record from binary (buffer) and puts it into formated record (record)
int read_record_ptr_to_ptr(char* buffer, struct record* record);
// returns -1 when it fails, 0 when not
int write_record_filename(char * filename, struct record *record); 

struct record *get_all(int max_records);
