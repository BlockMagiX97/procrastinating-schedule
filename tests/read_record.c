#include "../record.h"
#include <stdio.h>
int main() {
	struct record *rec = read_record("record_2");
	printf("finish: %ld\n", rec->finish);
	printf("time to do: %ld\n", rec->time_to_do);
	printf(rec->task);
}
