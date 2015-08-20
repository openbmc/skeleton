#include "interfaces/sensor.h"

/* ---------------------------------------------------------------------------------------------------- */

static GDBusObjectManagerServer *manager = NULL;
static SensorIntegerSettable *sensor = NULL;

static gboolean
on_get_units    (SensorIntegerSettable  *sen,
                GDBusMethodInvocation  *invocation,
                gpointer                user_data)
{
  const gchar* val = sensor_integer_settable_get_units(sen);
  sensor_integer_settable_complete_get_units(sen,invocation,val);
  return TRUE;
}

static gboolean
on_get          (SensorIntegerSettable  *sen,
                GDBusMethodInvocation  *invocation,
                gpointer                user_data)
{
  guint val = sensor_integer_settable_get_value(sen);
  sensor_integer_settable_complete_get_value(sen,invocation,val);
  return TRUE;
}
static gboolean
on_set          (SensorIntegerSettable  *sen,
                GDBusMethodInvocation  *invocation,
                guint                   value,
                gpointer                user_data)
{
  sensor_integer_settable_set_value(sen,value);
  sensor_integer_settable_complete_set_value(sen,invocation);
  sensor_integer_settable_emit_changed(sen,value);
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

  manager = g_dbus_object_manager_server_new ("/org/openbmc/Sensors/HostStatus");

  gchar *s;
  s = g_strdup_printf ("/org/openbmc/Sensors/HostStatus/0");
  object = object_skeleton_new (s);
  g_free (s);

  sensor = sensor_integer_settable_skeleton_new ();
  object_skeleton_set_sensor_integer_settable (object, sensor);
  g_object_unref (sensor);

  //define method callbacks here
  g_signal_connect (sensor,
                    "handle-get-value",
                    G_CALLBACK (on_get),
                    NULL); /* user_data */
  g_signal_connect (sensor,
                    "handle-set-value",
                    G_CALLBACK (on_set),
                    NULL); /* user_data */
  g_signal_connect (sensor,
                    "handle-get-units",
                    G_CALLBACK (on_get_units),
                    NULL); /* user_data */

  sensor_integer_settable_set_units(sensor,"");
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

gint
main (gint argc, gchar *argv[])
{
  GMainLoop *loop;

  guint id;
  //g_type_init ();
  loop = g_main_loop_new (NULL, FALSE);

  id = g_bus_own_name (G_BUS_TYPE_SESSION,
                       "org.openbmc.Sensors.HostStatus",
                       G_BUS_NAME_OWNER_FLAGS_ALLOW_REPLACEMENT |
                       G_BUS_NAME_OWNER_FLAGS_REPLACE,
                       on_bus_acquired,
                       on_name_acquired,
                       on_name_lost,
                       loop,
                       NULL);
  g_main_loop_run (loop);
  
  g_bus_unown_name (id);
  g_main_loop_unref (loop);
  return 0;
}
