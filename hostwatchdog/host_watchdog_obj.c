#include "interfaces/openbmc_intf.h"
#include "openbmc.h"

/* ------------------------------------------------------------------------- */

static const gchar* dbus_object_path = "/org/openbmc/watchdog";
static const gchar* instance_name = "host0";
static const gchar* dbus_name = "org.openbmc.watchdog.Host";

static GDBusObjectManagerServer *manager = NULL;
static guint watchdogid = 0;

static gboolean
poll_watchdog(gpointer user_data)
{
	Watchdog *watchdog = object_get_watchdog((Object*)user_data);

	guint count = watchdog_get_watchdog(watchdog);
	g_print("Polling watchdog: %d\n",count);

	if(count == 0)
	{
		//watchdog error, emit error and stop watchdog
		watchdogid = 0;
		watchdog_emit_watchdog_error(watchdog);
		return FALSE;
	}

	//reset watchdog
	watchdog_set_watchdog(watchdog,0);
	return TRUE;
}

static gboolean
remove_watchdog(void)
{
	if(watchdogid)
	{
		g_source_remove(watchdogid);
		watchdogid = 0;
	}
	return TRUE;
}

static gboolean
set_poll_interval(Watchdog *wd,
		GDBusMethodInvocation *invocation,
		guint interval,
		gpointer user_data)
{
	g_print("Setting watchdog poll interval to: %d\n", interval);
	watchdog_set_poll_interval(wd, interval);
	watchdog_complete_set(wd,invocation);
	return TRUE;
}

static gboolean
on_start(Watchdog *wd,
		GDBusMethodInvocation *invocation,
		gpointer user_data)
{
	remove_watchdog();
	watchdog_set_watchdog(wd,0);
	guint poll_interval = watchdog_get_poll_interval(wd);
	g_print("Starting watchdog with poll interval: %d\n", poll_interval);
	watchdogid = g_timeout_add(poll_interval, poll_watchdog, user_data);
	watchdog_complete_start(wd,invocation);
	return TRUE;
}

static gboolean
on_poke(Watchdog *wd,
		GDBusMethodInvocation *invocation,
		gpointer user_data)
{
	watchdog_set_watchdog(wd,1);
	watchdog_complete_poke(wd,invocation);
	return TRUE;
}

static gboolean
on_stop(Watchdog *wd,
		GDBusMethodInvocation *invocation,
		gpointer user_data)
{
	g_print("Stopping watchdog\n");
	remove_watchdog();
	watchdog_complete_stop(wd,invocation);
	return TRUE;
}

static void
on_bus_acquired(GDBusConnection *connection,
		const gchar *name,
		gpointer user_data)
{
	manager = g_dbus_object_manager_server_new(dbus_object_path);
	gchar *s;
	s = g_strdup_printf("%s/%s",dbus_object_path,instance_name);
	ObjectSkeleton *object = object_skeleton_new(s);
	g_free(s);

	Watchdog *wd = watchdog_skeleton_new();
	object_skeleton_set_watchdog(object, wd);
	g_object_unref(wd);

	// set properties
	watchdog_set_watchdog(wd,1);

	//define method callbacks here
	g_signal_connect(wd,
			"handle-start",
			G_CALLBACK(on_start),
			object); /* user_data */

	g_signal_connect(wd,
			"handle-poke",
			G_CALLBACK(on_poke),
			object); /* user_data */

	g_signal_connect(wd,
			"handle-stop",
			G_CALLBACK(on_stop),
			object); /* user_data */

	g_signal_connect(wd,
			"handle-set",
			G_CALLBACK(set_poll_interval),
			object); /* user_data */

	/* Export the object (@manager takes its own reference to @object) */
	g_dbus_object_manager_server_set_connection(manager, connection);
	g_dbus_object_manager_server_export(manager, G_DBUS_OBJECT_SKELETON(object));
	g_object_unref(object);
}

static void
on_name_acquired(GDBusConnection *connection,
		const gchar *name,
		gpointer user_data)
{
	//g_print ("Acquired the name %s\n", name);
}

static void
on_name_lost(GDBusConnection *connection,
		const gchar *name,
		gpointer user_data)
{
	//g_print ("Lost the name %s\n", name);
}


gint
main(gint argc, gchar *argv[])
{
	GMainLoop *loop;
	cmdline cmd;
	cmd.argc = argc;
	cmd.argv = argv;
	guint id;
	loop = g_main_loop_new(NULL, FALSE);

	id = g_bus_own_name(DBUS_TYPE,
			dbus_name,
			G_BUS_NAME_OWNER_FLAGS_ALLOW_REPLACEMENT |
			G_BUS_NAME_OWNER_FLAGS_REPLACE,
			on_bus_acquired,
			on_name_acquired,
			on_name_lost,
			&cmd,
			NULL);

	g_main_loop_run(loop);

	g_bus_unown_name(id);
	g_main_loop_unref(loop);
	return 0;
}
