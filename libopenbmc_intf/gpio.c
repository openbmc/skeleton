#define _GNU_SOURCE

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <argp.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "openbmc_intf.h"
#include "gpio.h"


int gpio_writec(GPIO* gpio, char value)
{
	g_assert (gpio != NULL);
	int rc = GPIO_OK;
	char buf[1];
	buf[0] = value;

	if (lseek(gpio->fd, 0, SEEK_SET) == -1)
	{
		return GPIO_ERROR;
	}

	if (write(gpio->fd, buf, 1) != 1)
	{
		rc = GPIO_WRITE_ERROR;
	}
	return rc;
}

int gpio_write(GPIO* gpio, uint8_t value)
{
	g_assert (gpio != NULL);
	int rc = GPIO_OK;
	char buf[1];
	buf[0] = '0';
	if (value==1)
	{
		buf[0]='1';
	}

	if (lseek(gpio->fd, 0, SEEK_SET) == -1)
	{
		return GPIO_ERROR;
	}

	if (write(gpio->fd, buf, 1) != 1)
	{
		rc = GPIO_WRITE_ERROR;
	}
	return rc;
}

int gpio_read(GPIO* gpio, uint8_t *value)
{
	g_assert (gpio != NULL);
	char buf[1];
	int r = GPIO_OK;
	if (gpio->fd <= 0)
	{
		r = GPIO_ERROR;
	}
	else
	{
		if (lseek(gpio->fd, 0, SEEK_SET) == -1)
		{
			return GPIO_ERROR;
		}

		if (read(gpio->fd,&buf,1) != 1)
		{
			r = GPIO_READ_ERROR;
		} else {
			if (buf[0]=='1') {
				*value = 1;
			} else {
				*value = 0;
			}
		}
	}
	return r;
}
int gpio_clock_cycle(GPIO* gpio, int num_clks) {
	g_assert (gpio != NULL);
        int i=0;
	int r=GPIO_OK;
        for (i=0;i<num_clks;i++) {
                if (gpio_writec(gpio,'0') == -1) {
			r = GPIO_WRITE_ERROR;
			break;
		}
		if (gpio_writec(gpio,'1') == -1) {
			r = GPIO_WRITE_ERROR;
			break;
		}
        }
	return r;
}

// Gets the gpio device path from gpio manager object
int gpio_init(GDBusConnection *connection, GPIO* gpio)
{
	int rc = GPIO_OK;
	GDBusProxy *proxy;
	GError *error;
	GVariant *result;

	error = NULL;
	g_assert_no_error (error);
	error = NULL;
	proxy = g_dbus_proxy_new_sync (connection,
                                 G_DBUS_PROXY_FLAGS_NONE,
                                 NULL,                      /* GDBusInterfaceInfo */
                                 "org.openbmc.managers.System", /* name */
                                 "/org/openbmc/managers/System", /* object path */
                                 "org.openbmc.managers.System",        /* interface */
                                 NULL, /* GCancellable */
                                 &error);
	if (error != NULL) {
		return GPIO_LOOKUP_ERROR;
	}

	result = g_dbus_proxy_call_sync (proxy,
                                   "gpioInit",
                                   g_variant_new ("(s)", gpio->name),
                                   G_DBUS_CALL_FLAGS_NONE,
                                   -1,
                                   NULL,
                                   &error);

	if (error != NULL) {
		return GPIO_LOOKUP_ERROR;
	}
	g_assert (result != NULL);
	g_variant_get (result, "(&si&s)", &gpio->dev,&gpio->num,&gpio->direction);
	g_print("GPIO Lookup:  %s = %d,%s\n",gpio->name,gpio->num,gpio->direction);

	//export and set direction
	char dev[254];
	char data[4];
	int fd;
	do {
		struct stat st;

		sprintf(dev,"%s/gpio%d/value",gpio->dev,gpio->num);
		//check if gpio is exported, if not export
    		int result = stat(dev, &st);
    		if (result)
		{
			sprintf(dev,"%s/export",gpio->dev);
			fd = open(dev, O_WRONLY);
			if (fd == GPIO_ERROR) {
				rc = GPIO_OPEN_ERROR;
				break;
			}
			sprintf(data,"%d",gpio->num);
			rc = write(fd,data,strlen(data));
			close(fd);
			if (rc != strlen(data)) {
				rc = GPIO_WRITE_ERROR;
				break;
			}
		}
		const char* file = "edge";
		const char* direction = gpio->direction;
		if (strcmp(direction, "in") == 0)
		{
			file = "direction";
		}
		else if (strcmp(direction, "out") == 0)
		{
			file = "direction";

			// Read current value, so we can set 'high' or 'low'.
			// Setting direction directly to 'out' is the same as
			// setting to 'low' which can change the value in the
			// GPIO.
			uint8_t value = 0;
			rc = gpio_open(gpio);
			if (rc) break;
			rc = gpio_read(gpio, &value);
			if (rc) break;
			gpio_close(gpio);

			direction = (value ? "high" : "low");
		}
		sprintf(dev,"%s/gpio%d/%s",gpio->dev,gpio->num,file);
		fd = open(dev,O_WRONLY);
		if (fd == GPIO_ERROR) {
			rc = GPIO_WRITE_ERROR;
			break;
		}
		rc = write(fd,direction,strlen(direction));
		if (rc != strlen(direction)) {
			rc = GPIO_WRITE_ERROR;
			break;
		}

		close(fd);
		rc = GPIO_OK;
	} while(0);

	return rc;
}




char* get_gpio_dev(GPIO* gpio)
{
	char* buf;
	asprintf(&buf, "%s/gpio%d/value", gpio->dev, gpio->num);
	return buf;
}

int gpio_open_interrupt(GPIO* gpio, GIOFunc func, gpointer user_data)
{
	int rc = GPIO_OK;
	char buf[255];
	sprintf(buf, "%s/gpio%d/value", gpio->dev, gpio->num);
	gpio->fd = open(buf, O_RDONLY | O_NONBLOCK );
	gpio->irq_inited = false;
	if (gpio->fd == -1)
	{
		rc = GPIO_OPEN_ERROR;
	}
	else
	{
		GIOChannel* channel = g_io_channel_unix_new( gpio->fd);
		guint id = g_io_add_watch( channel, G_IO_PRI, func, user_data );
	}
	return rc;
}

int gpio_open(GPIO* gpio)
{
	g_assert (gpio != NULL);
	// open gpio for writing or reading
	char buf[254];
	int rc = 0;
	gpio->fd = -1;
	if (gpio->direction == NULL) {
		return GPIO_OPEN_ERROR;
	}
	if (strcmp(gpio->direction,"in")==0)
	{
		sprintf(buf, "%s/gpio%d/value", gpio->dev, gpio->num);
		gpio->fd = open(buf, O_RDONLY);
	}
	else
	{
		sprintf(buf, "%s/gpio%d/value", gpio->dev, gpio->num);
		gpio->fd = open(buf, O_RDWR);

	}
	if (gpio->fd == -1) {
		return GPIO_OPEN_ERROR;
	}
	return GPIO_OK;
}

void gpio_close(GPIO* gpio)
{
	close(gpio->fd);
}
