#include "interfaces/openbmc_intf.h"
#include "openbmc.h"
#include "sensor_threshold.h"


/* ---------------------------------------------------------------------------------------------------- */

static const gchar* dbus_object_path = "/org/openbmc/sensors";
static const gchar* dbus_name        = "org.openbmc.sensors.Temperature.Ambient";
static guint heartbeat = 0;

static GDBusObjectManagerServer *manager = NULL;

static gboolean
poll_sensor(gpointer user_data)
{
	SensorValue *sensor = object_get_sensor_value((Object*)user_data);
	SensorThreshold *threshold = object_get_sensor_threshold((Object*)user_data);
	SensorI2c *i2c = object_get_sensor_i2c((Object*)user_data);

 	GVariant* v_value = sensor_value_get_value(sensor);
	guint poll_interval = sensor_value_get_poll_interval(sensor);

	//TODO:  Change to actually read sensor
	double value = GET_VARIANT_D(v_value);
	//g_print("Reading I2C = %s; Address = %s; %f\n",
	//	sensor_i2c_get_dev_path(i2c),sensor_i2c_get_address(i2c),value);

	value = value+1;
	// Do this in case of an error
	//sensor_value_emit_error(sensor);

	if (heartbeat > 4000)
	{
		heartbeat = 0;
		sensor_value_emit_heartbeat(sensor,dbus_name);
	}
	else
 	{
		heartbeat = heartbeat+poll_interval;
	}

    // End actually reading sensor

    //if changed, set property and emit signal
    if (value != GET_VARIANT_D(v_value))
    {
	GVariant* v_new_value = NEW_VARIANT_D(value);
	sensor_value_set_value(sensor,v_new_value);
	const gchar* units = sensor_value_get_units(sensor);
	sensor_value_emit_changed(sensor,v_new_value,units);
	check_thresholds(threshold,v_new_value);
    }
    return TRUE;
}

static gboolean
on_init         (SensorValue  *sen,
                GDBusMethodInvocation  *invocation,
                gpointer                user_data)
{

	guint poll_interval = sensor_value_get_poll_interval(sen);
	g_timeout_add(poll_interval, poll_sensor, user_data);
	sensor_value_complete_init(sen,invocation);
	return TRUE;
}



static void 
on_bus_acquired (GDBusConnection *connection,
                 const gchar     *name,
                 gpointer         user_data)
{
  //	g_print ("Acquired a message bus connection: %s\n",name);

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
 		s = g_strdup_printf ("%s/Temperature/%s",dbus_object_path,cmd->argv[i]);
		ObjectSkeleton *object = object_skeleton_new (s);
		g_free (s);

		SensorValue *sensor = sensor_value_skeleton_new ();
  		object_skeleton_set_sensor_value (object, sensor);
  		g_object_unref (sensor);
		
		SensorThreshold *threshold = sensor_threshold_skeleton_new();
		object_skeleton_set_sensor_threshold (object,threshold);
		g_object_unref (threshold);

		SensorI2c *i2c = sensor_i2c_skeleton_new();
		object_skeleton_set_sensor_i2c (object,i2c);
		g_object_unref (i2c);


  		// set properties
		GVariant* value = g_variant_new_variant(g_variant_new_double(1.0));
		sensor_value_set_value(sensor,value);
  		sensor_value_set_units(sensor,"C");
  		sensor_value_set_settable(sensor,FALSE);
		sensor_threshold_set_state(threshold,NOT_SET);
		
		sensor_threshold_set_upper_critical(threshold,
			g_variant_new_variant(g_variant_new_double(0.0)));
		sensor_threshold_set_upper_warning(threshold,
			g_variant_new_variant(g_variant_new_double(0.0)));
		sensor_threshold_set_lower_warning(threshold,
			g_variant_new_variant(g_variant_new_double(0.0)));
		sensor_threshold_set_lower_critical(threshold,
			g_variant_new_variant(g_variant_new_double(0.0)));

		//define method callbacks here

 		g_signal_connect (sensor,
                    "handle-init",
                    G_CALLBACK (on_init),
                    object); /* user_data */
 
  		g_signal_connect (threshold,
                    "handle-get-state",
                    G_CALLBACK (get_threshold_state),
                    NULL); /* user_data */



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
//  g_print ("Acquired the name %s\n", name);
}

static void
on_name_lost (GDBusConnection *connection,
              const gchar     *name,
              gpointer         user_data)
{
//  g_print ("Lost the name %s\n", name);
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
