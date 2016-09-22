#include "power_gpio.h"

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

	error = NULL;
	g_assert_no_error (error);
	error = NULL;
	proxy = g_dbus_proxy_new_sync(connection,
			G_DBUS_PROXY_FLAGS_NONE,
			NULL,							/* GDBusInterfaceInfo */
			"org.openbmc.managers.System",	/* name */
			"/org/openbmc/managers/System",	/* object path */
			"org.openbmc.managers.System",	/* interface */
			NULL,							/* GCancellable */
			&error);
	if(error != NULL) {
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
		g_print("Power GPIO ERROR: failed to call getPowerConfiguration\n");
		return FALSE;
	}

	memset(power_gpio, 0, sizeof(*power_gpio));
	g_assert(value != NULL);
	g_variant_get(value, "(&sa(sb)a(sb))", &power_good_in_name,
            &power_up_outs_iter, &reset_outs_iter);

	g_print("Power GPIO power good input %s\n", power_good_in_name);
	power_gpio->power_good_in.name = strdup(power_good_in_name);
	power_gpio->num_power_up_outs = g_variant_iter_n_children(
			power_up_outs_iter);
	g_print("Power GPIO %d power_up outputs\n",
			power_gpio->num_power_up_outs);
	power_gpio->power_up_outs = calloc(power_gpio->num_power_up_outs,
			sizeof(GPIO));
	power_gpio->power_up_pols = calloc(power_gpio->num_power_up_outs,
			sizeof(gboolean));
	for(i = 0; g_variant_iter_next(power_up_outs_iter, "(&sb)",
				&power_up_out_name, &power_up_polarity);
			i++) {
		g_print("Power GPIO power_up[%d] = %s active %s\n", i,
				power_up_out_name, power_up_polarity ? "HIGH" : "LOW");
		power_gpio->power_up_outs[i].name = strdup(power_up_out_name);
		power_gpio->power_up_pols[i] = power_up_polarity;
	}
	power_gpio->num_reset_outs = g_variant_iter_n_children(reset_outs_iter);
	g_print("Power GPIO %d reset outputs\n", power_gpio->num_reset_outs);
	power_gpio->reset_outs = calloc(power_gpio->num_reset_outs, sizeof(GPIO));
	power_gpio->reset_pols = calloc(power_gpio->num_reset_outs,
			sizeof(gboolean));
	for(i = 0; g_variant_iter_next(reset_outs_iter, "(&sb)", &reset_out_name,
				&reset_out_polarity); i++) {
		g_print("Power GPIO reset[%d] = %s active %s\n", i, reset_out_name,
				reset_out_polarity ? "HIGH" : "LOW");
		power_gpio->reset_outs[i].name = strdup(reset_out_name);
		power_gpio->reset_pols[i] = reset_out_polarity;
	}

	g_variant_iter_free(power_up_outs_iter);
	g_variant_iter_free(reset_outs_iter);
	g_variant_unref(value);

	return TRUE;
}

void free_power_gpio(PowerGpio *power_gpio) {
	int i;
	free(power_gpio->power_good_in.name);
	for(i = 0; i < power_gpio->num_power_up_outs; i++) {
		free(power_gpio->power_up_outs[i].name);
	}
	free(power_gpio->power_up_outs);
	free(power_gpio->power_up_pols);
	for(i = 0; i < power_gpio->num_reset_outs; i++) {
		free(power_gpio->reset_outs[i].name);
	}
	free(power_gpio->reset_outs);
	free(power_gpio->reset_pols);
}
