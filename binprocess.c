#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//类型定义区
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef char s8;
typedef short s16;
typedef int s32;
typedef long long s64;

#define RECORD_FILE "record.bin"
#define NEW_FILE "recordnew.bin"

struct save_unit {
	u32 time_ms;
	s32 weight;
};

int main()
{
	FILE* record;
	FILE* new_record;
	struct save_unit unit;

	record = fopen(RECORD_FILE, "r");
	if (record == NULL) {
		perror(RECORD_FILE);
		return -1;
	}

	new_record = fopen(NEW_FILE, "w");
	if (new_record == NULL) {
		perror(NEW_FILE);
		return -1;
	}

	while (fread(&unit, 1, sizeof(struct save_unit), record) > 0) {
		printf("time(ms): %u\tweight: %d\n", unit.time_ms, unit.weight);
		unit.weight = unit.weight + 652;
		fwrite(&unit, 1, sizeof(struct save_unit), new_record);
	}

	fclose(record);
	fclose(new_record);

	return 0;
}
