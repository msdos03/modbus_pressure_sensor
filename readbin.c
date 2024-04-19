#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//类型定义区

#define RECORD_FILE "record.bin"

struct save_unit {
	uint32_t time_ms;
	int32_t weight;
};

int main()
{
	int fd;
	struct save_unit unit;

	fd = open(RECORD_FILE, O_RDONLY);
	if (fd < 0) {
		perror(RECORD_FILE);
		return -1;
	}

	while (read(fd, &unit, sizeof(struct save_unit)) > 0) {
		printf("time(ms): %u\tweight: %d\n", unit.time_ms, unit.weight);
	}

	return 0;
}
