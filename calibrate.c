#include <stdio.h>
#include <stdint.h>
#include <modbus/modbus.h>
#include <modbus/modbus-rtu.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

//宏定义区
#define DEVICE_ADDR 1
#define BAUDRATE 9600
#define SERIAL_DEV "/dev/ttyUSB0"

//类型定义区

//函数声明区
modbus_t *open_device();

//函数区
int main(int argc, char *argv[])
{
	modbus_t *sensor;
	int32_t weight;
	uint8_t command = 'a';
	uint32_t i;
	uint16_t reg_buf;
	uint16_t tab_reg[2];

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-z") == 0) {
			command = 'z';
			break;
		} else if (strcmp(argv[i], "-a") == 0) {
			command = 'a';
			break;
		} else if (strcmp(argv[i], "-w") == 0) {
			command = 'w';
			break;
		} else {
			printf(
			"CMCU-06 modbus force sensor calibrate\n"
			"\n"
			"Usage:\tsetting <-z/w/a>\n"
			"\n"
			"-h,--help\tShow this help page\n"
			"\n"
			"-z\t\tcalibrate zero point only\n"
			"\n"
			"-w\t\tcalibrate weight only\n"
			"\n"
			"-a\t\tcalibrate zero point and weight\n"
			"\n"
			"default: calibrate zero point and weight\n"
			"\n"
			"Author: msdos03 <https://github.com/msdos03>\n"
			);
			exit(EXIT_FAILURE);
		}
	}

	sensor = open_device();

	modbus_write_register(sensor, 0x17, 1);//解除写保护
	modbus_read_registers(sensor, 0x17, 1, &reg_buf);//检验是否解除成功
	if (reg_buf != 1) {
		printf("error: failed to turn off write protect");
		exit(-1);
	}

	modbus_read_registers(sensor, 0, 2, tab_reg);
	weight = tab_reg[0] | (tab_reg[1] << 16);
	printf("weight before calibration: %d\n", weight);

	if (command == 'z' || command == 'a') {
		printf("please remove any force on the sensor and press ENTER to continue\n");
		while (getchar() != '\n');
		modbus_write_register(sensor, 0x16, 1);//往命令寄存器写入归零命令
		sleep(1);//等待设备准备好 1000ms

		modbus_read_registers(sensor, 0, 2, tab_reg);
		weight = tab_reg[0] | (tab_reg[1] << 16);
		if (weight) {
			printf("warning: output value is: %dg after zero point calibration, please don't move the sensor\n", weight);
		} else {
			printf("zero point calibration succeeded\n");
		}
	}

	if (command == 'w' || command == 'a') {
		do {
			printf("please enter the weight you put on the sensor (20-65535):\n");
			scanf("%hu", &reg_buf);
		} while (reg_buf < 20);
		modbus_write_register(sensor, 0x06, reg_buf);//往砝码值寄存器写入用户输入的值
		sleep(1);//等待设备准备好 1000ms

		modbus_read_registers(sensor, 0, 2, tab_reg);
		weight = tab_reg[0] | (tab_reg[1] << 16);
		if (weight != reg_buf) {
			printf("warning: after weight calibration: %dg , please don't move the sensor\n", weight);
		} else {
			printf("weight calibration succeeded\n");
		}
	}

	modbus_write_register(sensor, 0x17, 0);//加写保护
	modbus_read_registers(sensor, 0x17, 1, &reg_buf);//检验是否加成功
	if (reg_buf != 0) {
		printf("warning: failed to turn on write protect");
		return -1;
	}

	modbus_free(sensor);

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
