#ifndef __POWER_GPIO_H__
#define __POWER_GPIO_H__

#include <stddef.h>
#include <glib.h>
#include "gpio.h"

typedef struct PowerGpio {
	/* Active high pin that is asserted following successful host power up. */
	GPIO power_good_in;
	/* Selectable polarity pins enabling host power rails. */
	size_t num_power_up_outs;
	GPIO *power_up_outs;
	gboolean *power_up_pols;  /* TRUE for active high */
	/* Selectable polarity pins holding system complexes in reset. */
	size_t num_reset_outs;
	GPIO *reset_outs;
	gboolean *reset_pols;  /* TRUE for active high */
} PowerGpio;

/* Read system configuration for power GPIOs. */
gboolean read_power_gpio(GDBusConnection *connection, PowerGpio *power_gpio);
/* Frees internal buffers. Does not free parameter. Does not close GPIOs. */
void free_power_gpio(PowerGpio *power_gpio);

#endif
