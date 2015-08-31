#include "interfaces/sensor2.h"
#include "openbmc.h"
#include "sensor_threshold.h"


/* ---------------------------------------------------------------------------------------------------- */

static const gchar* dbus_object_path = "/org/openbmc/sensors/Temperature/Ambient";
static const gchar* dbus_name        = "org.openbmc.sensors.Temperature.Ambient";
static const guint poll_interval = 3000;
static guint heartbeat = 0;

static GDBusObjectManagerServer *manager = NULL;

static gchar* i2c_bus = "";
static gchar* i2c_address = "";
static gboolean inited = FALSE;

static gboolean
poll_sensor(gpointer user_data)
{
	if (!inited)
	{
		return TRUE;
	}
	SensorInteger *sensor = object_get_sensor_integer((Object*)user_data);
	SensorIntegerThreshold *threshold = object_get_sensor_integer_threshold((Object*)user_data);
 	guint value = sensor_integer_get_value(sensor);
	//TOOD:  Change to actually read sensor
	value = value+1;
	if (heartbeat > 10000)
	{
		heartbeat = 0;
		sensor_integer_emit_heartbeat(sensor,dbus_name);
	}
	else
 	{
		heartbeat = heartbeat+poll_interval;
	}

    // End actually reading sensor

    //if changed, set property and emit signal
    if (value != sensor_integer_get_value(sensor))
    {
       sensor_integer_set_value(sensor,value);
       sensor_integer_emit_changed(sensor,value);
       check_thresholds(threshold,value);
    }
    return TRUE;
}

static gboolean
on_init         (SensorInteger  *sen,
                GDBusMethodInvocation  *invocation,
                gpointer                user_data)
{
  inited = TRUE;
  g_print("Sensor init");
  sensor_integer_complete_init(sen,invocation);
  return TRUE;
}


static gboolean
on_get_units    (SensorInteger  *sen,
                GDBusMethodInvocation  *invocation,
                gpointer                user_data)
{
  const gchar* val = sensor_integer_get_units(sen);
  sensor_integer_complete_get_units(sen,invocation,val);
  return TRUE;
}

static gboolean
on_get (SensorInteger                 *sen,
                GDBusMethodInvocation  *invocation,
                gpointer                user_data)
{
  guint reading = sensor_integer_get_value(sen);
  sensor_integer_complete_get_value(sen,invocation,reading);
  return TRUE;
}

static gboolean
on_set_config (SensorInteger                 *sen,
                GDBusMethodInvocation  *invocation,
		gchar**                  config,
                gpointer                user_data)
{
  g_print("I2C bus = %s\n",config[0]);
  g_print("I2C addr = %s\n",config[1]);
  sensor_integer_complete_set_config_data(sen,invocation);
  return TRUE;
}


static void 
on_bus_acquired (GDBusConnection *connection,
                 const gchar     *name,
                 gpointer         user_data)
{
  	g_print ("Acquired a message bus connection: %s\n",name);

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

		SensorInteger *sensor = sensor_integer_skeleton_new ();
  		object_skeleton_set_sensor_integer (object, sensor);
  		g_object_unref (sensor);
		
		SensorIntegerThreshold *threshold = sensor_integer_threshold_skeleton_new();
		object_skeleton_set_sensor_integer_threshold (object,threshold);
		g_object_unref (threshold);

  		// set units
  		sensor_integer_set_units(sensor,"C");
		sensor_integer_threshold_set_state(threshold,NOT_SET);
  		//define method callbacks here
  		g_signal_connect (sensor,
                    "handle-get-value",
                    G_CALLBACK (on_get),
                    NULL); /* user_data */
  		g_signal_connect (sensor,
                    "handle-get-units",
                    G_CALLBACK (on_get_units),
                    NULL); /* user_data */

  		g_signal_connect (sensor,
                    "handle-set-config-data",
                    G_CALLBACK (on_set_config),
                    NULL); /* user_data */

 		g_signal_connect (sensor,
                    "handle-init",
                    G_CALLBACK (on_init),
                    NULL); /* user_data */
 
  		g_signal_connect (threshold,
                    "handle-set",
                    G_CALLBACK (set_thresholds),
                    NULL); /* user_data */

  		g_signal_connect (threshold,
                    "handle-get-state",
                    G_CALLBACK (get_threshold_state),
                    NULL); /* user_data */

  		g_timeout_add(poll_interval, poll_sensor, object);

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
  g_print ("Acquired the name %s\n", name);
}

static void
on_name_lost (GDBusConnection *connection,
              const gchar     *name,
              gpointer         user_data)
{
  g_print ("Lost the name %s\n", name);
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
