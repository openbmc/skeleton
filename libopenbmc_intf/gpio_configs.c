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

#include "gpio_configs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

gboolean read_gpios(GDBusConnection *connection, GpioConfigs *gpios)
{
	GDBusProxy *proxy;
	GError *error = NULL;
	GVariant *value;
	gchar *power_good_in_name;
	gchar *latch_out_name;
	GVariantIter *power_up_outs_iter;
	GVariantIter *reset_outs_iter;
	GVariantIter *pci_reset_outs_iter;
	gchar *power_up_out_name;
	gchar *reset_out_name;
	gchar *pci_reset_out_name;
	gboolean power_up_polarity;
	gboolean reset_out_polarity;
	gboolean pci_reset_out_polarity;
	gboolean pci_reset_out_hold;

	gchar *fsi_data_name;
	gchar *fsi_clk_name;
	gchar *fsi_enable_name;
	gchar *cronus_sel_name;
	GVariantIter *optionals_iter;
	gchar *optional_name;
	gboolean optional_polarity;
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
			"getGpioConfiguration",
			NULL,
			G_DBUS_CALL_FLAGS_NONE,
			-1,
			NULL,
			&error);
	if(error != NULL) {
		fprintf(stderr, "Power GPIO: call to getGpioConfiguration failed: %s\n",
				error->message);
		g_error_free(error);
		return FALSE;
	}

	g_assert(value != NULL);
	memset(gpios, 0, sizeof(*gpios));
	g_variant_get(
		value, "(&s&sa(sb)a(sb)a(sbb)&s&s&s&sa(sb))",
		&power_good_in_name, &latch_out_name,
		&power_up_outs_iter, &reset_outs_iter, &pci_reset_outs_iter,
		&fsi_data_name, &fsi_clk_name,
		&fsi_enable_name, &cronus_sel_name,
		&optionals_iter);

	g_print("Power GPIO latch output: %s\n", latch_out_name);
	if(*latch_out_name != '\0') {  /* latch is optional */
		gpios->power_gpio.latch_out.name = strdup(latch_out_name);
	}
	g_print("Power GPIO power good input: %s\n", power_good_in_name);
	gpios->power_gpio.power_good_in.name = g_strdup(power_good_in_name);
	gpios->power_gpio.num_power_up_outs = g_variant_iter_n_children(
			power_up_outs_iter);
	g_print("Power GPIO %zu power_up outputs\n",
			gpios->power_gpio.num_power_up_outs);
	gpios->power_gpio.power_up_outs = g_malloc0_n(gpios->power_gpio.num_power_up_outs,
			sizeof(GPIO));
	gpios->power_gpio.power_up_pols = g_malloc0_n(gpios->power_gpio.num_power_up_outs,
			sizeof(gboolean));
	for(i = 0; g_variant_iter_next(power_up_outs_iter, "(&sb)",
				&power_up_out_name, &power_up_polarity);
			i++) {
		g_print("Power GPIO power_up[%d] = %s active %s\n", i,
				power_up_out_name, power_up_polarity ? "HIGH" : "LOW");
		gpios->power_gpio.power_up_outs[i].name = g_strdup(power_up_out_name);
		gpios->power_gpio.power_up_pols[i] = power_up_polarity;
	}
	gpios->power_gpio.num_reset_outs = g_variant_iter_n_children(reset_outs_iter);
	g_print("Power GPIO %zu reset outputs\n", gpios->power_gpio.num_reset_outs);
	gpios->power_gpio.reset_outs = g_malloc0_n(gpios->power_gpio.num_reset_outs, sizeof(GPIO));
	gpios->power_gpio.reset_pols = g_malloc0_n(gpios->power_gpio.num_reset_outs,
			sizeof(gboolean));
	for(i = 0; g_variant_iter_next(reset_outs_iter, "(&sb)", &reset_out_name,
				&reset_out_polarity); i++) {
		g_print("Power GPIO reset[%d] = %s active %s\n", i, reset_out_name,
				reset_out_polarity ? "HIGH" : "LOW");
		gpios->power_gpio.reset_outs[i].name = g_strdup(reset_out_name);
		gpios->power_gpio.reset_pols[i] = reset_out_polarity;
	}

	gpios->power_gpio.num_pci_reset_outs = g_variant_iter_n_children(pci_reset_outs_iter);
	g_print("Power GPIO %d pci reset outputs\n", gpios->power_gpio.num_pci_reset_outs);
	gpios->power_gpio.pci_reset_outs = g_malloc0_n(gpios->power_gpio.num_pci_reset_outs,
			sizeof(GPIO));
	gpios->power_gpio.pci_reset_pols = g_malloc0_n(gpios->power_gpio.num_pci_reset_outs,
			sizeof(gboolean));
	gpios->power_gpio.pci_reset_holds = g_malloc0_n(gpios->power_gpio.num_pci_reset_outs,
			sizeof(gboolean));
	for(i = 0; g_variant_iter_next(pci_reset_outs_iter, "(&sbb)", &pci_reset_out_name,
				&pci_reset_out_polarity, &pci_reset_out_hold); i++) {
		g_print("Power GPIO pci reset[%d] = %s active %s, hold - %s\n", i,
				pci_reset_out_name,
				pci_reset_out_polarity ? "HIGH" : "LOW",
				pci_reset_out_hold ? "Yes" : "No");
		gpios->power_gpio.pci_reset_outs[i].name = g_strdup(pci_reset_out_name);
		gpios->power_gpio.pci_reset_pols[i] = pci_reset_out_polarity;
		gpios->power_gpio.pci_reset_holds[i] = pci_reset_out_hold;
	}


	g_print("FSI DATA GPIO: %s\n", fsi_data_name);
	gpios->hostctl_gpio.fsi_data.name = strdup(fsi_data_name);

	g_print("FSI CLK GPIO: %s\n", fsi_clk_name);
	gpios->hostctl_gpio.fsi_clk.name = strdup(fsi_clk_name);

	g_print("FSI ENABLE GPIO: %s\n", fsi_enable_name);
	gpios->hostctl_gpio.fsi_enable.name = strdup(fsi_enable_name);

	g_print("CRONUS SEL GPIO: %s\n", cronus_sel_name);
	gpios->hostctl_gpio.cronus_sel.name = strdup(cronus_sel_name);

	gpios->hostctl_gpio.num_optionals = g_variant_iter_n_children(optionals_iter);
	g_print("Hostctl GPIO optionals: %zu\n", gpios->hostctl_gpio.num_optionals);
	gpios->hostctl_gpio.optionals = g_malloc0_n(gpios->hostctl_gpio.num_optionals, sizeof(GPIO));
	gpios->hostctl_gpio.optional_pols = g_malloc0_n(gpios->hostctl_gpio.num_optionals, sizeof(gboolean));
	for (i = 0; g_variant_iter_next(optionals_iter, "(&sb)", &optional_name, &optional_polarity); ++i) {
		g_print("Hostctl optional GPIO[%d] = %s active %s\n", i, optional_name, optional_polarity ? "HIGH" : "LOW");
		gpios->hostctl_gpio.optionals[i].name = g_strdup(optional_name);
		gpios->hostctl_gpio.optional_pols[i] = optional_polarity;
	}

	g_variant_iter_free(power_up_outs_iter);
	g_variant_iter_free(reset_outs_iter);
	g_variant_iter_free(pci_reset_outs_iter);
	g_variant_iter_free(optionals_iter);
	g_variant_unref(value);

	return TRUE;
}

void free_gpios(GpioConfigs *gpios) {
	int i;
	g_free(gpios->power_gpio.latch_out.name);
	g_free(gpios->power_gpio.power_good_in.name);
	for(i = 0; i < gpios->power_gpio.num_power_up_outs; i++) {
		g_free(gpios->power_gpio.power_up_outs[i].name);
	}
	g_free(gpios->power_gpio.power_up_outs);
	g_free(gpios->power_gpio.power_up_pols);
	for(i = 0; i < gpios->power_gpio.num_reset_outs; i++) {
		g_free(gpios->power_gpio.reset_outs[i].name);
	}
	g_free(gpios->power_gpio.reset_outs);
	g_free(gpios->power_gpio.reset_pols);
	for(i = 0; i < gpios->power_gpio.num_pci_reset_outs; i++) {
		g_free(gpios->power_gpio.pci_reset_outs[i].name);
	}
	g_free(gpios->power_gpio.pci_reset_outs);
	g_free(gpios->power_gpio.pci_reset_pols);
	g_free(gpios->power_gpio.pci_reset_holds);

	g_free(gpios->hostctl_gpio.fsi_data.name);
	g_free(gpios->hostctl_gpio.fsi_clk.name);
	g_free(gpios->hostctl_gpio.fsi_enable.name);
	g_free(gpios->hostctl_gpio.cronus_sel.name);
	for (i = 0; i < gpios->hostctl_gpio.num_optionals; ++i) {
		g_free(gpios->hostctl_gpio.optionals[i].name);
	}
}
