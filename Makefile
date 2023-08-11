all:
	gcc -o setting setting.c -lmodbus
	gcc -o readval readval.c -lmodbus
	gcc -o calibrate calibrate.c -lmodbus

clean:
	rm -f setting
	rm -f readval
	rm -f calibrate
	rm -f *.o

