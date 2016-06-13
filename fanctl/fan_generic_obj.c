#include "interfaces/openbmc_intf.h"
#include "openbmc.h"

/* ------------------------------------------------------------------------- */

static const gchar* dbus_object_path = "/org/openbmc/control";
static const gchar* dbus_name = "org.openbmc.control.Fan";
static guint heartbeat = 0;

static GDBusObjectManagerServer *manager = NULL;

static gboolean
poll_sensor(gpointer user_data)
{
	//FruFan *fan = object_get_fan((Object*)user_data);
	return TRUE;
}

static gboolean
on_set_speed(Fan *fan,
		GDBusMethodInvocation *invocation,
		guint speed,
		gpointer user_data)
{
	fan_set_speed(fan,speed);
	fan_complete_set_speed(fan,invocation);
	return TRUE;
}

static gboolean
on_get_speed(Fan *fan,
		GDBusMethodInvocation *invocation,
		gpointer user_data)
{
	guint reading = fan_get_speed(fan);
	fan_complete_get_speed(fan,invocation,reading);
	return TRUE;
}

static void
on_bus_acquired(GDBusConnection *connection,
		const gchar *name,
		gpointer user_data)
{
	//g_print ("Acquired a message bus connection: %s\n",name);

	cmdline *cmd = user_data;
	if(cmd->argc < 2)
	{
		g_print("No objects created.  Put object name(s) on command line\n");
		return;
	}
	manager = g_dbus_object_manager_server_new(dbus_object_path);
	int i=0;
	for(i=1;i<cmd->argc;i++)
	{
		gchar *s;
		s = g_strdup_printf("%s/%s",dbus_object_path,cmd->argv[i]);
		ObjectSkeleton *object = object_skeleton_new(s);
		g_free(s);

		Fan *fan = fan_skeleton_new();
		object_skeleton_set_fan(object, fan);
		g_object_unref(fan);

		//define method callbacks here
		g_signal_connect(fan,
				"handle-get-speed",
				G_CALLBACK(on_get_speed),
				NULL); /* user_data */
		g_signal_connect(fan,
				"handle-set-speed",
				G_CALLBACK(on_set_speed),
				NULL); /* user_data */

		//g_timeout_add(poll_interval, poll_sensor, object);

		/* Export the object (@manager takes its own reference to @object) */
		g_dbus_object_manager_server_set_connection(manager, connection);
		g_dbus_object_manager_server_export(manager, G_DBUS_OBJECT_SKELETON(object));
		g_object_unref(object);
	}
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
