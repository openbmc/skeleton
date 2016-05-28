#include "interfaces/openbmc_intf.h"
#include "openbmc.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "gpio.h"

#define NUM_SLOTS 8
GPIO slots[NUM_SLOTS] = {
	{ "SLOT0_RISER_PRESENT" },
	{ "SLOT1_RISER_PRESENT" },
	{ "SLOT2_RISER_PRESENT" },
	{ "SLOT0_PRESENT" },
	{ "SLOT1_PRESENT" },
	{ "SLOT2_PRESENT" },
	{ "MEZZ0_PRESENT" },
	{ "MEZZ1_PRESENT" },
};

typedef struct {
	const char* bus_name;
	const char* path;
	const char* intf_name;
} object_info;



/* ------------------------------------------------------------------------- */
int
get_object(GDBusProxy *proxy, GPIO* gpio, object_info* obj_info)
{
	g_print("Checking Presence: %s\n",gpio->name);
	GError *error;
	GVariant *parm;
	GVariant *result;

	error = NULL;
	parm = g_variant_new("(ss)","GPIO_PRESENT",gpio->name);
	result = g_dbus_proxy_call_sync(proxy,
			"getObjectFromId",
			parm,
			G_DBUS_CALL_FLAGS_NONE,
			-1,
			NULL,
			&error);
	g_assert_no_error(error);

	GVariantIter *iter = g_variant_iter_new(result);
	GVariant* v_result = g_variant_iter_next_value(iter);

	g_variant_get(v_result,"(sss)",&obj_info->bus_name,&obj_info->path,&obj_info->intf_name);
	int rc=0;
	if(strlen(obj_info->bus_name) == 0) {
		rc = 1;
	}
	g_variant_unref(v_result);
	g_variant_unref(result);

	return rc;
}

int
get_presence(GDBusConnection* connection, GPIO* gpio, uint8_t* present)
{
	int rc = GPIO_OK;
	do {
		rc = gpio_init(connection,gpio);
		if(rc != GPIO_OK) { break; }
		uint8_t gpio_val;
		rc = gpio_open(gpio);
		if(rc != GPIO_OK) { break; }
		rc = gpio_read(gpio,&gpio_val);
		if(rc != GPIO_OK) { gpio_close(gpio); break; }
		gpio_close(gpio);
		*present = gpio_val;
	} while(0);
	if(rc != GPIO_OK)
	{
		printf("ERROR pcie_slot_present: GPIO error %s (rc=%d)\n",gpio->name,rc);
	}
	return rc;
}

void
update_fru_obj(GDBusConnection* connection, object_info* obj_info, const char* present)
{
	GDBusProxy *proxy;
	GError *error;
	GVariant *parm;

	error = NULL;
	proxy = g_dbus_proxy_new_sync(connection,
			G_DBUS_PROXY_FLAGS_NONE,
			NULL, /* GDBusInterfaceInfo* */
			obj_info->bus_name, /* name */
			obj_info->path, /* object path */
			obj_info->intf_name, /* interface name */
			NULL, /* GCancellable */
			&error);
	g_assert_no_error(error);

	error = NULL;
	parm = g_variant_new("(s)",present);

	g_dbus_proxy_call_sync(proxy,
			"setPresent",
			parm,
			G_DBUS_CALL_FLAGS_NONE,
			-1,
			NULL,
			&error);

	g_assert_no_error(error);
}

gint
main(gint argc, gchar *argv[])
{
	GMainLoop *loop;
	GDBusConnection *c;
	GDBusProxy *sys_proxy;
	GError *error;

	loop = g_main_loop_new(NULL, FALSE);

	error = NULL;
	c = g_bus_get_sync(DBUS_TYPE, NULL, &error);

	error = NULL;
	sys_proxy = g_dbus_proxy_new_sync(c,
			G_DBUS_PROXY_FLAGS_NONE,
			NULL, /* GDBusInterfaceInfo* */
			"org.openbmc.managers.System", /* name */
			"/org/openbmc/managers/System", /* object path */
			"org.openbmc.managers.System", /* interface name */
			NULL, /* GCancellable */
			&error);
	g_assert_no_error(error);

	int i = 0;
	int rc = 0;
	for(i=0;i<NUM_SLOTS;i++)
	{
		object_info obj_info;
		uint8_t present;
		do {
			rc = get_object(sys_proxy,&slots[i],&obj_info);
			if(rc) { break; }
			rc = get_presence(c,&slots[i],&present);
			//if (rc) { break; }
			// TODO: send correct state
			if(present == 0) {
				update_fru_obj(c,&obj_info,"True");
			} else {
				update_fru_obj(c,&obj_info,"False");
			}
		} while(0);
	}

	g_object_unref(c);
	g_main_loop_unref(loop);
	return 0;
}
