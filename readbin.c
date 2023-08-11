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

struct save_unit {
	u32 time_ms;
	s32 weight;
};

int main()
{
	int fd;
	struct save_unit unit;

	fd = open(RECORD_FILE, O_RDONLY);

	while (read(fd, &unit, sizeof(struct save_unit))) {
		printf("time(ms): %u\tweight: %d\n", unit.time_ms, unit.weight);
	}

	return 0;
}
