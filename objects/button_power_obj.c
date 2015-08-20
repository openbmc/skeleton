#include "interfaces/button.h"

/* ---------------------------------------------------------------------------------------------------- */

static GDBusObjectManagerServer *manager = NULL;
static Button *button = NULL;
static const int gpio = 12;

static gboolean
on_is_on       (Button          *btn,
                GDBusMethodInvocation  *invocation,
                gpointer                user_data)
{
  gboolean btn_state=button_get_state(btn);
  button_complete_is_on(btn,invocation,btn_state);
  return TRUE;

}

static gboolean
on_sim_button_press       (Button          *btn,
                GDBusMethodInvocation  *invocation,
                gpointer                user_data)
{
  g_print("Simulating button pressed\n");
  button_emit_button_pressed(btn);
  button_complete_sim_button_press(btn,invocation);
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

  manager = g_dbus_object_manager_server_new ("/org/openbmc/buttons/ButtonPower");

  gchar *s;
  s = g_strdup_printf ("/org/openbmc/buttons/ButtonPower/0");
  object = object_skeleton_new (s);
  g_free (s);

  button = button_skeleton_new ();
  object_skeleton_set_button (object, button);
  g_object_unref (button);

  //define method callbacks
  g_signal_connect (button,
                    "handle-is-on",
                    G_CALLBACK (on_is_on),
                    NULL); /* user_data */
  g_signal_connect (button,
                    "handle-sim-button-press",
                    G_CALLBACK (on_sim_button_press),
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


gint
main (gint argc, gchar *argv[])
{
  GMainLoop *loop;

  guint id;
  //g_type_init ();
  loop = g_main_loop_new (NULL, FALSE);

  id = g_bus_own_name (G_BUS_TYPE_SESSION,
                       "org.openbmc.buttons.ButtonPower",
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
