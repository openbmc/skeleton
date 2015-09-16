#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <argp.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "interfaces/openbmc_intf.h"
#include "gpio.h"


int gpio_writec(GPIO* gpio, char value)
{
	int rc = GPIO_OK;
	char buf[1];
	buf[0] = value;
	if (write(gpio->fd, buf, 1) != 1)
	{
		rc = GPIO_WRITE_ERROR;
	}
	return rc;
}

int gpio_write(GPIO* gpio, uint8_t value)
{
	int rc = GPIO_OK;
	char buf[1];
	buf[0] = '0';
	if (value==1)
	{
		buf[0]='1';
	} 
	if (write(gpio->fd, buf, 1) != 1)
	{
		rc = GPIO_WRITE_ERROR;
	}
	return rc;
}

int gpio_read(GPIO* gpio, uint8_t* value)
{
	char buf[1];
	int r = GPIO_OK;
	if (gpio->fd <= 0)
	{
		r = GPIO_ERROR;	
	}
	else
	{
		if (read(gpio->fd,&buf,1) != 1)
		{
			g_print("here1\n");
			r = GPIO_READ_ERROR;
		} else {
			g_print("here2\n");
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
	sprintf(dev,"%s/export",gpio->dev);
	do {
		int fd = open(dev, O_WRONLY);
		if (fd == GPIO_ERROR) {
			rc = GPIO_OPEN_ERROR;
			break;
		} 
		sprintf(data,"%d",gpio->num);
		rc = write(fd,data,strlen(data));
		close(fd);
		if (rc == GPIO_ERROR) {
			rc = GPIO_WRITE_ERROR;
			break;
		}

		sprintf(dev,"%s/gpio%d/direction",gpio->dev,gpio->num);
		fd = open(dev,O_WRONLY);
		if (fd == GPIO_ERROR) {
			rc = GPIO_WRITE_ERROR;
			break;
		}
		rc = write(fd,gpio->direction,strlen(gpio->direction));
		close(fd);
	} while(0);

	return rc;
}
char* get_gpio_dev(GPIO* gpio)
{
	char* buf;
	sprintf(buf, "%s/gpio%d/value", gpio->dev, gpio->num);
	return buf;
}

int gpio_open(GPIO* gpio)
{
	// open gpio for writing or reading
	char buf[254];
	int rc = 0;
	if (strcmp(gpio->direction,"in")==0)
	{
		sprintf(buf, "%s/gpio%d/value", gpio->dev, gpio->num);
		gpio->fd = open(buf, O_RDONLY);
	}
	else
	{
		sprintf(buf, "%s/gpio%d/value", gpio->dev, gpio->num);
		gpio->fd = open(buf, O_WRONLY);

	}
	return gpio->fd;
}

void gpio_close(GPIO* gpio)
{
	close(gpio->fd);
}
