#include <stdio.h>
#include <modbus/modbus.h>
#include <modbus/modbus-rtu.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

//宏定义区
#define DEVICE_ADDR 1
#define BAUDRATE 9600
#define SERIAL_DEV "/dev/ttyUSB0"

//类型定义区
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef char s8;
typedef short s16;
typedef int s32;
typedef long long s64;

//函数声明区
modbus_t *open_device();

//函数区
int main()
{
	u16 tab_reg[2];
	s32 weight;
	modbus_t *sensor;

	sensor = open_device();

	while(1) {
		modbus_read_registers(sensor, 0, 2, tab_reg);
		weight = tab_reg[0] | (tab_reg[1] << 16);
		printf("weight: %dg\n", weight);
		usleep(100000);//延迟100ms
	}

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
