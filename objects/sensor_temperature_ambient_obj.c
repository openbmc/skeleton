#include "interfaces/sensor.h"



/* ---------------------------------------------------------------------------------------------------- */
typedef enum { NORMAL,LOWER_CRITICAL,LOWER_WARNING,UPPER_WARNING,UPPER_CRITICAL } threshold_states;


static const gchar* dbus_object_path = "/org/openbmc/sensors/Temperature/Ambient";
static const gchar* dbus_name        = "org.openbmc.sensors.Temperature.Ambient";



static GDBusObjectManagerServer *manager = NULL;
static SensorInteger *sensor = NULL;

static gchar* i2c_bus = "";
static gchar* i2c_address = "";
static gboolean thresholds_set = FALSE;



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

static gboolean
on_set_thresholds (SensorInteger                 *sen,
                   GDBusMethodInvocation  *invocation,
		   guint                  lc,
		   guint                  lw,
		   guint                  uw,
		   guint                  uc,
                   gpointer               user_data)
{
  sensor_integer_set_threshold_lower_critical(sen,lc);
  sensor_integer_set_threshold_lower_warning(sen,lw);
  sensor_integer_set_threshold_upper_warning(sen,uw);
  sensor_integer_set_threshold_upper_critical(sen,uc);
  sensor_integer_complete_set_thresholds(sen,invocation);
  thresholds_set = TRUE;
  return TRUE;
}

static gboolean
on_get_threshold_state (SensorInteger                 *sen,
                   GDBusMethodInvocation  *invocation,
                   gpointer               user_data)
{
  guint state = sensor_integer_get_threshold_state(sen);
  sensor_integer_complete_get_threshold_state(sen,invocation,state);
  return TRUE;
}


static gboolean
check_thresholds()
{
  if (thresholds_set == TRUE) {
  threshold_states state = NORMAL;
  guint value = sensor_integer_get_value(sensor);

  if (value < sensor_integer_get_threshold_lower_critical(sensor)) {
    state = LOWER_CRITICAL;
  }
  else if(value < sensor_integer_get_threshold_lower_warning(sensor)) {
    state = LOWER_WARNING;
  }
  else if(value > sensor_integer_get_threshold_upper_critical(sensor)) {
    state = UPPER_CRITICAL;
  }
  else if(value > sensor_integer_get_threshold_upper_warning(sensor)) {
    state = UPPER_WARNING;
  }
  // only emit signal if threshold state changes
  if (state != sensor_integer_get_threshold_state(sensor))
  {
     sensor_integer_set_threshold_state(sensor,state);
     if (state == LOWER_CRITICAL || state == UPPER_CRITICAL)
     {
       sensor_integer_emit_critical(sensor);
       g_print("Critical\n");
     }
     else if (state == LOWER_WARNING || state == UPPER_WARNING)
     {
       sensor_integer_emit_warning(sensor);
       g_print("Warning\n");
     }
  }
  }
}

static void 
on_bus_acquired (GDBusConnection *connection,
                 const gchar     *name,
                 gpointer         user_data)
{
  ObjectSkeleton *object;
  guint n;

  g_print ("Acquired a message bus connection: %s\n",name);

  manager = g_dbus_object_manager_server_new (dbus_object_path);

  gchar *s;
  s = g_strdup_printf ("%s/0",dbus_object_path);
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
 
  g_signal_connect (sensor,
                    "handle-set-thresholds",
                    G_CALLBACK (on_set_thresholds),
                    NULL); /* user_data */

  g_signal_connect (sensor,
                    "handle-get-threshold-state",
                    G_CALLBACK (on_get_threshold_state),
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
    guint value = sensor_integer_get_value(sensor);
    //TOOD:  Change to actually read sensor
    value = value+1;
    g_print("Polling sensor:  %d\n",value);

    //if changed, set property and emit signal
    if (value != sensor_integer_get_value(sensor))
    {
       g_print("Sensor changed\n");
       sensor_integer_set_value(sensor,value);
       sensor_integer_emit_changed(sensor,value);
       if (thresholds_set == TRUE)
       {
         check_thresholds();
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
                       dbus_name,
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
