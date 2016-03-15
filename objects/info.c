#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <systemd/sd-bus.h>
#include "i2c-dev.h"
#include "log.h"

const char *gService = "org.openbmc.Sensors";
const char *gObjPath = "/org/openbmc/sensors/temperature/cpu0/core0";
const char *gObjPath_o = "/org/openbmc/sensors/host/OccStatus";
const char *gIntPath = "org.openbmc.SensorValue";

const char *gService_c = "org.openbmc.control.Chassis";
const char *gObjPath_c = "/org/openbmc/control/chassis0";
const char *gIntPath_c = "org.openbmc.control.Chassis";
//const char *chassis_iface = "org.openbmc.SensorValue";

char *gMessage = NULL;
sd_bus *bus = NULL;

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

static int i2c_open() {
  int fd;
  char fn[32];
  int rc;

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
  int n_msg = 0;
  int rc;

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
    msg[n_msg].flags = I2C_M_RD
      | ((g_use_pec) ? I2C_CLIENT_PEC : 0)
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
  int fd, i;
  char *test;
  test="Ken";
  char str[10];
  fd = i2c_open();
  if (fd < 0) {
    return -1;
  }

  if (i2c_io(fd) < 0) {
    close(fd);
    return -1;
  }

    //printf("Received:\n ");
    if (g_n_read == 0) {
      g_n_read = g_read_bytes[0] + 1;
    }
    for (i = 0; i < g_n_read; i++) {
      //printf(" 0x%x", g_read_bytes[i]);
    }
    if ((g_read_tmp[2]!=g_read_bytes[2])||(g_read_tmp[3]!=g_read_bytes[3]))
    {
    sprintf(str, "HDD change:0x%x,0x%x", g_read_bytes[2], g_read_bytes[3]);
   
	send_esel_to_dbus(str, "Low", "assoc", "hack", 3);
    }

    g_read_tmp[2]=g_read_bytes[2];
    g_read_tmp[3]=g_read_bytes[3];
    close(fd);
}

int send_esel_to_dbus(const char *desc, const char *sev, const char *details, uint8_t *debug, size_t debuglen) {

	sd_bus *mbus = NULL;
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message *reply = NULL, *m=NULL;
    uint16_t x;
    int r;
	sd_bus_error_free(&error);

	printf("add sel\n");
 	r = sd_bus_open_system(&mbus);
	if (r < 0) {
		fprintf(stderr, "Failed to connect to system bus: %s\n", strerror(-r));
		goto finish;
	}

    r = sd_bus_message_new_method_call(mbus,&m,
    									"org.openbmc.records.events",
    									"/org/openbmc/records/events",
    									"org.openbmc.recordlog",
    									"acceptHostMessage");
    if (r < 0) {
        fprintf(stderr, "Failed to add the method object: %s\n", strerror(-r));
        goto finish;
    }

    r = sd_bus_message_append(m, "sss", desc, sev, details);
    if (r < 0) {
        fprintf(stderr, "Failed add the message strings : %s\n", strerror(-r));
        goto finish;
    }

    r = sd_bus_message_append_array(m, 'y', debug, debuglen);
    if (r < 0) {
        fprintf(stderr, "Failed to add the raw array of bytes: %s\n", strerror(-r));
        goto finish;
    }
    // Call the IPMI responder on the bus so the message can be sent to the CEC
    r = sd_bus_call(mbus, m, 0, &error, &reply);
    if (r < 0) {
        fprintf(stderr, "Failed to call the method: %s %s\n", __FUNCTION__, strerror(-r));
        goto finish;
    }
    r = sd_bus_message_read(reply, "q", &x);
    if (r < 0) {
        fprintf(stderr, "Failed to get a rc from the method: %s\n", strerror(-r));
    }

finish:
    sd_bus_error_free(&error);
    sd_bus_message_unref(m);
    sd_bus_message_unref(reply);
    return r;	
}
int start_system_information(void) {

	sd_bus *bus;
	sd_bus_slot *slot;
	int r, x, rc,retry;
	char *OccStatus;
    r = -1;
	while(r < 0) {
	/* Connect to the user bus this time */
	r = sd_bus_open_system(&bus);
		if(r < 0){	
			fprintf(stderr, "Failed to connect to system bus: %s\n", strerror(-r));
		sleep(1);
			}
	//	goto finish;
	
		}
	// SD Bus error report mechanism.
	sd_bus_error bus_error = SD_BUS_ERROR_NULL;
	sd_bus_message *response = NULL, *m = NULL;;
        sd_bus_error_free(&bus_error);
        sd_bus_message_unref(response);
//		send_esel_to_dbus("desc", "sev", "assoc", "hack", 3);

while(1){
       sd_bus_error_free(&bus_error);


	   
       rc = sd_bus_call_method(bus,                   // On the System Bus
                                gService_c,               // Service to contact
                                gObjPath_c,            // Object path
                                gIntPath_c,              // Interface name
                                "getPowerState",          // Method to be called
                                &bus_error,                 // object to return error
                                &response,                  // Response message on success
                                NULL);                       // input message (string,byte)
                                // NULL);                  // First argument to getObjectFromId
                                //"BOARD_1");             // Second Argument
        if(rc < 0)
        {
            fprintf(stderr, "Failed to resolve getPowerState to dbus: %s\n", bus_error.message);
	    r = sd_bus_open_system(&bus);
                if(r < 0){
                        fprintf(stderr, "Failed to connect to system bus: %s\n", strerror(-r));
                        sleep(1);
                        }

            goto finish;
        }

        rc = sd_bus_message_read(response, "i", &x);
        if (rc < 0 )
        {
           fprintf(stderr, "Failed to parse response message:[%s]\n", strerror(-rc));
           goto finish;
        }
        printf("PowerState value=[%d] \n",x);
	if (x == 0 ) goto finish;
	get_hdd_status();	
        sleep(1);
       sd_bus_error_free(&bus_error);
       rc = sd_bus_call_method(bus,                   // On the System Bus
                                gService,               // Service to contact
                                gObjPath_o,            // Object path
                                gIntPath,              // Interface name
                                "getValue",          // Method to be called
                                &bus_error,                 // object to return error
                                &response,                  // Response message on success
                                NULL);                       // input message (string,byte)
                                // NULL);                  // First argument to getObjectFromId
                                //"BOARD_1");             // Second Argument
        if(rc < 0)
        {
            fprintf(stderr, "Failed to resolve fruid to dbus: %s\n", bus_error.message);
            goto finish;
        }

        rc = sd_bus_message_read(response, "v","s", &OccStatus);
        if (rc < 0 )
        {
           fprintf(stderr, "Failed to parse response message:[%s]\n", strerror(-rc));
           goto finish;
        }
        printf("OCCtate value=[%s][%d] \n",OccStatus,strcmp(OccStatus, "Disable"));
	
        if (strcmp(OccStatus, "Disable") != 1 ) goto finish;

        rc = sd_bus_call_method(bus,                   // On the System Bus
                                gService,               // Service to contact
                                gObjPath,            // Object path
                                gIntPath,              // Interface name
                                "getValue",          // Method to be called
                                &bus_error,                 // object to return error
                                &response,                  // Response message on success
                                NULL);                       // input message (string,byte)
                                // NULL);                  // First argument to getObjectFromId
                                //"BOARD_1");             // Second Argument

        if(rc < 0)
        {
            fprintf(stderr, "Failed to resolve fruid to dbus: %s\n", bus_error.message);
            goto finish;
        }

	rc = sd_bus_message_read(response, "v","i", &x);
	if (rc < 0 )
	{ 
           fprintf(stderr, "Failed to parse response message:[%s]\n", strerror(-rc));
           goto finish;
	}
	printf("CPU value=[%d] \n",x);

	if(x >= 90)
	{
	//printf("====Ken poweroff==== \n");
	send_esel_to_dbus("CPU thermal trip", "High", "assoc", "hack", 3);
	sd_bus_error_free(&bus_error);
        rc = sd_bus_call_method(bus,                   // On the System Bus
                                gService_c,               // Service to contact
                                gObjPath_c,            // Object path
                                gIntPath_c,              // Interface name
                                "powerOff",          // Method to be called
                                &bus_error,                 // object to return error
                                &response,                  // Response message on success
                                NULL);                       // input message (string,byte)
                                // NULL);                  // First argument to getObjectFromId
                                //"BOARD_1");             // Second Argument

        if(rc < 0)
        {
            fprintf(stderr, "Failed to resolve poweroff to dbus: %s\n", bus_error.message);
            goto finish;
        }

	}
	
	sleep(1);
///		sd_bus_unref(bus);
	finish:
		sd_bus_unref(bus);
		sleep(1);
}
	return r < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}


int main(int argc, char *argv[]) {

	return start_system_information();
}
