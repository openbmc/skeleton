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

static const char* xstop0      = "/org/openbmc/sensors/host/cpu0/Xstop";
static const char* srvc_name   = "org.openbmc.Sensors";
static const char* iface_name  = "org.openbmc.SensorValue";

GPIO gpio_xstop0	= (GPIO){ "CHECKSTOP0" }; /* This object will use these GPIOs */
sd_bus* bus         = NULL;
sd_bus_slot* slot   = NULL;

typedef struct {
    bool isXstopped;
} HostState;

HostState hostState0 = (HostState) {false};

int bus_property_get_bool(sd_bus *bus, const char *path, const char *interface, const char *property, sd_bus_message *reply, void *userdata, sd_bus_error *error)
{ 
        int b = *(bool*) userdata;

        return sd_bus_message_append_basic(reply, 'b', &b);
}

static const sd_bus_vtable host_xstop_vtable[] =
{
    SD_BUS_VTABLE_START(0),
    SD_BUS_PROPERTY("IsXstopped", "b", bus_property_get_bool, offsetof(HostState, isXstopped), SD_BUS_VTABLE_PROPERTY_EMITS_CHANGE),
    SD_BUS_VTABLE_END,
};

static gboolean
on_host_xstop (GIOChannel *channel, GIOCondition condition, gpointer user_data)
{
	GError* error       = 0;
	gsize bytes_read    = 0;
	gchar buf[2]; 
	buf[1] = '\0';

	g_io_channel_seek_position(channel, 0, G_SEEK_SET, 0);
	GIOStatus rc = g_io_channel_read_chars(channel, buf, 1, &bytes_read, &error);

	if (gpio_xstop0.irq_inited) {
         hostState0.isXstopped = true;
    }
    else {
        gpio_xstop0.irq_inited = true;
    }

	return true;
}

int main(void)
{
    int rc              = 0;
    sd_bus_error err    = SD_BUS_ERROR_NULL;
    sd_bus_message *res = NULL;

    /* Latch onto system bus. */
    rc = sd_bus_open_system(&bus);
    if(rc < 0) {
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
    if (rc < 0) {
        fprintf(stderr, "Failed to add object to dbus: %s\n", strerror(-rc));
        goto cleanup;
    }

    /* Setup GPIO IRQ for Host xstop */
	rc = GPIO_OK;
	do {
		rc = gpio_init(bus, &gpio_xstop0);
		if (rc != GPIO_OK) { goto cleanup; }
		rc = gpio_open_interrupt(&gpio_xstop0, on_host_xstop, NULL/*object*/);
		if (rc != GPIO_OK) { goto cleanup; }
	} while(0);

	if (rc != GPIO_OK) {
		fprintf(stderr, "ERROR CHECKSTOP0: GPIO setup =%d\n", rc);
        goto cleanup;
	}

    /* Register as a dbus service */
    rc = sd_bus_request_name (bus, srvc_name, 0);
    if (rc < 0) {
        fprintf(stderr, "Failed to register service: %s\n", strerror(-rc));
        goto cleanup;
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
                                "HostXstopSensor");

    if(rc < 0) {
        fprintf(stderr, "Failed to init gpio for %s : %s\n", xstop0, err.message);
        return -1;
    }

    for (;;) {
        rc = sd_bus_process (bus, NULL);
        if (rc < 0) {
            fprintf(stderr, "Failed to process bus: %s\n", strerror(-rc));
            goto cleanup;
        }
        if (rc > 0) continue;

        rc = sd_bus_wait (bus, (uint64_t)-1);
        if (rc < 0) {
            fprintf(stderr, "Failed to wait on bus: %s\n", strerror(-rc));
            goto cleanup;
        }
    }
                                
cleanup:
    sd_bus_slot_unref (slot);
    sd_bus_unref (bus);
    return rc;
}
