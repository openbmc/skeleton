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
#include <systemd/sd-bus.h>

// Gets the gpio device path from gpio manager object
int gpio_init(void *connection, GPIO* gpio)
{
	int rc = GPIO_OK;
	sd_bus_error err    = SD_BUS_ERROR_NULL;
	sd_bus_message *res = NULL;

	sd_bus_error_free(&err);
	sd_bus_message_unref(res);

	rc = sd_bus_call_method ((sd_bus*)connection,
							"org.openbmc.managers.System",  /* name */
							"/org/openbmc/managers/System", /* object path */
							"org.openbmc.managers.System",  /* interface */
							"gpioInit",
							&err,
							&res,
							"s",
							gpio->name);
                             
	if(rc < 0)
	{
		fprintf(stderr, "Failed to init gpio for %s : %s\n", gpio->name, err.message);
		return -1;
	}
    
	rc = sd_bus_message_read (res, "sis", &gpio->dev, &gpio->num, &gpio->direction);
	if (rc < 0)
		return -1;
	else
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
