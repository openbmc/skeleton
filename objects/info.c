#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <systemd/sd-bus.h>
#include "i2c-dev.h"
#include "log.h"


#define NUM_CPU_CORE	12

const char *gService = "org.openbmc.Sensors";
const char *gCPU0ObjPath[NUM_CPU_CORE] = {"/org/openbmc/sensors/temperature/cpu0/core0",
						"/org/openbmc/sensors/temperature/cpu0/core1",
						"/org/openbmc/sensors/temperature/cpu0/core2",
						"/org/openbmc/sensors/temperature/cpu0/core3",
						"/org/openbmc/sensors/temperature/cpu0/core4",
						"/org/openbmc/sensors/temperature/cpu0/core5",
						"/org/openbmc/sensors/temperature/cpu0/core6",
						"/org/openbmc/sensors/temperature/cpu0/core7",
						"/org/openbmc/sensors/temperature/cpu0/core8",
						"/org/openbmc/sensors/temperature/cpu0/core9",
						"/org/openbmc/sensors/temperature/cpu0/core10",
						"/org/openbmc/sensors/temperature/cpu0/core11"};

const char *gCPU1ObjPath[NUM_CPU_CORE] = {"/org/openbmc/sensors/temperature/cpu1/core0",
						"/org/openbmc/sensors/temperature/cpu1/core1",
						"/org/openbmc/sensors/temperature/cpu1/core2",
						"/org/openbmc/sensors/temperature/cpu1/core3",
						"/org/openbmc/sensors/temperature/cpu1/core4",
						"/org/openbmc/sensors/temperature/cpu1/core5",
						"/org/openbmc/sensors/temperature/cpu1/core6",
						"/org/openbmc/sensors/temperature/cpu1/core7",
						"/org/openbmc/sensors/temperature/cpu1/core8",
						"/org/openbmc/sensors/temperature/cpu1/core9",
						"/org/openbmc/sensors/temperature/cpu1/core10",
						"/org/openbmc/sensors/temperature/cpu1/core11"};

const char *gObjPath_o = "/org/openbmc/sensors/host/cpu0/OccStatus";
const char *gIntPath = "org.openbmc.SensorValue";

const char *gService_c = "org.openbmc.control.Chassis";
const char *gObjPath_c = "/org/openbmc/control/chassis0";
const char *gIntPath_c = "org.openbmc.control.Chassis";
//const char *chassis_iface = "org.openbmc.SensorValue";

char *gMessage = NULL;

#define MAX_BYTES 255

int g_use_pec = 0;
int g_has_write = 1;
int g_n_write = 0;
uint8_t g_write_bytes[MAX_BYTES];
int g_has_read = 1;
int g_n_read = -1;
uint8_t g_read_bytes[MAX_BYTES];
uint8_t g_read_tmp[MAX_BYTES];
uint8_t g_bus = -1;
uint8_t g_slave_addr = 0xff;

static int i2c_open()
{
	int rc = 0, fd = -1;
	char fn[32];

	g_bus = 6;
	snprintf(fn, sizeof(fn), "/dev/i2c-%d", g_bus);
	fd = open(fn, O_RDWR);
	if (fd == -1) {
		LOG_ERR(errno, "Failed to open i2c device %s", fn);
		close(fd);
		return -1;
	}

	g_slave_addr = 0x4f;

	rc = ioctl(fd, I2C_SLAVE, g_slave_addr);
	if (rc < 0) {
		LOG_ERR(errno, "Failed to open slave @ address 0x%x", g_slave_addr);
		close(fd);
	}

	return fd;
}

static int i2c_io(int fd) {
	struct i2c_rdwr_ioctl_data data;
	  struct i2c_msg msg[2];
	int rc = 0, n_msg = 0;

	memset(&msg, 0, sizeof(msg));

	g_slave_addr = 0x5f;
	g_use_pec = 0;
	g_n_write = 3;
	g_write_bytes[0] = 0xfc;
	g_write_bytes[1] = 0x3;
	g_write_bytes[2] = 0x0;
	g_n_read = 5;

	if (1) {
		msg[n_msg].addr = g_slave_addr;
		msg[n_msg].flags = (g_use_pec) ? I2C_CLIENT_PEC : 0;
		msg[n_msg].len = g_n_write;
		msg[n_msg].buf = g_write_bytes;
		n_msg++;
	}

	if (1) {
		msg[n_msg].addr = g_slave_addr;
		msg[n_msg].flags = I2C_M_RD | ((g_use_pec) ? I2C_CLIENT_PEC : 0)
					| ((g_n_read == 0) ? I2C_M_RECV_LEN : 0);
		/*
		* In case of g_n_read is 0, block length will be added by
		* the underlying bus driver.
		*/
		msg[n_msg].len = (g_n_read) ? g_n_read : 256;
		msg[n_msg].buf = g_read_bytes;
		if (g_n_read == 0) {
			/* If we're using variable length block reads, we have to set the
			* first byte of the buffer to at least one or the kernel complains.
			*/
			g_read_bytes[0] = 1;
		}
		n_msg++;
	  }

	data.msgs = msg;
	data.nmsgs = n_msg;

	rc = ioctl(fd, I2C_RDWR, &data);
	if (rc < 0) {
		LOG_ERR(errno, "Failed to do raw io");
		close(fd);
		return -1;
	}

	return 0;
}

int get_hdd_status(void)
{
	int fd = -1, i = 0;
	char *test = NULL;
	test = "Ken";
	char str[10];

	fd = i2c_open();
	if (fd < 0)
		return -1;

	if (i2c_io(fd) < 0) {
		close(fd);
		return -1;
	}

//	printf("Received:\n ");
	if (g_n_read == 0)
		g_n_read = g_read_bytes[0] + 1;

#if 0
	for (i = 0; i < g_n_read; i++)
		printf(" 0x%x", g_read_bytes[i]);
#endif

	if ((g_read_tmp[2] != g_read_bytes[2]) || (g_read_tmp[3] != g_read_bytes[3])) {
		sprintf(str, "HDD change:0x%x,0x%x", g_read_bytes[2], g_read_bytes[3]);
		send_esel_to_dbus(str, "Low", "assoc", "hack", 3);
	}

	g_read_tmp[2]=g_read_bytes[2];
	g_read_tmp[3]=g_read_bytes[3];
	close(fd);
}

int send_esel_to_dbus(const char *desc, const char *sev, const char *details, uint8_t *debug, size_t debuglen)
{
	sd_bus *mbus = NULL;
	sd_bus_error error = SD_BUS_ERROR_NULL;
	sd_bus_message *reply = NULL, *m = NULL;
	uint16_t value = 0;
	int ret = 0;

	fprintf(stderr,"Add SEL due to Thermal Trip\n");
	ret = sd_bus_open_system(&mbus);
	if (ret < 0) {
		fprintf(stderr, "Failed to connect to system bus: %s\n", strerror(-ret));
		goto finish;
	}

	ret = sd_bus_message_new_method_call(mbus,&m,
						"org.openbmc.records.events",
						"/org/openbmc/records/events",
						"org.openbmc.recordlog",
						"acceptHostMessage");
	if (ret < 0) {
		fprintf(stderr, "Failed to add the method object: %s\n", strerror(-ret));
		goto finish;
	}

	ret = sd_bus_message_append(m, "sss", desc, sev, details);
	if (ret < 0) {
		fprintf(stderr, "Failed add the message strings : %s\n", strerror(-ret));
		goto finish;
	}

	ret = sd_bus_message_append_array(m, 'y', debug, debuglen);
	if (ret < 0) {
		fprintf(stderr, "Failed to add the raw array of bytes: %s\n", strerror(-ret));
		goto finish;
	}

	// Call the IPMI responder on the bus so the message can be sent to the CEC
	ret = sd_bus_call(mbus, m, 0, &error, &reply);
	if (ret < 0) {
		fprintf(stderr, "Failed to call the method: %s %s\n", __FUNCTION__, strerror(-ret));
		goto finish;
	}
	ret = sd_bus_message_read(reply, "q", &value);
	if (ret < 0) {
		fprintf(stderr, "Failed to get a rc from the method: %s\n", strerror(-ret));
	}

finish:
	sd_bus_error_free(&error);
	m = sd_bus_message_unref(m);
	reply = sd_bus_message_unref(reply);
	mbus = sd_bus_flush_close_unref(mbus);
	return ret;
}

int start_system_information(void)
{
	sd_bus *bus = NULL;
	sd_bus_error bus_error = SD_BUS_ERROR_NULL;
	sd_bus_message *response = NULL;
	int ret = 0, value = 0, i = 0, HighestCPUtemp = 0, thermaltrip_lock = 0;
	char *OccStatus = NULL;

	/* Connect to the user bus this time */
	do {
		ret = sd_bus_open_system(&bus);
		if(ret < 0) {
			fprintf(stderr, "Failed to connect to system bus for info: %s\n", strerror(-ret));
			bus = sd_bus_flush_close_unref(bus);
		}
		sleep(1);
	} while (ret < 0);
	
	while (1) {
		ret = sd_bus_call_method(bus,                   // On the System Bus
					gService_c,               // Service to contact
					gObjPath_c,            // Object path
					gIntPath_c,              // Interface name
					"getPowerState",          // Method to be called
					&bus_error,                 // object to return error
					&response,                  // Response message on success
					NULL);                       // input message (string,byte)
		if(ret < 0) {
			fprintf(stderr, "Failed to get power state from dbus: %s\n", bus_error.message);
			goto finish;
		}

		ret = sd_bus_message_read(response, "i", &value);
		if (ret < 0 ) {
			fprintf(stderr, "Failed to parse GetPowerState response message:[%s]\n", strerror(-ret));
			goto finish;
		}
		sd_bus_error_free(&bus_error);
		response = sd_bus_message_unref(response);
//		fprintf(stderr,"PowerState value = [%d] \n",value);
		if (value == 0 ) {
			thermaltrip_lock = 0;
			goto finish;
		} else {
			if (thermaltrip_lock == 1)	//probabily log two thermaltrip events, so block by this flag
				goto finish;
		}
	
		get_hdd_status();

		ret = sd_bus_call_method(bus,                   // On the System Bus
					gService,               // Service to contact
					gObjPath_o,            // Object path
					gIntPath,              // Interface name
					"getValue",          // Method to be called
					&bus_error,                 // object to return error
					&response,                  // Response message on success
					NULL);                       // input message (string,byte)
		if(ret < 0) {
			fprintf(stderr, "Failed to get OCC state from dbus: %s\n", bus_error.message);
			goto finish;
		}
		ret = sd_bus_message_read(response, "v","s", &OccStatus);
		if (ret < 0 ) {
			fprintf(stderr, "Failed to parse GetOccState response message:[%s]\n", strerror(-ret));
			goto finish;
		}
		sd_bus_error_free(&bus_error);
		response = sd_bus_message_unref(response);
//		fprintf(stderr,"OCCState value = [%s][%d] \n",OccStatus,strcmp(OccStatus, "Disable"));
		if (strcmp(OccStatus, "Disable") != 1 )
			goto finish;

		for(i=0; i<NUM_CPU_CORE; i++) {
			ret = sd_bus_call_method(bus,                   // On the System Bus
						gService,               // Service to contact
						gCPU0ObjPath[i],            // Object path
						gIntPath,              // Interface name
						"getValue",          // Method to be called
						&bus_error,                 // object to return error
						&response,                  // Response message on success
						NULL);                       // input message (string,byte)
			if(ret < 0) {
//				fprintf(stderr, "Failed to get CPU 0 temperature from dbus: %s\n", bus_error.message);
				value = 0;
			} else {
				ret = sd_bus_message_read(response, "v","i", &value);
				if (ret < 0 ) {
					fprintf(stderr, "Failed to parse GetCpu0Temp response message:[%s]\n", strerror(-ret));
					value = 0;
				}
			}
//			fprintf(stderr, "CPU0 core %d temperature is %d\n",i ,value);
			if(value > HighestCPUtemp)
				HighestCPUtemp = value;

			sd_bus_error_free(&bus_error);
			response = sd_bus_message_unref(response);
		}

		for(i=0; i<NUM_CPU_CORE; i++) {
			ret = sd_bus_call_method(bus,                   // On the System Bus
						gService,               // Service to contact
						gCPU1ObjPath[i],            // Object path
						gIntPath,              // Interface name
						"getValue",          // Method to be called
						&bus_error,                 // object to return error
						&response,                  // Response message on success
						NULL);                       // input message (string,byte)
			if(ret < 0) {
//				fprintf(stderr, "Failed to get CPU 1 temperature from dbus: %s\n", bus_error.message);
				value = 0;
			} else {
				ret = sd_bus_message_read(response, "v","i", &value);
				if (ret < 0 ) {
					fprintf(stderr, "Failed to parse GetCpu1Temp response message:[%s]\n", strerror(-ret));
					value = 0;
				}
			}
//			fprintf(stderr, "CPU1 core %d temperature is %d\n",i ,value);
			if(value > HighestCPUtemp )
				HighestCPUtemp = value;

			sd_bus_error_free(&bus_error);
			response = sd_bus_message_unref(response);
		}
//		fprintf(stderr, "Highest CPU temperature = [%d]\n", HighestCPUtemp);

		if(HighestCPUtemp >= 90) {
			thermaltrip_lock = 1;
			send_esel_to_dbus("thermal shutdown, CPU temperature = 90degC", "High", "assoc", "hack", 3);

			ret = sd_bus_call_method(bus,                   // On the System Bus
						gService_c,               // Service to contact
						gObjPath_c,            // Object path
						gIntPath_c,              // Interface name
						"powerOff",          // Method to be called
						&bus_error,                 // object to return error
						&response,                  // Response message on success
						NULL);                       // input message (string,byte)
			if(ret < 0)
			{
				fprintf(stderr, "Failed to power off from dbus: %s\n", bus_error.message);
				goto finish;
			}
			sd_bus_error_free(&bus_error);
			response = sd_bus_message_unref(response);
		}
	
finish:
		sd_bus_error_free(&bus_error);
		response = sd_bus_message_unref(response);
		sd_bus_flush(bus);
		OccStatus = NULL;
		HighestCPUtemp = 0;
		sleep(1);
	}

	bus = sd_bus_flush_close_unref(bus);
	return ret < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}


int main(int argc, char *argv[]) {

	return start_system_information();
}
