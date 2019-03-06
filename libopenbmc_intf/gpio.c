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
#include <dirent.h>
#include "openbmc_intf.h"
#include "gpio.h"
#include "gpio_json.h"

#include <sys/ioctl.h>
#include <linux/gpio.h>

#define GPIO_PORT_OFFSET 8
#define GPIO_BASE_PATH "/sys/class/gpio"

cJSON* gpio_json = NULL;

int gpio_write(GPIO* gpio, uint8_t value)
{
	g_assert (gpio != NULL);
	struct gpiohandle_data data;
	memset(&data, 0, sizeof(data));
	data.values[0] = value;

	if (gpio->fd <= 0)
	{
		return GPIO_ERROR;
	}

	if (ioctl(gpio->fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data) < 0)
	{
		return GPIO_WRITE_ERROR;
	}

	return GPIO_OK;
}

int gpio_read(GPIO* gpio, uint8_t *value)
{
	g_assert (gpio != NULL);
	struct gpiohandle_data data;
	memset(&data, 0, sizeof(data));

	if (gpio->fd <= 0)
	{
		return GPIO_ERROR;
	}

	if (ioctl(gpio->fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data) < 0)
	{
		return GPIO_READ_ERROR;
	}

	*value = data.values[0];

	return GPIO_OK;
}

/**
 * Determine the GPIO base number for the system.  It is found in
 * the 'base' file in the /sys/class/gpio/gpiochipX/ directory where the
 * /sys/class/gpio/gpiochipX/label file has a '1e780000.gpio' in it.
 *
 * Note: This method is ASPEED specific.  Could add support for
 * additional SOCs in the future.
 *
 * @return int - the GPIO base number, or < 0 if not found
 */
int get_gpio_base()
{
	int gpio_base = -1;

	DIR* dir = opendir(GPIO_BASE_PATH);
	if (dir == NULL)
	{
		fprintf(stderr, "Unable to open directory %s\n",
				GPIO_BASE_PATH);
		return -1;
	}

	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL)
	{
		/* Look in the gpiochip<X> directories for a file called 'label' */
		/* that contains '1e780000.gpio', then in that directory read */
		/* the GPIO base out of the 'base' file. */

		if (strncmp(entry->d_name, "gpiochip", 8) != 0)
		{
			continue;
		}

		gboolean is_bmc = FALSE;
		char* label_name;
		asprintf(&label_name, "%s/%s/label",
				GPIO_BASE_PATH, entry->d_name);

		FILE* fd = fopen(label_name, "r");
		free(label_name);

		if (!fd)
		{
			continue;
		}

		char label[14];
		if (fgets(label, 14, fd) != NULL)
		{
			if (strcmp(label, "1e780000.gpio") == 0)
			{
				is_bmc = TRUE;
			}
		}
		fclose(fd);

		if (!is_bmc)
		{
			continue;
		}

		char* base_name;
		asprintf(&base_name, "%s/%s/base",
				GPIO_BASE_PATH, entry->d_name);

		fd = fopen(base_name, "r");
		free(base_name);

		if (!fd)
		{
			continue;
		}

		if (fscanf(fd, "%d", &gpio_base) != 1)
		{
			gpio_base = -1;
		}
		fclose(fd);

		/* We found the right file. No need to continue. */
		break;
	}
	closedir(dir);

	if (gpio_base == -1)
	{
		fprintf(stderr, "Could not find GPIO base\n");
	}

	return gpio_base;
}

/**
 * Converts the GPIO port/offset nomenclature value
 * to a number.  Supports the ASPEED method where
 * num = base + (port * 8) + offset, and the port is an
 * A-Z character that converts to 0 to 25.  The base
 * is obtained form the hardware.
 *
 * For example: "A5" -> 5,  "Z7" -> 207
 *
 * @param[in] gpio - the GPIO name
 *
 * @return int - the GPIO number or < 0 if not found
 */
int convert_gpio_to_num(const char* gpio)
{
	size_t len = strlen(gpio);
	if (len < 2)
	{
		fprintf(stderr, ("Invalid GPIO name %s\n", gpio));
		return -1;
	}

	/* Read the offset from the last character */
	if (!isdigit(gpio[len-1]))
	{
		fprintf(stderr, "Invalid GPIO offset in GPIO %s\n", gpio);
		return -1;
	}

	int offset = gpio[len-1] - '0';

	/* Read the port from the second to last character */
	if (!isalpha(gpio[len-2]))
	{
		fprintf(stderr, "Invalid GPIO port in GPIO %s\n", gpio);
		return -1;
	}
	int port = toupper(gpio[len-2]) - 'A';

	/* Check for a 2 character port, like AA */
	if ((len == 3) && isalpha(gpio[len-3]))
	{
		port += 26 * (toupper(gpio[len-3]) - 'A' + 1);
	}

	return (port * GPIO_PORT_OFFSET) + offset;
}

/**
 * Returns the cJSON pointer to the GPIO definition
 * for the GPIO passed in.
 *
 * @param[in] gpio_name - the GPIO name, like BMC_POWER_UP
 *
 * @return cJSON* - pointer to the cJSON object or NULL
 *				  if not found.
 */
cJSON* get_gpio_def(const char* gpio_name)
{
	if (gpio_json == NULL)
	{
		gpio_json = load_json();
		if (gpio_json == NULL)
		{
			return NULL;
		}
	}

	cJSON* gpio_defs = cJSON_GetObjectItem(gpio_json, "gpio_definitions");
	g_assert(gpio_defs != NULL);

	cJSON* def;
	cJSON_ArrayForEach(def, gpio_defs)
	{
		cJSON* name = cJSON_GetObjectItem(def, "name");
		g_assert(name != NULL);

		if (strcmp(name->valuestring, gpio_name) == 0)
		{
			return def;
		}
	}
	return NULL;
}

/**
 * Frees the gpio_json memory
 *
 * Can be called once when callers are done calling making calls
 * to gpio_init() so that the JSON only needs to be loaded once.
 */
void gpio_inits_done()
{
	if (gpio_json != NULL)
	{
		cJSON_Delete(gpio_json);
		gpio_json = NULL;
	}
}

/**
 * Fills in the dev, direction, and num elements in
 * the GPIO structure.
 *
 * @param gpio - the GPIO structure to fill in
 *
 * @return GPIO_OK if successful
 */
int gpio_get_params(GPIO* gpio)
{
	gpio->dev = g_strdup(GPIO_BASE_PATH);

	const cJSON* def = get_gpio_def(gpio->name);
	if (def == NULL)
	{
		fprintf(stderr, "Unable to find GPIO %s in the JSON\n",
				gpio->name);
		return GPIO_LOOKUP_ERROR;
	}

	const cJSON* dir = cJSON_GetObjectItem(def, "direction");
	g_assert(dir != NULL);
	gpio->direction = g_strdup(dir->valuestring);

	/* Must use either 'num', like 87, or 'pin', like "A5" */
	const cJSON* num = cJSON_GetObjectItem(def, "num");
	if ((num != NULL) && cJSON_IsNumber(num))
	{
		gpio->num = num->valueint;
	}
	else
	{
		const cJSON* pin = cJSON_GetObjectItem(def, "pin");
		g_assert(pin != NULL);

		gpio->num = convert_gpio_to_num(pin->valuestring);
		if (gpio->num < 0)
		{
			return GPIO_LOOKUP_ERROR;
		}
	}
	// TODO: For thie purposes of skeleton and the projects that use it,
	// it should be safe to assume this will always be 0. Eventually skeleton
	// should be going away, but if the need arises before then this may need
	// to be updated to handle non-zero cases.
	gpio->chip_id = 0;
	return GPIO_OK;
}

int gpio_open(GPIO* gpio)
{
	g_assert (gpio != NULL);

	char buf[255];
	sprintf(buf, "/dev/gpiochip%d", gpio->chip_id);
	gpio->fd = open(buf, 0);
	if (gpio->fd == -1)
	{
		return GPIO_OPEN_ERROR;
	}

	struct gpiohandle_request req;
	memset(&req, 0, sizeof(req));
	strncpy(req.consumer_label, "skeleton-gpio",  sizeof(req.consumer_label));

	// open gpio for writing or reading
	if (gpio->direction == NULL)
	{
		gpio_close(gpio);
		return GPIO_OPEN_ERROR;
	}
	req.flags = (strcmp(gpio->direction,"in") == 0) ? GPIOHANDLE_REQUEST_INPUT
								: GPIOHANDLE_REQUEST_OUTPUT;

	req.lineoffsets[0] = gpio->num;
	req.lines = 1;

	if (strcmp(gpio->direction,"out") == 0)
	{
		// Not sure this is right to assume it is always high on output...
		req.default_values[0] = 1;
	}

	int rc = ioctl(gpio->fd, GPIO_GET_LINEHANDLE_IOCTL, &req);
	if (rc < 0)
	{
		gpio_close(gpio);
		return GPIO_OPEN_ERROR;
	}
	gpio_close(gpio);
	gpio->fd = req.fd;

	return GPIO_OK;
}

void gpio_close(GPIO* gpio)
{
	if(gpio->fd < 0)
		return;

	close(gpio->fd);
	gpio->fd = -1;
}
