GDBUSLIBS=-Llib $(shell pkg-config --libs gio-unix-2.0 glib-2.0) -lopenbmc_intf
SDBUSLIBS=$(shell pkg-config --libs libsystemd)

GDBUSINC=$(shell pkg-config --cflags gio-unix-2.0 glib-2.0) -Iincludes -I.
SDBUSINC=$(shell pkg-config --cflags libsystemd)
PFLASHINC=-Ilibflash -Icommon
INCLUDES=$(GDBUSINC) $(SDBUSINC) $(PFLASHINC)

%.o: interfaces/%.c 
	$(CC) -c -fPIC -o obj/$@ $< $(CFLAGS) $(INCLUDES)

%.o: objects/%.c
	$(CC) -c -o obj/$@ $< $(CFLAGS) $(INCLUDES)

%.o: includes/%.c
	$(CC) -c -o obj/$@ $< $(CFLAGS) $(INCLUDES)

all: setup libopenbmc_intf power_control led_controller button_power button_reset control_host host_watchdog board_vpd pcie_slot_present flash_bios flasher hwmons_barreleye control_bmc

setup: 
	mkdir -p obj lib

clean:  
	rm -rf obj lib bin/*.exe

libopenbmc_intf: openbmc_intf.o
	$(CC) -shared -o lib/$@.so obj/openbmc_intf.o $(LDFLAGS)

power_control: power_control_obj.o gpio.o libopenbmc_intf
	$(CC) -o bin/$@.exe obj/gpio.o obj/power_control_obj.o $(LDFLAGS) $(GDBUSLIBS)

led_controller: led_controller.o
	$(CC) -o bin/$@.exe obj/led_controller.o $(LDFLAGS) $(SDBUSLIBS)

button_power: button_power_obj.o gpio.o libopenbmc_intf
	$(CC) -o bin/$@.exe obj/button_power_obj.o obj/gpio.o $(LDFLAGS) $(GDBUSLIBS)

button_reset: button_reset_obj.o gpio.o libopenbmc_intf
	$(CC) -o bin/$@.exe obj/button_reset_obj.o obj/gpio.o $(LDFLAGS) $(GDBUSLIBS)


control_host: control_host_obj.o gpio.o libopenbmc_intf
	$(CC) -o bin/$@.exe obj/gpio.o obj/control_host_obj.o $(LDFLAGS) $(GDBUSLIBS)

flash_bios:  flash_bios_obj.o libopenbmc_intf
	$(CC) -o bin/$@.exe obj/flash_bios_obj.o $(LDFLAGS) $(GDBUSLIBS)

host_watchdog: host_watchdog_obj.o libopenbmc_intf
	$(CC) -o bin/$@.exe obj/host_watchdog_obj.o $(LDFLAGS) $(GDBUSLIBS)

board_vpd: board_vpd_obj.o libopenbmc_intf
	$(CC) -o bin/$@.exe obj/board_vpd_obj.o $(LDFLAGS) $(GDBUSLIBS)

pcie_slot_present: pcie_slot_present_obj.o gpio.o libopenbmc_intf
	$(CC) -o bin/$@.exe obj/pcie_slot_present_obj.o obj/gpio.o $(LDFLAGS) $(GDBUSLIBS)

flasher: flasher_obj.o libopenbmc_intf
	$(CC) -o bin/$@.exe obj/flasher_obj.o $(LDFLAGS) $(GDBUSLIBS) -lflash

hwmons_barreleye: hwmons_barreleye.o libopenbmc_intf
	$(CC) -o bin/$@.exe obj/hwmons_barreleye.o $(LDFLAGS) $(GDBUSLIBS)

control_bmc: control_bmc_obj.o libopenbmc_intf
	$(CC) -o bin/$@.exe obj/control_bmc_obj.o $(LDFLAGS) $(GDBUSLIBS)
