# modbus_pressure_sensor

# 使用流程:
0准备:

连接线路，确保串口设备文件为/dev/ttyUSB0

安装git,make,gcc,libmodbus-dev

克隆本仓库

在仓库目录内执行"make"来编译 (重新编译前要执行"make clean")

1设置:

运行"sudo ./setting -r > setting.cfg"来读取当前设置并保存到setting.cfg

根据下文说明和技术资料修改setting.cfg中每项设置的值

运行"sudo ./setting -w"把setting.cfg的设置写入设备

2校准:

运行"sudo ./calibrate -a"来进行零点校准和重量校准，根据程序内提示操作

3开始记录数据:

运行"sudo ./readval -c 读取次数 -d 读取间隔"来读取设备数据，读取到的数据会显示在屏幕上并写入record.bin文件

4读取记录的数据文件(record.bin)

运行"./readbin"，数据会全部输出到屏幕上

# 配置文件详解:
zc_strength=追零强度

zero_chase= 追零范围

zerochase_enable=追零使能

graduation_val=分度值

midval_num=中值滤波值

sample_rate=采样速率

average_num=平均滤波值

dynamic_trace=动态跟踪范围

creep_trace=蠕变跟踪范围

stable_weight=稳定重量开关
