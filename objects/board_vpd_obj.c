#include "interfaces/openbmc_intf.h"
#include "openbmc.h"


/* ---------------------------------------------------------------------------------------------------- */

gint
main (gint argc, gchar *argv[])
{
	GMainLoop *loop;
	GDBusConnection *c;
	GDBusProxy *p;
 	GError *error;
	GVariant *parm;
	GVariant *result;

	loop = g_main_loop_new (NULL, FALSE);

	error = NULL;
	c = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, &error);

	error = NULL;
	p = g_dbus_proxy_new_sync (c,
                             G_DBUS_PROXY_FLAGS_NONE,
                             NULL,                      /* GDBusInterfaceInfo* */
                             "org.openbmc.managers.Frus", /* name */
                             "/org/openbmc/managers/Frus", /* object path */
                             "org.openbmc.managers.Frus",        /* interface name */
                             NULL,                      /* GCancellable */
                             &error);
	g_assert_no_error (error);
	parm = g_variant_new("(isv)",21,"manufacturer",g_variant_new_string("ibmibm"));
	result = g_dbus_proxy_call_sync (p,
                                   "updateFruField",
				   parm,
                                   G_DBUS_CALL_FLAGS_NONE,
                                   -1,
                                   NULL,
                                   &error);
	g_assert_no_error (error);
	//g_main_loop_run (loop);
	//g_bus_unown_name (id);
 	g_main_loop_unref (loop);
 	return 0;
}
