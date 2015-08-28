CC=gcc
OBJS    = objects/pflash/progress.o objects/pflash/ast-sf-ctrl.o
OBJS	+= objects/pflash/libflash/libflash.o objects/pflash/libflash/libffs.o
OBJS	+= objects/pflash/arm_io.o
LIBS=/gsa/ausgsa/home/n/j/njames/openbmc
HOME = /media/sf_vbox/openbmc
CFLAGS=$(shell pkg-config --libs --cflags gtk+-2.0 glib-2.0)

%.o: interfaces/%.c 
	$(CC) -c -o obj/$@ $< -I$(HOME) -I$(HOME)/includes $(CFLAGS)

%.o: objects/%.c
	$(CC) -c -o obj/$@ $< -I$(HOME) -I$(HOME)/includes -I$(HOME)/objects/pflash $(CFLAGS)

%.o: includes/%.c
	$(CC) -c -o obj/$@ $< -I$(HOME) -I$(HOME)/includes -I$(HOME)/objects/pflash $(CFLAGS)

%.o: objects/pflash/%.c
	$(CC) -c -o obj/$@ $< -I$(HOME) -I$(HOME)/objects/pflash $(CFLAGS)

power_control: power_control.o power_control_obj.o gpio.o
	$(CC) -o bin/$@.exe obj/gpio.o obj/power_control.o obj/power_control_obj.o $(CFLAGS)

chassis_identify: led.o chassis_identify_obj.o gpio.o
	$(CC) -o bin/$@.exe obj/gpio.o obj/led.o obj/chassis_identify_obj.o $(CFLAGS)

sensor_ambient: sensor2.o sensor_temperature_ambient_obj.o sensor_threshold.o
	$(CC) -o bin/$@.exe obj/openbmc_utilities.o obj/sensor2.o obj/sensor_temperature_ambient_obj.o $(CFLAGS)

button_power: button.o button_power_obj.o gpio.o
	$(CC) -o bin/$@.exe obj/button.o obj/button_power_obj.o $(CFLAGS)

sensor_host_status: sensor.o sensor_host_status_obj.o
	$(CC) -o bin/$@.exe obj/sensor.o obj/sensor_host_status_obj.o $(CFLAGS)

control_host: control_host.o control_host_obj.o gpio.o
	$(CC) -o bin/$@.exe obj/gpio.o obj/control_host.o obj/control_host_obj.o $(CFLAGS)

flash_bios: pflash.o flash.o flash_bios_obj.o
	$(CC) -o bin/$@.exe obj/flash.o obj/flash_bios_obj.o  $(OBJS)  $(CFLAGS)

