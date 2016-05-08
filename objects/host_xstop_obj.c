#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <systemd/sd-bus.h>
#include "gpio.h"

/* 
 * -----------------------------------------------
 * FIXME: Fetch GPIO NUM from MRW instead?
 * -----------------------------------------------
 */
GPIO gpio_xstop    = (GPIO){ "CHECKSTOP" }; /* This object will use these GPIOs */
const int gpio_num_xstop    = 440;
const char* gpio_dev_path   = "/sys/class/gpio";
const char* gpio_dev_exp    = "/sys/class/gpio/export";
const char* gpio_dev_xstop  = "/sys/class/gpio/gpio440";
const char* gpio_val_xstop  = "/sys/class/gpio/gpio440/value";
const char* gpio_dir_xstop  = "/sys/class/gpio/gpio440/direction";

typedef struct {
    bool isXstopped;
} HostState;

HostState hostState = (HostState) {false};

static const char* xstop0      = "/org/openbmc/sensors/host/cpu0/Xstop";
static const char* srvc_name   = "org.openbmc.Sensors";
static const char* iface_name  = "org.openbmc.SensorValue";

int bus_property_get_bool(sd_bus *bus, const char *path, const char *interface, const char *property, sd_bus_message *reply, void *userdata, sd_bus_error *error)
{ 
}

static const sd_bus_vtable host_xstop_vtable[] =
{
    SD_BUS_VTABLE_START(0),
    SD_BUS_PROPERTY("IsXstopped", "b", bus_property_get_bool, offsetof(HostState, isXstopped), SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_VTABLE_END,
};


sd_bus* bus         = NULL;
sd_bus_slot* slot   = NULL;

static gboolean
on_host_xstop (GIOChannel *channel, 
                GIOCondition condition, 
                gpointer user_data)
{

	GError* error       = 0;
	gsize bytes_read    = 0;
	gchar buf[2]; 
	buf[1] = '\0';
	g_io_channel_seek_position(channel, 0, G_SEEK_SET, 0);
	GIOStatus rc = g_io_channel_read_chars(channel,
                                            buf, 1,
                                            &bytes_read,
                                            &error);
	printf("%s\n",buf);
	
	if (gpio_xstop.irq_inited)
	{
        hostState.isXstopped = true;
        /* Wait for 30 secs */ 
        /* and Reboot the host. */
	} 
	else 
    { 
        gpio_xstop.irq_inited = true; 
    }

	return true;
}

int main(void)
{
    int rc              = 0;
    int fd              = 0;
    int result          = 0;
    struct stat st;
	char data[4];

    sd_bus_error err    = SD_BUS_ERROR_NULL;
    sd_bus_message *res = NULL;

    /* Latch onto system bus. */
    rc = sd_bus_open_system(&bus);
    if(rc < 0)
    {
        fprintf(stderr,"Error opening system bus.\n");
        goto cleanup;
    }

    /* Install object for Host0 */
    rc = sd_bus_add_object_vtable (bus,
                                    &slot,
                                    xstop0,
                                    iface_name,
                                    host_xstop_vtable,
                                    NULL);
    if (rc < 0)
    {
        fprintf(stderr, "Failed to add object to dbus: %s\n", strerror(-rc));
        goto cleanup;
    }

    /*
     * Setup GPIO IRQ for Host xstop
     */
	rc = GPIO_OK;
	do {
		rc = gpio_init(bus, &gpio_xstop);
		if (rc != GPIO_OK) { goto cleanup; }
		rc = gpio_open_interrupt(&gpio_xstop, on_host_xstop, NULL/*object*/);
		if (rc != GPIO_OK) { goto cleanup; }
	} while(0);

	if (rc != GPIO_OK)
	{
		printf("ERROR PowerButton: GPIO setup (rc=%d)\n",rc);
        goto cleanup;
	}

    /* Register as a dbus service */
    rc = sd_bus_request_name (bus, srvc_name, 0);
    if (rc < 0)
    {
        fprintf(stderr, "Failed to register service: %s\n", strerror(-rc));
        goto cleanup;
    }

    /* Register for GPIO IRQ */
    gpio_xstop.irq_inited = false;
	fd = open(gpio_val_xstop, O_RDONLY | O_NONBLOCK );
	if (fd == -1)
	{
		rc = -1;
        goto cleanup;
	}
	else
	{
		GIOChannel* channel = g_io_channel_unix_new(fd);
		g_io_add_watch(channel, G_IO_PRI, on_host_xstop, NULL);
	}

    /* Register with Sensor Manager */
    sd_bus_error_free(&err);
    sd_bus_message_unref(res);

    rc = sd_bus_call_method (bus,
                                "org.openbmc.Sensors",  /* name */
                                "/org/openbmc/sensors", /* object path */
                                "org.openbmc.Sensors",  /* interface */
                                "register",
                                &err,
                                &res,
                                "ss",
                                xstop0,
                                "HostXstopSensor"); /* FIXME: register method needs the Sensor class name ! */
                             
    if(rc < 0)
    {
        fprintf(stderr, "Failed to init gpio for %s : %s\n", xstop0, err.message);
        return -1;
    }
    for (;;) {
        rc = sd_bus_process (bus, NULL);
        if (rc < 0)
        {
            fprintf(stderr, "Failed to process bus: %s\n", strerror(-rc));
            goto cleanup;
        }
        if (rc > 0) continue;

        rc = sd_bus_wait (bus, (uint64_t)-1);
        if (rc < 0)
        {
            fprintf(stderr, "Failed to wait on bus: %s\n", strerror(-rc));
            goto cleanup;
        }
    }
                                
cleanup:
    sd_bus_slot_unref (slot);
    sd_bus_unref (bus);
    return rc;
}
