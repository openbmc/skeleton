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


int gpio_writec(GPIO* gpio, char value)
{
	g_assert (gpio != NULL);
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
	g_assert (gpio != NULL);
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

char* get_gpio_dev(GPIO* gpio)
{
	char* buf;
	sprintf(buf, "%s/gpio%d/value", gpio->dev, gpio->num);
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
