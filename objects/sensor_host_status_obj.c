#include "interfaces/openbmc_intf.h"
#include "openbmc.h"

#define BOOTED 100
/* ---------------------------------------------------------------------------------------------------- */

static const gchar* dbus_object_path = "/org/openbmc/sensors";
static const gchar* dbus_name        = "org.openbmc.sensors.HostStatus";
static guint heartbeat = 0;

static GDBusObjectManagerServer *manager = NULL;

static gboolean
on_init         (SensorValue  *sen,
                GDBusMethodInvocation  *invocation,
                gpointer                user_data)
{
	sensor_value_complete_init(sen,invocation);
	return TRUE;
}

static gboolean
on_init_control         (Control *control,
                GDBusMethodInvocation  *invocation,
                gpointer                user_data)
{
	control_complete_init(control,invocation);
	return TRUE;
}

static void
on_set_ipmi (GDBusConnection* connection,
               const gchar* sender_name,
               const gchar* object_path,
               const gchar* interface_name,
               const gchar* signal_name,
               GVariant* parameters,
               gpointer user_data) 
{
	SensorMatch *match = object_get_sensor_match((Object*)user_data);
	SensorValue *sen = object_get_sensor_value((Object*)user_data);
	SensorIpmi *ipmi = object_get_sensor_ipmi((Object*)user_data);
	Control* control = object_get_control((Object*)user_data);
	
	guchar sensor_id;
	guchar host_status;
	g_variant_get (parameters, "(yy)", &sensor_id,&host_status);
	guchar my_sensor_id = sensor_ipmi_get_sensor_id(ipmi);
	if (my_sensor_id == sensor_id)
	{
		GVariant *old_value = sensor_value_get_value(sen);
		GVariant *value = NEW_VARIANT_B(host_status);
		if (VARIANT_COMPARE(old_value,value) != 0)
		{
			sensor_value_set_value(sen, value);
			sensor_value_emit_changed(sen, value, "");
			if (host_status == BOOTED)
			{
				sensor_match_set_state(match,host_status);
				sensor_match_emit_sensor_match(match,host_status);
				control_emit_goto_system_state(control,"BOOTED");
			}
		}
	}
}

static void 
on_bus_acquired (GDBusConnection *connection,
                 const gchar     *name,
                 gpointer         user_data)
{
//  	g_print ("Acquired a message bus connection: %s\n",name);

  	cmdline *cmd = user_data;
	if (cmd->argc < 2)
	{
		g_print("No objects created.  Put object name(s) on command line\n");
		return;
	}	
  	manager = g_dbus_object_manager_server_new (dbus_object_path);
  	int i=0;
  	for (i=1;i<cmd->argc;i++)
  	{
		gchar *s;
 		s = g_strdup_printf ("%s/%s",dbus_object_path,cmd->argv[i]);
		ObjectSkeleton *object = object_skeleton_new (s);
		g_free (s);

		SensorValue *sensor = sensor_value_skeleton_new ();
  		object_skeleton_set_sensor_value (object, sensor);
  		g_object_unref (sensor);

		SensorMatch *match = sensor_match_skeleton_new ();
  		object_skeleton_set_sensor_match (object, match);
  		g_object_unref (match);

		SensorIpmi *ipmi = sensor_ipmi_skeleton_new ();
  		object_skeleton_set_sensor_ipmi (object, ipmi);
  		g_object_unref (ipmi);

		Control *control = control_skeleton_new ();
  		object_skeleton_set_control (object, control);
  		g_object_unref (control);

		//must init variant
		GVariant* v = NEW_VARIANT_B(0);
		sensor_value_set_value(sensor,v);
	
		// set units
  		sensor_value_set_units(sensor,"");
  		sensor_value_set_settable(sensor,TRUE);
		//must emit change so sensor manager sees initial value
		sensor_value_emit_changed(sensor,v,"");

		//signal handlers
		g_dbus_connection_signal_subscribe(connection,
                                   "org.openbmc.sensors.IpmiBt",
                                   "org.openbmc.sensors.IpmiBt",
                                   "SetSensor",
                                   "/org/openbmc/sensors/IpmiBt",
                                   NULL,
                                   G_DBUS_SIGNAL_FLAGS_NONE,
                                   (GDBusSignalCallback) on_set_ipmi,
                                   object,
                                   NULL );
  		//define method callbacks here
  		g_signal_connect (sensor,
                    "handle-init",
                    G_CALLBACK (on_init),
                    NULL); /* user_data */
  
 		g_signal_connect (control,
                    "handle-init",
                    G_CALLBACK (on_init_control),
                    NULL); /* user_data */

		//g_signal_connect (sensor,
                  //  "handle-set-value",
                  //  G_CALLBACK (on_set_value),
                  //  object); /* user_data */


  		/* Export the object (@manager takes its own reference to @object) */
  		g_dbus_object_manager_server_export (manager, G_DBUS_OBJECT_SKELETON (object));
  		g_object_unref (object);
	}

  /* Export all objects */
  g_dbus_object_manager_server_set_connection (manager, connection);
}

static void
on_name_acquired (GDBusConnection *connection,
                  const gchar     *name,
                  gpointer         user_data)
{
  //g_print ("Acquired the name %s\n", name);
}

static void
on_name_lost (GDBusConnection *connection,
              const gchar     *name,
              gpointer         user_data)
{
  //g_print ("Lost the name %s\n", name);
}


gint
main (gint argc, gchar *argv[])
{
  GMainLoop *loop;
  cmdline cmd;
  cmd.argc = argc;
  cmd.argv = argv;
  guint id;
  loop = g_main_loop_new (NULL, FALSE);

  id = g_bus_own_name (G_BUS_TYPE_SESSION,
                       dbus_name,
                       G_BUS_NAME_OWNER_FLAGS_ALLOW_REPLACEMENT |
                       G_BUS_NAME_OWNER_FLAGS_REPLACE,
                       on_bus_acquired,
                       on_name_acquired,
                       on_name_lost,
                       &cmd,
                       NULL);

  g_main_loop_run (loop);
  
  g_bus_unown_name (id);
  g_main_loop_unref (loop);
  return 0;
}
