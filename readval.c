#include <stdio.h>
#include <modbus/modbus.h>
#include <modbus/modbus-rtu.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//宏定义区
#define DEVICE_ADDR 1
#define BAUDRATE 9600
#define SERIAL_DEV "/dev/ttyUSB0"
#define RECORD_FILE "record.bin"

//类型定义区
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef char s8;
typedef short s16;
typedef int s32;
typedef long long s64;

struct save_unit {
	u32 time_ms;
	s32 weight;
};

//函数声明区
modbus_t *open_device();
int save_data(int fd, struct save_unit *unit);

//函数区
int main(int argc, char const *argv[])
{
	u16 tab_reg[2];
	modbus_t *sensor;
	u32 counter = 0xFFFFFFFF;
	u32 i;
	u32 delay = 100000;//单位:微秒
	s32 fd;
	struct timespec spec;
	struct timespec start;
	struct save_unit unit;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-c") == 0) {
			if (i != argc - 1) {
				counter = atoi(argv[i + 1]);
				i = i + 1;
				continue;
			} else {
				printf("-c argument cannot appear seperately\n");
				exit(EXIT_FAILURE);
			}
		} else if (strcmp(argv[i], "-d") == 0) {
			if (i != argc - 1) {
				delay = atoi(argv[i + 1]);
				i = i + 1;
				continue;
			} else {
				printf("-d argument cannot appear seperately\n");
				exit(EXIT_FAILURE);
			}
		} else if (strcmp(argv[i], "-h") == 0) {
			printf(
			"CMCU-06 modbus force sensor readval\n"
			"\n"
			"Usage:\treadval <-c> <count>\n"
			"\n"
			"-h,--help\tShow this help page\n"
			"\n"
			"-c\t\tspecify count of output\n"
			"\n"
			"-d\t\tspecify read delay in us\n"
			"\n"
			"default: count = 0xFFFFFFFF delay = 100000 us\n"
			"\n"
			"Author: msdos03 <https://github.com/msdos03>\n"
			);
			exit(EXIT_FAILURE);
		}
	}

	sensor = open_device();

	fd = open(RECORD_FILE, O_CREAT | O_WRONLY | O_TRUNC, 0x777);
	lseek(fd, 0, SEEK_SET);

	clock_gettime(CLOCK_MONOTONIC, &start);//获取起始时间

	while(counter > 0) {
		modbus_read_registers(sensor, 0, 2, tab_reg);//获取重量
		unit.weight = tab_reg[0] | (tab_reg[1] << 16);

		clock_gettime(CLOCK_MONOTONIC, &spec);//获取时间

		unit.time_ms = (spec.tv_sec - start.tv_sec) * 1000 + ((spec.tv_nsec - start.tv_nsec) / 1000000);//转化成毫秒

		save_data(fd, &unit);

		printf("weight: %dg\n", unit.weight);
		usleep(delay);//延迟delay us
		counter--;
	}

	return 0;
}

int save_data(int fd, struct save_unit *unit)
{
	printf("%d\t", unit->time_ms);

	write(fd, unit, sizeof(struct save_unit));

	return 0;
}

//打开设备函数 返回值：设备结构体指针
modbus_t *open_device()
{
	modbus_t *sensor;

	sensor = modbus_new_rtu(SERIAL_DEV, BAUDRATE, 'N', 8, 1);//打开modbus rtu设备
	if (sensor == NULL) {
 		fprintf(stderr, "Unable to create the libmodbus context\n");
		exit(-1);
	}

	if (modbus_connect(sensor) == -1) {
		fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
		modbus_free(sensor);//关闭modbus rtu设备
		exit(-1);
	}

	modbus_set_slave(sensor, DEVICE_ADDR);//设置从设备地址

	usleep(100000);//等待设备准备好

	return sensor;
}
