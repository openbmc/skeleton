#include "interfaces/sensor.h"

/* ---------------------------------------------------------------------------------------------------- */

static GDBusObjectManagerServer *manager = NULL;
static SensorInteger *sensor = NULL;

static gchar* i2c_bus = "";
static gchar* i2c_address = "";
static gboolean go = FALSE;

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
  go = TRUE;
  return TRUE;
}


static void 
on_bus_acquired (GDBusConnection *connection,
                 const gchar     *name,
                 gpointer         user_data)
{
  ObjectSkeleton *object;
  guint n;

  g_print ("Acquired a message bus connection: %s\n",name);

  manager = g_dbus_object_manager_server_new ("/org/openbmc/Sensors/Temperature/Ambient");

  gchar *s;
  s = g_strdup_printf ("/org/openbmc/Sensors/Temperature/Ambient/0");
  object = object_skeleton_new (s);
  g_free (s);

  sensor = sensor_integer_skeleton_new ();
  object_skeleton_set_sensor_integer (object, sensor);
  g_object_unref (sensor);

  sensor_integer_set_units(sensor,"C");
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



  /* Export the object (@manager takes its own reference to @object) */
  g_dbus_object_manager_server_export (manager, G_DBUS_OBJECT_SKELETON (object));
  g_object_unref (object);

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

static gboolean
poll_sensor()
{
  if (go)
  {
    guint value = sensor_integer_get_value(sensor);
    //TOOD:  Change to actually read sensor
    value = value+1;
    g_print("Polling sensor:  %d\n",value);

    //if changed, set property and emit signal
    if (value != sensor_integer_get_value(sensor))
    {
       g_print("Sensor changed");
       sensor_integer_set_value(sensor,value);
       sensor_integer_emit_changed(sensor,value);
    }
  }
    return TRUE;
}

gint
main (gint argc, gchar *argv[])
{
  GMainLoop *loop;

  guint id;
  //g_type_init ();
  loop = g_main_loop_new (NULL, FALSE);

  id = g_bus_own_name (G_BUS_TYPE_SESSION,
                       "org.openbmc.Sensors.Temperature.Ambient",
                       G_BUS_NAME_OWNER_FLAGS_ALLOW_REPLACEMENT |
                       G_BUS_NAME_OWNER_FLAGS_REPLACE,
                       on_bus_acquired,
                       on_name_acquired,
                       on_name_lost,
                       loop,
                       NULL);

  g_timeout_add(5000, poll_sensor, NULL);
  g_main_loop_run (loop);
  
  g_bus_unown_name (id);
  g_main_loop_unref (loop);
  return 0;
}
