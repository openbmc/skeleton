#CC=gcc
OBJS    = objects/pflash/progress.o objects/pflash/ast-sf-ctrl.o
OBJS	+= objects/pflash/libflash/libflash.o objects/pflash/libflash/libffs.o
OBJS	+= objects/pflash/arm_io.o
OBJS2   = progress.o ast-sf-ctrl.o libflash.o libffs.o arm_io.o
OBJS3   = obj/progress.o obj/ast-sf-ctrl.o obj/libflash.o obj/libffs.o obj/arm_io.o
LIBS = ./bin
OFLAGS =-L$(LIBS) -lopenbmc_intf
HOME = .
CFLAGS=$(shell pkg-config --libs --cflags gio-unix-2.0 glib-2.0)
#CFLAGS = -pthread -I/usr/include/gio-unix-2.0/ -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -lgio-2.0 -lgobject-2.0 -lglib-2.0

%.o: interfaces/%.c 
	$(CC) -c -fPIC -o obj/$@ $< -I$(HOME) -I$(HOME)/includes $(CFLAGS)

%.o: objects/%.c
	$(CC) -c -o obj/$@ $< -L$(LIBS) -I$(HOME) -I$(HOME)/includes -I$(HOME)/objects/pflash  $(CFLAGS)

%.o: includes/%.c
	$(CC) -c -o obj/$@ $< -L$(LIBS) -I$(HOME) -I$(HOME)/includes -I$(HOME)/objects/pflash $(CFLAGS)

%.o: objects/pflash/%.c
	$(CC) -c -o obj/$@ $< -I$(HOME) -I$(HOME)/objects/pflash  $(CFLAGS)

%.o: objects/pflash/libflash/%.c
	$(CC) -c -o obj/$@ $< -I$(HOME) -I$(HOME)/objects/pflash  $(CFLAGS)

setup: 
	mkdir obj

clean:  
	rm -rf obj

libopenbmc_intf: openbmc_intf.o
	$(CC) -shared -o bin/$@.so obj/openbmc_intf.o $(CFLAGS)

power_control: power_control_obj.o gpio.o
	$(CC) -o bin/$@.exe obj/gpio.o obj/power_control_obj.o $(OFLAGS) $(CFLAGS)

chassis_identify: chassis_identify_obj.o gpio.o
	$(CC) -o bin/$@.exe obj/gpio.o obj/chassis_identify_obj.o $(OFLAGS) $(CFLAGS)

sensor_ambient: sensor_threshold.o sensor_temperature_ambient_obj.o
	$(CC) -o bin/$@.exe obj/sensor_threshold.o obj/sensor_temperature_ambient_obj.o $(OFLAGS) $(CFLAGS)

button_power: button_power_obj.o gpio.o
	$(CC) -o bin/$@.exe obj/button_power_obj.o obj/gpio.o $(OFLAGS) $(CFLAGS)

sensor_host_status: sensor_host_status_obj.o
	$(CC) -o bin/$@.exe obj/sensor_host_status_obj.o $(OFLAGS) $(CFLAGS)

control_host: control_host_obj.o gpio.o
	$(CC) -o bin/$@.exe obj/gpio.o obj/control_host_obj.o $(OFLAGS) $(CFLAGS)

flash_bios:  flash_bios_obj.o
	$(CC) -o bin/$@.exe obj/flash_bios_obj.o $(OFLAGS)  $(CFLAGS)

fan: fan_generic_obj.o gpio.o
	$(CC) -o bin/$@.exe obj/gpio.o obj/fan_generic_obj.o $(OFLAGS) $(CFLAGS)

host_watchdog: host_watchdog_obj.o
	$(CC) -o bin/$@.exe obj/host_watchdog_obj.o $(OFLAGS) $(CFLAGS)

control_bmc: control_bmc_obj.o
	$(CC) -o bin/$@.exe obj/control_bmc_obj.o $(OFLAGS) $(CFLAGS)

sensor_occ: sensor_occ_obj.o
	$(CC) -o bin/$@.exe obj/sensor_occ_obj.o $(OFLAGS) $(CFLAGS)

board_vpd: board_vpd_obj.o
	$(CC) -o bin/$@.exe obj/board_vpd_obj.o $(OFLAGS) $(CFLAGS)

pcie_slot_present: pcie_slot_present_obj.o gpio.o
	$(CC) -o bin/$@.exe obj/pcie_slot_present_obj.o obj/gpio.o $(OFLAGS) $(CFLAGS)

flasher:  $(OBJS2) flasher_obj.o
	$(CC) -o bin/$@.exe obj/flasher_obj.o $(OFLAGS)  $(OBJS3)  $(CFLAGS)


all: clean setup libopenbmc_intf power_control chassis_identify sensor_ambient button_power sensor_host_status control_host fan host_watchdog control_bmc sensor_occ board_vpd pcie_slot_present flash_bios flasher
