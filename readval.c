#include <stdio.h>
#include <stdint.h>
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
#include <signal.h>

//宏定义区
#define DEVICE_ADDR 1
#define BAUDRATE 9600
#define SERIAL_DEV "/dev/ttyUSB0"
#define RECORD_FILE "record.bin"

//类型定义区

//全局变量区
int fd;
modbus_t *sensor;
struct timespec start_time;
volatile int quit_flag = 0;

//结构体定义区
struct save_unit {
	uint32_t time_ms;
	int32_t weight;
};

//函数声明区
modbus_t *open_device();
void record_once(int sig);

//函数区
void safe_exit(int sig)
{
	quit_flag = 1;
	return;
}

int main(int argc, char *argv[])
{
	uint32_t i;
	uint32_t delay = 100;//单位:ms
	struct timespec delay_ts;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-d") == 0) {
			if (i != argc - 1) {
				delay = atoi(argv[i + 1]);
				if(delay < 40) {
					printf("warning: delay cannot be less than 40ms, setting it to 40ms");
					delay = 40;
				}
				i = i + 1;
				continue;
			} else {
				printf("-d argument cannot appear seperately\n");
				exit(EXIT_FAILURE);
			}
		} else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
			printf(
			"CMCU-06 modbus force sensor readval\n"
			"\n"
			"Usage:\treadval <-d> <delay>\n"
			"\n"
			"-h,--help\tShow this help page\n"
			"\n"
			"-d\t\tspecify read delay in ms (no less than 40ms)\n"
			"\n"
			"default: delay = 100 ms\n"
			"\n"
			"To exit the program safely, press CTRL+C\n"
			"\n"
			"Author: msdos03 <https://github.com/msdos03>\n"
			);
			exit(EXIT_FAILURE);
		}
	}

	delay_ts.tv_sec = delay / 1000;
	delay_ts.tv_nsec = (delay % 1000) * 1000000;

	sensor = open_device();

	fd = open(RECORD_FILE, O_CREAT | O_WRONLY | O_TRUNC, 0x777);
	lseek(fd, 0, SEEK_SET);

	//重定向sigint信号到safe_exit处理函数
	struct sigaction sigact = {
		.sa_handler = safe_exit,
		.sa_flags = 0,
	};
	sigemptyset(&sigact.sa_mask);

	sigaction(SIGINT, &sigact, NULL);

	//重定向sigusr1信号到record_once处理函数，来为设置定时器作准备
	sigact.sa_handler = record_once;
	sigaction(SIGUSR1, &sigact, NULL);

	timer_t timer;
	struct sigevent sev = {
		.sigev_notify = SIGEV_SIGNAL,
		.sigev_signo = SIGUSR1,
	};
	timer_create(CLOCK_MONOTONIC, &sev, &timer);

	struct itimerspec its = {
		.it_interval = delay_ts,
		.it_value = delay_ts,
	};

	clock_gettime(CLOCK_MONOTONIC_COARSE, &start_time);//获取起始时间
	timer_settime(timer, 0, &its, NULL);//启动定时器

	while(1) {
		sleep(1);
	}

	return 0;
}

void record_once(int sig)
{
	uint16_t tab_reg[2];
	struct save_unit unit;
	struct timespec spec;

	if (quit_flag) {
		printf("\nrecording program exit\n");
		close(fd);
		modbus_free(sensor);
		exit(0);
	}

	modbus_read_registers(sensor, 0, 2, tab_reg);//获取重量
	unit.weight = tab_reg[0] | (tab_reg[1] << 16);

	clock_gettime(CLOCK_MONOTONIC_COARSE, &spec);//获取时间，记录时间不得超过u32最大值毫秒（约为1193小时）
	unit.time_ms = (spec.tv_sec - start_time.tv_sec) * 1000 + ((spec.tv_nsec - start_time.tv_nsec) / 1000000);

	//将数据写入文件
	write(fd, &unit, sizeof(struct save_unit));
	//质量除以128舍弃余数，打印这个数量的'#'来可视化数据
	if(unit.weight > 0) {
		for(uint32_t i = 0; i <= (unit.weight)>>7; i++) {
			putchar('#');
		}
	}

	printf("\ntime: %u\tweight: %dg\n", unit.time_ms, unit.weight);

	return;
}

//打开设备函数 返回值：设备结构体指针
modbus_t *open_device()
{
	modbus_t *sensor;
	//打开modbus rtu设备
	sensor = modbus_new_rtu(SERIAL_DEV, BAUDRATE, 'N', 8, 1);
	if (sensor == NULL) {
 		fprintf(stderr, "Unable to create the libmodbus context\n");
		exit(-1);
	}

	if (modbus_connect(sensor) == -1) {
		fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
		//关闭modbus rtu设备
		modbus_free(sensor);
		exit(-1);
	}

	//设置从设备地址
	modbus_set_slave(sensor, DEVICE_ADDR);
	//等待设备准备好
	usleep(100000);

	return sensor;
}
