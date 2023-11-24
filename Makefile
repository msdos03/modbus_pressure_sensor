all:
	gcc -o setting setting.c -lmodbus
	gcc -o readval readval.c -lmodbus
	gcc -o calibrate calibrate.c -lmodbus
	gcc -o readbin readbin.c
	gcc -o binprocess binprocess.c

clean:
	rm -f setting
	rm -f readval
	rm -f calibrate
	rm -f readbin
	rm -f binprocess
	rm -f *.o

