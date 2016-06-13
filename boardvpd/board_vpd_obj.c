#include "interfaces/openbmc_intf.h"
#include "openbmc.h"


/* ------------------------------------------------------------------------- */

gint
main(gint argc, gchar *argv[])
{
	GMainLoop *loop;
	GDBusConnection *c;
	GDBusProxy *p;
	GError *error;
	GVariant *parm;

	loop = g_main_loop_new(NULL, FALSE);

	error = NULL;
	c = g_bus_get_sync(DBUS_TYPE, NULL, &error);

	error = NULL;
	p = g_dbus_proxy_new_sync(c,
			G_DBUS_PROXY_FLAGS_NONE,
			NULL, /* GDBusInterfaceInfo* */
			"org.openbmc.managers.Inventory", /* name */
			"/org/openbmc/inventory/items/system/io_board", /* object path */
			"org.openbmc.InventoryItem", /* interface name */
			NULL, /* GCancellable */
			&error);
	g_assert_no_error(error);

	//TODO: Read actual vpd
	g_print("Reading VPD\n");
	GVariantBuilder *b;
	GVariant *dict;

	b = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));
	g_variant_builder_add(b, "{sv}", "manufacturer", g_variant_new_string("ibm"));
	g_variant_builder_add(b, "{sv}", "part_num", g_variant_new_string("3N0001"));
	dict = g_variant_builder_end(b);

	//proxy_call wants parm as an array
	parm = g_variant_new("(v)",dict);

	error = NULL;
	g_dbus_proxy_call_sync(p,
			"update",
			parm,
			G_DBUS_CALL_FLAGS_NONE,
			-1,
			NULL,
			&error);
	g_assert_no_error(error);

	g_object_unref(p);
	g_object_unref(c);
	g_main_loop_unref(loop);
	return 0;
}
