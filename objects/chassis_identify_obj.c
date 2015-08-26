#include "interfaces/led.h"

/* ---------------------------------------------------------------------------------------------------- */
static const gchar* dbus_object_path = "/org/openbmc/leds/ChassisIdentify";
static const gchar* dbus_name        = "org.openbmc.leds.ChassisIdentify";

static GDBusObjectManagerServer *manager = NULL;
//static Led *led = NULL;
static uint gpio = 12;

static gboolean
on_set_on       (Led          *led,
                GDBusMethodInvocation  *invocation,
                gpointer                user_data)
{
  g_print("Turn on chassis identify led\n");
  //TODO:  implement in hardware
  led_complete_set_on(led,invocation);
  return TRUE;

}

static gboolean
on_set_off       (Led          *led,
                GDBusMethodInvocation  *invocation,
                gpointer                user_data)
{
  g_print("Turn off chassis identify led\n");
  //TODO:  implement in hardware
  led_complete_set_off(led,invocation);
  return TRUE;
}

static void 
on_bus_acquired (GDBusConnection *connection,
                 const gchar     *name,
                 gpointer         user_data)
{
  ObjectSkeleton *object;
  Led *led;
  guint n;

  g_print ("Acquired a message bus connection: %s\n",name);

  manager = g_dbus_object_manager_server_new (dbus_object_path);

  gchar *s;
  s = g_strdup_printf ("%s/0",dbus_object_path);
  object = object_skeleton_new (s);
  g_free (s);

  led = led_skeleton_new ();
  object_skeleton_set_led (object, led);
  g_object_unref (led);

  //define method callbacks
  g_signal_connect (led,
                    "handle-set-on",
                    G_CALLBACK (on_set_on),
                    NULL); /* user_data */
  g_signal_connect (led,
                    "handle-set-off",
                    G_CALLBACK (on_set_off),
                    NULL);

  led_set_color(led,0);
  led_set_function(led,"CHASSIS_IDENTIFY");
 
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
                       dbus_name,
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
