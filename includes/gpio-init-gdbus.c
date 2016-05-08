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


// Gets the gpio device path from gpio manager object
int gpio_init(void *connection, GPIO* gpio)
{
	int rc = GPIO_OK;
	GDBusProxy *proxy;
	GError *error;
	GVariant *result;

	error = NULL;
	g_assert_no_error (error);
	error = NULL;
	proxy = g_dbus_proxy_new_sync ((GDBusConnection*)connection,
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
		if (strcmp(gpio->direction,"in")==0 || strcmp(gpio->direction,"out")==0)
		{
			file = "direction";
		}
		sprintf(dev,"%s/gpio%d/%s",gpio->dev,gpio->num,file);
		fd = open(dev,O_WRONLY);
		if (fd == GPIO_ERROR) {
			rc = GPIO_WRITE_ERROR;
			break;
		}
		rc = write(fd,gpio->direction,strlen(gpio->direction));
		if (rc != strlen(gpio->direction)) {
			rc = GPIO_WRITE_ERROR;
			break;
		}

		close(fd);
		rc = GPIO_OK;
	} while(0);

	return rc;
}
