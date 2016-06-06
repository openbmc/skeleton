#CC=gcc
OBJS    = objects/pflash/progress.o objects/pflash/ast-sf-ctrl.o
OBJS	+= objects/pflash/libflash/libflash.o objects/pflash/libflash/libffs.o
OBJS	+= objects/pflash/arm_io.o
OBJS2   = progress.o ast-sf-ctrl.o libflash.o libffs.o arm_io.o
OBJS3   = obj/progress.o obj/ast-sf-ctrl.o obj/libflash.o obj/libffs.o obj/arm_io.o
INCLUDES=$(shell pkg-config --cflags gio-unix-2.0 glib-2.0) -Iincludes -Iobjects/pflash -I.
LIBS=$(shell pkg-config --libs gio-unix-2.0 glib-2.0) -Llib -lopenbmc_intf
INCLUDES += $(shell pkg-config --cflags --libs libsystemd) -I. -O2
LIB_FLAG += $(shell pkg-config  --libs libsystemd)

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

all: setup libopenbmc_intf power_control led_controller button_power button_reset control_host host_watchdog board_vpd pcie_slot_present flash_bios flasher pflash hwmons_barreleye control_bmc

setup: 
	mkdir -p obj lib

clean:  
	rm -rf obj lib bin/*.exe

libopenbmc_intf: openbmc_intf.o
	$(CC) -shared -o lib/$@.so obj/openbmc_intf.o $(LDFLAGS)

power_control: power_control_obj.o gpio.o gpio-init-gdbus.o object_mapper.o libopenbmc_intf
	$(CC) -o bin/$@.exe obj/gpio.o obj/gpio-init-gdbus.o obj/power_control_obj.o obj/object_mapper.o $(LDFLAGS) $(LIBS)

led_controller: led_controller.o
	$(CC) -o bin/$@.exe obj/led_controller.o $(LDFLAGS) $(LIB_FLAG)

button_power: button_power_obj.o gpio.o gpio-init-gdbus.o object_mapper.o libopenbmc_intf
	$(CC) -o bin/$@.exe obj/button_power_obj.o obj/gpio.o obj/gpio-init-gdbus.o obj/object_mapper.o $(LDFLAGS) $(LIBS)

button_reset: button_reset_obj.o gpio.o gpio-init-gdbus.o object_mapper.o libopenbmc_intf
	$(CC) -o bin/$@.exe obj/button_reset_obj.o obj/gpio.o obj/gpio-init-gdbus.o obj/object_mapper.o $(LDFLAGS) $(LIBS)

control_host: control_host_obj.o gpio.o gpio-init-gdbus.o object_mapper.o libopenbmc_intf
	$(CC) -o bin/$@.exe obj/gpio.o obj/gpio-init-gdbus.o obj/control_host_obj.o obj/object_mapper.o $(LDFLAGS) $(LIBS)

flash_bios:  flash_bios_obj.o libopenbmc_intf
	$(CC) -o bin/$@.exe obj/flash_bios_obj.o $(LDFLAGS) $(LIBS)

host_watchdog: host_watchdog_obj.o libopenbmc_intf
	$(CC) -o bin/$@.exe obj/host_watchdog_obj.o $(LDFLAGS) $(LIBS)

host_xstop: host_xstop_obj.o gpio.o gpio-init-sdbus.o
	$(CC) -o bin/$@.exe obj/host_xstop_obj.o obj/gpio.o obj/gpio-init-gdbus.o $(LDFLAGS) $(LIBS) -lsystemd

board_vpd: board_vpd_obj.o libopenbmc_intf
	$(CC) -o bin/$@.exe obj/board_vpd_obj.o $(LDFLAGS) $(LIBS)

pcie_slot_present: pcie_slot_present_obj.o gpio.o gpio-init-gdbus.o libopenbmc_intf
	$(CC) -o bin/$@.exe obj/pcie_slot_present_obj.o obj/gpio.o obj/gpio-init-gdbus.o $(LDFLAGS) $(LIBS)

flasher:  $(OBJS2) flasher_obj.o libopenbmc_intf
	$(CC) -o bin/$@.exe obj/flasher_obj.o $(OBJS3) $(LDFLAGS) $(LIBS)

pflash:  $(OBJS2) pflash.o
	$(CC) -o bin/$@ obj/pflash.o $(OBJS3) $(LDFLAGS)

hwmons_barreleye: hwmons_barreleye.o libopenbmc_intf
	$(CC) -o bin/$@.exe obj/hwmons_barreleye.o $(LDFLAGS) $(LIBS)

control_bmc: control_bmc_obj.o libopenbmc_intf
	$(CC) -o bin/$@.exe obj/control_bmc_obj.o $(LDFLAGS) $(LIBS)

