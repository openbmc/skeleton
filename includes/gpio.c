#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <argp.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "gpio.h"


void gpio_writec(GPIO* gpio, char value)
{
	char buf[1];
	buf[0] = value;
	if (write(gpio->fd, buf, 1) != 1)
	{
		//TODO: error handling
		printf("Write error\n");
	} 
}

void gpio_write(GPIO* gpio, uint8_t value)
{
	char buf[1];
	buf[0] = '0';
	if (value==1)
	{
		buf[0]='1';
	} 
	if (write(gpio->fd, buf, 1) != 1)
	{
		//TODO: error handling
		printf("write error\n");
	} 
}

uint8_t gpio_read(GPIO* gpio)
{
	char buf[1];
	if (read(gpio->fd,&buf,1) != 1)
	{
		//TODO: error hjandling
		printf("read error\n");
	}
	if (buf[0]=='1') {
		return 1;
	}
	return 0;	

}
void gpio_clock_cycle(GPIO* gpio, int num_clks) {
        int i=0;
        for (i=0;i<num_clks;i++) {
                gpio_writec(gpio,'0');
                gpio_writec(gpio,'1');
        }
}

// Gets the gpio device path from gpio manager object
void gpio_init(GDBusConnection *connection, GPIO* gpio)
{
	GDBusProxy *proxy;
	GError *error;
	GVariant *result;

	error = NULL;
	g_assert_no_error (error);
	error = NULL;
	proxy = g_dbus_proxy_new_sync (connection,
                                 G_DBUS_PROXY_FLAGS_NONE,
                                 NULL,                      /* GDBusInterfaceInfo */
                                 "org.openbmc.managers.Gpios", /* name */
                                 "/org/openbmc/managers/Gpios", /* object path */
                                 "org.openbmc.managers.Gpios",        /* interface */
                                 NULL, /* GCancellable */
                                 &error);
	g_assert_no_error (error);

	result = g_dbus_proxy_call_sync (proxy,
                                   "init",
                                   g_variant_new ("(s)", gpio->name),
                                   G_DBUS_CALL_FLAGS_NONE,
                                   -1,
                                   NULL,
                                   &error);
  
	g_assert_no_error (error);
	g_assert (result != NULL);
	g_variant_get (result, "(&si&s)", &gpio->dev,&gpio->num,&gpio->direction);
	g_print("GPIO Lookup:  %s = %d,%s\n",gpio->name,gpio->num,gpio->direction);
	
	//export and set direction
	char dev[254];
	char data[4];
	sprintf(dev,"%s/export",gpio->dev);
	int fd = open(dev, O_WRONLY);
	sprintf(data,"%d",gpio->num);
	write(fd,data,strlen(data));
	close(fd);

	sprintf(dev,"%s/gpio%d/direction",gpio->dev,gpio->num);
	fd = open(dev,O_WRONLY);
	write(fd,gpio->direction,strlen(gpio->direction));
	close(fd);


}
int gpio_open(GPIO* gpio)
{
	// open gpio for writing or reading
	char buf[254];
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
	if (gpio->fd == -1)
	{
		printf("error opening: %s\n",buf);
	}
	return gpio->fd;
}

void gpio_close(GPIO* gpio)
{
	close(gpio->fd);
}
