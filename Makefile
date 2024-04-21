compile:
	gcc -O2 setting.c 	-o setting 	-lmodbus
	gcc -O2 readval.c 	-o readval 	-lmodbus
	gcc -O2 calibrate.c 	-o calibrate 	-lmodbus
	gcc -O2 readbin.c 	-o readbin
	gcc -O2 binprocess.c 	-o binprocess

clean:
	rm -f setting
	rm -f readval
	rm -f calibrate
	rm -f readbin
	rm -f binprocess
	rm -f *.o

