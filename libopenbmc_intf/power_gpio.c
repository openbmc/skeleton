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

#include "power_gpio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

gboolean read_power_gpio(GDBusConnection *connection, PowerGpio *power_gpio)
{
	GDBusProxy *proxy;
	GError *error;
	GVariant *value;
	gchar *power_good_in_name;
	GVariantIter *power_up_outs_iter;
	GVariantIter *reset_outs_iter;
	gchar *power_up_out_name;
	gchar *reset_out_name;
	gboolean power_up_polarity;
	gboolean reset_out_polarity;
	int i;

	proxy = g_dbus_proxy_new_sync(connection,
			G_DBUS_PROXY_FLAGS_NONE,
			NULL,							/* GDBusInterfaceInfo */
			"org.openbmc.managers.System",	/* name */
			"/org/openbmc/managers/System",	/* object path */
			"org.openbmc.managers.System",	/* interface */
			NULL,							/* GCancellable */
			&error);
	if(error != NULL) {
		fprintf(stderr, "Unable to create proxy: %s\n", error->message);
		g_error_free(error);
		return FALSE;
	}

	value = g_dbus_proxy_call_sync(proxy,
			"getPowerConfiguration",
			NULL,
			G_DBUS_CALL_FLAGS_NONE,
			-1,
			NULL,
			&error);
	if(error != NULL) {
		fprintf(stderr, "Power GPIO: call to getPowerConfiguration failed: %s\n",
				error->message);
		g_error_free(error);
		return FALSE;
	}

	g_assert(value != NULL);
	memset(power_gpio, 0, sizeof(*power_gpio));
	g_variant_get(value, "(&sa(sb)a(sb))", &power_good_in_name,
            &power_up_outs_iter, &reset_outs_iter);

	g_print("Power GPIO power good input %s\n", power_good_in_name);
	power_gpio->power_good_in.name = g_strdup(power_good_in_name);
	power_gpio->num_power_up_outs = g_variant_iter_n_children(
			power_up_outs_iter);
	g_print("Power GPIO %d power_up outputs\n",
			power_gpio->num_power_up_outs);
	power_gpio->power_up_outs = g_malloc0_n(power_gpio->num_power_up_outs,
			sizeof(GPIO));
	power_gpio->power_up_pols = g_malloc0_n(power_gpio->num_power_up_outs,
			sizeof(gboolean));
	for(i = 0; g_variant_iter_next(power_up_outs_iter, "(&sb)",
				&power_up_out_name, &power_up_polarity);
			i++) {
		g_print("Power GPIO power_up[%d] = %s active %s\n", i,
				power_up_out_name, power_up_polarity ? "HIGH" : "LOW");
		power_gpio->power_up_outs[i].name = g_strdup(power_up_out_name);
		power_gpio->power_up_pols[i] = power_up_polarity;
	}
	power_gpio->num_reset_outs = g_variant_iter_n_children(reset_outs_iter);
	g_print("Power GPIO %d reset outputs\n", power_gpio->num_reset_outs);
	power_gpio->reset_outs = g_malloc0_n(power_gpio->num_reset_outs, sizeof(GPIO));
	power_gpio->reset_pols = g_malloc0_n(power_gpio->num_reset_outs,
			sizeof(gboolean));
	for(i = 0; g_variant_iter_next(reset_outs_iter, "(&sb)", &reset_out_name,
				&reset_out_polarity); i++) {
		g_print("Power GPIO reset[%d] = %s active %s\n", i, reset_out_name,
				reset_out_polarity ? "HIGH" : "LOW");
		power_gpio->reset_outs[i].name = g_strdup(reset_out_name);
		power_gpio->reset_pols[i] = reset_out_polarity;
	}

	g_variant_iter_free(power_up_outs_iter);
	g_variant_iter_free(reset_outs_iter);
	g_variant_unref(value);

	return TRUE;
}

void free_power_gpio(PowerGpio *power_gpio) {
	int i;
	g_free(power_gpio->power_good_in.name);
	for(i = 0; i < power_gpio->num_power_up_outs; i++) {
		g_free(power_gpio->power_up_outs[i].name);
	}
	g_free(power_gpio->power_up_outs);
	g_free(power_gpio->power_up_pols);
	for(i = 0; i < power_gpio->num_reset_outs; i++) {
		g_free(power_gpio->reset_outs[i].name);
	}
	g_free(power_gpio->reset_outs);
	g_free(power_gpio->reset_pols);
}
