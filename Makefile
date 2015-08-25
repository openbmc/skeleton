CC=gcc
OBJS    = objects/pflash/progress.o objects/pflash/ast-sf-ctrl.o
OBJS	+= objects/pflash/libflash/libflash.o objects/pflash/libflash/libffs.o
OBJS	+= objects/pflash/arm_io.o
LIBS=/gsa/ausgsa/home/n/j/njames/openbmc
HOME = /media/sf_vbox/openbmc
CFLAGS=$(shell pkg-config --libs --cflags gtk+-2.0 glib-2.0)

%.o: interfaces/%.c 
	$(CC) -c -o obj/$@ $< -I$(HOME) $(CFLAGS)

%.o: objects/%.c
	$(CC) -c -o obj/$@ $< -I$(HOME) -I$(HOME)/objects/pflash $(CFLAGS)

%.o: objects/pflash/%.c
	$(CC) -c -o obj/$@ $< -I$(HOME) -I$(HOME)/objects/pflash $(CFLAGS)

power_control: power_control.o power_control_obj.o
	$(CC) -o bin/$@.exe obj/power_control.o obj/power_control_obj.o $(CFLAGS)

chassis_identify: led.o chassis_identify_obj.o
	$(CC) -o bin/$@.exe obj/led.o obj/chassis_identify_obj.o $(CFLAGS)

sensor_ambient: sensor.o sensor_temperature_ambient_obj.o
	$(CC) -o bin/$@.exe obj/sensor.o obj/sensor_temperature_ambient_obj.o $(CFLAGS)

button_power: button.o button_power_obj.o
	$(CC) -o bin/$@.exe obj/button.o obj/button_power_obj.o $(CFLAGS)

sensor_host_status: sensor.o sensor_host_status_obj.o
	$(CC) -o bin/$@.exe obj/sensor.o obj/sensor_host_status_obj.o $(CFLAGS)

host_control: host_control.o host_control_obj.o
	$(CC) -o bin/$@.exe obj/host_control.o obj/host_control_obj.o $(CFLAGS)

flash_bios: pflash.o flash.o flash_bios_obj.o
	$(CC) -o bin/$@.exe obj/flash.o obj/flash_bios_obj.o  $(OBJS)  $(CFLAGS)

