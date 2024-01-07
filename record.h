#include <time.h>
struct record {
	time_t finish;
	time_t time_to_do;
	// invalid if priority == INT_MIN
	int priority;
	char *task;
};

// returns record or NULL in case of a fail
struct record *read_record(char *filename);
// returns 0 when succesfull otherwise -1
int read_record_ptr(char *filename, struct record* record);
// returns -1 when it fails, 0 when not
int write_record(int id, struct record *record);
// gets first record to be done
struct record *get_all(int max_records);
