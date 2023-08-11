#include <stdio.h>
#include <modbus/modbus.h>
#include <modbus/modbus-rtu.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//宏定义区
#define DEVICE_ADDR 1
#define BAUDRATE 9600
#define SERIAL_DEV "/dev/ttyUSB0"
#define SETTING_FILE "setting.cfg"

//类型定义区
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef char s8;
typedef short s16;
typedef int s32;
typedef long long s64;

struct sensor_cfg {		//中文说明	寄存器地址
	u16 zc_strength;	//追零强度	0x09
	u16 zero_chase;		//追零范围	0x0a
	u16 zerochase_enable;	//追零使能	0x0b
	u16 graduation_val;	//分度值	0x0c
	u16 midval_num;		//中值滤波值	0x0d
	u16 sample_rate;	//采样速率	0x0e
	u16 module_addr;	//模块地址	0x0f	不修改
	u16 baudrate;		//波特率	0x10	不修改
	u16 average_num;	//平均滤波值	0x11
	u16 dynamic_trace;	//动态跟踪范围	0x12
	u16 creep_trace;	//蠕变跟踪范围	0x13
	u16 stable_weight;	//稳定重量开关	0x14
} __attribute((aligned (2)));

//函数声明区
modbus_t *open_device();
s32 read_setting(modbus_t *sensor);
s32 write_setting(modbus_t *sensor);

//函数区
s32 main(int argc, char const *argv[])
{
	s32 i;
	modbus_t *sensor;
	s8 command = 'n';
	s32 weight;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-r") == 0) {
			command = 'r';
			break;
		} else if (strcmp(argv[i], "-w") == 0) {
			command = 'w';
			break;
		} else {
			printf(
			"CMCU-06 modbus force sensor setting\n"
			"\n"
			"Usage:\tsetting <-r/w>\n"
			"\n"
			"-h,--help\tShow this help page\n"
			"\n"
			"-r\t\tread settings from sensor to stdout\n"
			"\n"
			"-w\t\twrite settings from setting.cfg to sensor\n"
			"\n"
			"Author: msdos03 <https://github.com/msdos03>\n"
			);
			exit(EXIT_FAILURE);
		}
	}

	sensor = open_device();

	switch (command) {
	case 'r':
		read_setting(sensor);
		break;
	case 'w':
		write_setting(sensor);
		break;
	case 'n':
		printf(
		"setting\n"
		"\n"
		"Usage:\tsetting <-r/w>\n"
		"\n"
		"-h,--help\tShow this help page\n"
		"\n"
		"-r\t\tread settings\n"
		"\n"
		"-w\t\twrite settings\n"
		"\n"
		"Author: msdos03 <https://github.com/msdos03>\n"
		);
		exit(EXIT_FAILURE);
	}

	modbus_free(sensor);

	return 0;
}

//从setting.cfg写设定到设备 返回值：0或-1
s32 write_setting(modbus_t *sensor)
{
	s32 fd;
	u8 *filebuf;
	size_t file_size;
	u8 *linehead_p;
	u16 reg_buf;

	modbus_write_register(sensor, 0x17, 1);//解除写保护
	modbus_read_registers(sensor, 0x17, 1, &reg_buf);//检验是否解除成功
	if (reg_buf != 1) {
		printf("error: failed to turn off write protect");
		exit(-1);
	}

	fd = open(SETTING_FILE, O_RDONLY);
	file_size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);

	filebuf = malloc(file_size + 1);//为字符串结束符留一字节
	filebuf[file_size] = '\0';//给最后一个字节赋值为结束符

	read(fd, filebuf, file_size);

	linehead_p = strtok(filebuf, "\n");
	while (linehead_p != NULL) {
		if (strncmp(linehead_p, "zc_strength", strlen("zc_strength")) == 0) {
			printf("%d\n", atoi(linehead_p + sizeof("zc_strength")));
			modbus_write_register(sensor, 0x09, atoi(linehead_p + sizeof("zc_strength")));
		} else if (strncmp(linehead_p, "zero_chase", strlen("zero_chase")) == 0) {
			printf("%d\n", atoi(linehead_p + sizeof("zero_chase")));
			modbus_write_register(sensor, 0x0a, atoi(linehead_p + sizeof("zero_chase")));
		} else if (strncmp(linehead_p, "zerochase_enable", strlen("zerochase_enable")) == 0) {
			printf("%d\n", atoi(linehead_p + sizeof("zerochase_enable")));
			modbus_write_register(sensor, 0x0b, atoi(linehead_p + sizeof("zerochase_enable")));
		} else if (strncmp(linehead_p, "graduation_val", strlen("graduation_val")) == 0) {
			printf("%d\n", atoi(linehead_p + sizeof("graduation_val")));
			modbus_write_register(sensor, 0x0c, atoi(linehead_p + sizeof("graduation_val")));
		} else if (strncmp(linehead_p, "midval_num", strlen("midval_num")) == 0) {
			printf("%d\n", atoi(linehead_p + sizeof("midval_num")));
			modbus_write_register(sensor, 0x0d, atoi(linehead_p + sizeof("midval_num")));
		} else if (strncmp(linehead_p, "sample_rate", strlen("sample_rate")) == 0) {
			printf("%d\n", atoi(linehead_p + sizeof("sample_rate")));
			modbus_write_register(sensor, 0x0e, atoi(linehead_p + sizeof("sample_rate")));
		} else if (strncmp(linehead_p, "average_num", strlen("average_num")) == 0) {
			printf("%d\n", atoi(linehead_p + sizeof("average_num")));
			modbus_write_register(sensor, 0x11, atoi(linehead_p + sizeof("average_num")));
		} else if (strncmp(linehead_p, "dynamic_trace", strlen("dynamic_trace")) == 0) {
			printf("%d\n", atoi(linehead_p + sizeof("dynamic_trace")));
			modbus_write_register(sensor, 0x12, atoi(linehead_p + sizeof("dynamic_trace")));
		} else if (strncmp(linehead_p, "creep_trace", strlen("creep_trace")) == 0) {
			printf("%d\n", atoi(linehead_p + sizeof("creep_trace")));
			modbus_write_register(sensor, 0x13, atoi(linehead_p + sizeof("creep_trace")));
		} else if (strncmp(linehead_p, "stable_weight", strlen("stable_weight")) == 0) {
			printf("%d\n", atoi(linehead_p + sizeof("stable_weight")));
			modbus_write_register(sensor, 0x14, atoi(linehead_p + sizeof("stable_weight")));
		}

		usleep(100000);//等待设备准备好 100ms
		linehead_p = strtok(NULL, "\n");
	}

	modbus_write_register(sensor, 0x17, 0);//加写保护
	modbus_read_registers(sensor, 0x17, 1, &reg_buf);//检验是否加成功
	if (reg_buf != 0) {
		printf("warning: failed to turn on write protect");
		return -1;
	}

	printf("write complete\n");

	return 0;
}

//读取设定并输出到标准输出 返回值：0
s32 read_setting(modbus_t *sensor)
{
	struct sensor_cfg cfg;

	modbus_read_registers(sensor, 0x09, sizeof(cfg)/2, (u16*)(&cfg));

	printf(
	"zc_strength= %d\n"
	"zero_chase= %d\n"
	"zerochase_enable= %d\n"
	"graduation_val= %d\n"
	"midval_num= %d\n"
	"sample_rate= %d\n"
//	"module_addr= %d\n"
//	"baudrate= %d\n"
	"average_num= %d\n"
	"dynamic_trace= %d\n"
	"creep_trace= %d\n"
	"stable_weight= %d\n",
	cfg.zc_strength,
	cfg.zero_chase,
	cfg.zerochase_enable,
	cfg.graduation_val,
	cfg.midval_num,
	cfg.sample_rate,
//	cfg.module_addr,
//	cfg.baudrate,
	cfg.average_num,
	cfg.dynamic_trace,
	cfg.creep_trace,
	cfg.stable_weight);

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

	return sensor;
}
