#include <openbmc_intf.h>
#include <openbmc.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <gpio.h>

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
get_presence(GPIO* gpio, uint8_t* present)
{
	int rc = GPIO_OK;
	do {
		rc = gpio_init(gpio);
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
	gpio_inits_done();
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
	GError *error;

	loop = g_main_loop_new(NULL, FALSE);

	error = NULL;
	c = g_bus_get_sync(DBUS_TYPE, NULL, &error);
	if(error)
		goto exit;

	int i = 0;
	int rc = 0;
	for(i=0;i<NUM_SLOTS;i++)
	{
		object_info obj_info;
		uint8_t present;
		do {
			rc = get_presence(&slots[i],&present);
			if (rc) { break; }
			// TODO: send correct state
			if(present == 0) {
				update_fru_obj(c,&obj_info,"True");
			} else {
				update_fru_obj(c,&obj_info,"False");
			}
		} while(0);
	}

exit:
	g_object_unref(c);
	g_main_loop_unref(loop);
	return 0;
}
