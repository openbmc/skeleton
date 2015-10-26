#CC=gcc
OBJS    = objects/pflash/progress.o objects/pflash/ast-sf-ctrl.o
OBJS	+= objects/pflash/libflash/libflash.o objects/pflash/libflash/libffs.o
OBJS	+= objects/pflash/arm_io.o
OBJS2   = progress.o ast-sf-ctrl.o libflash.o libffs.o arm_io.o
OBJS3   = obj/progress.o obj/ast-sf-ctrl.o obj/libflash.o obj/libffs.o obj/arm_io.o
INCLUDES=$(shell pkg-config --cflags gio-unix-2.0 glib-2.0) -Iincludes -Iobjects/pflash -I.
LIBS=$(shell pkg-config --libs gio-unix-2.0 glib-2.0) -Lbin -lopenbmc_intf

%.o: interfaces/%.c 
	$(CC) -c -fPIC -o obj/$@ $< $(CFLAGS) $(INCLUDES)

%.o: objects/%.c
	$(CC) -c -o obj/$@ $< $(LIBS) $(CFLAGS) $(INCLUDES)

%.o: includes/%.c
	$(CC) -c -o obj/$@ $< $(LIBS) $(CFLAGS) $(INCLUDES)

%.o: objects/pflash/%.c
	$(CC) -c -o obj/$@ $< $(CFLAGS) $(INCLUDES)

%.o: objects/pflash/libflash/%.c
	$(CC) -c -o obj/$@ $< $(CFLAGS) $(INCLUDES)

setup: 
	mkdir -p obj

clean:  
	rm -rf obj

libopenbmc_intf: openbmc_intf.o
	$(CC) -shared -o bin/$@.so obj/openbmc_intf.o $(LDFLAGS)

power_control: power_control_obj.o gpio.o libopenbmc_intf
	$(CC) -o bin/$@.exe obj/gpio.o obj/power_control_obj.o $(LDFLAGS) $(LIBS)

led_controller: led_controller.o gpio.o libopenbmc_intf
	$(CC) -o bin/$@.exe obj/gpio.o obj/led_controller.o $(LDFLAGS) $(LIBS)

sensor_ambient: sensor_threshold.o sensor_temperature_ambient_obj.o libopenbmc_intf
	$(CC) -o bin/$@.exe obj/sensor_threshold.o obj/sensor_temperature_ambient_obj.o $(LDFLAGS) $(LIBS)

button_power: button_power_obj.o gpio.o libopenbmc_intf
	$(CC) -o bin/$@.exe obj/button_power_obj.o obj/gpio.o $(LDFLAGS) $(LIBS)

control_host: control_host_obj.o gpio.o libopenbmc_intf
	$(CC) -o bin/$@.exe obj/gpio.o obj/control_host_obj.o $(LDFLAGS) $(LIBS)

flash_bios:  flash_bios_obj.o libopenbmc_intf
	$(CC) -o bin/$@.exe obj/flash_bios_obj.o $(LDFLAGS) $(LIBS)

fan: fan_generic_obj.o gpio.o libopenbmc_intf
	$(CC) -o bin/$@.exe obj/gpio.o obj/fan_generic_obj.o $(LDFLAGS) $(LIBS)

host_watchdog: host_watchdog_obj.o libopenbmc_intf
	$(CC) -o bin/$@.exe obj/host_watchdog_obj.o $(LDFLAGS) $(LIBS)

control_bmc: control_bmc_obj.o libopenbmc_intf
	$(CC) -o bin/$@.exe obj/control_bmc_obj.o $(LDFLAGS) $(LIBS)

control_bmc_barreleye: control_bmc_barreleye.o libopenbmc_intf
	$(CC) -o bin/$@.exe obj/control_bmc_barreleye.o $(LDFLAGS) $(LIBS)

board_vpd: board_vpd_obj.o libopenbmc_intf
	$(CC) -o bin/$@.exe obj/board_vpd_obj.o $(LDFLAGS) $(LIBS)

pcie_slot_present: pcie_slot_present_obj.o gpio.o libopenbmc_intf
	$(CC) -o bin/$@.exe obj/pcie_slot_present_obj.o obj/gpio.o $(LDFLAGS) $(LIBS)

flasher:  $(OBJS2) flasher_obj.o libopenbmc_intf
	$(CC) -o bin/$@.exe obj/flasher_obj.o $(OBJS3) $(LDFLAGS) $(LIBS)

hwmon:  hwmon_intf.o libopenbmc_intf
	$(CC) -o bin/$@.exe obj/hwmon_intf.o $(LDFLAGS) $(LIBS)


all: setup libopenbmc_intf power_control led_controller sensor_ambient button_power control_host fan host_watchdog control_bmc board_vpd pcie_slot_present flash_bios flasher control_bmc_barreleye
