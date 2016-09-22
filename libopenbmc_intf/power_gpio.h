/**
 * Copyright © 2016 Google Inc.
 * Copyright © 2016 IBM Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
	/* TRUE for active high */
	gboolean *power_up_pols;
	/* Selectable polarity pins holding system complexes in reset. */
	size_t num_reset_outs;
	GPIO *reset_outs;
	/* TRUE for active high */
	gboolean *reset_pols;
} PowerGpio;

/* Read system configuration for power GPIOs. */
gboolean read_power_gpio(GDBusConnection *connection, PowerGpio *power_gpio);
/* Frees internal buffers. Does not free parameter. Does not close GPIOs. */
void free_power_gpio(PowerGpio *power_gpio);

#endif
