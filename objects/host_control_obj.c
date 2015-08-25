#include "interfaces/host_control.h"

/* ---------------------------------------------------------------------------------------------------- */
static const gchar* dbus_object_path = "/org/openbmc/control/Host";
static const gchar* dbus_name        = "org.openbmc.control.Host";

static GDBusObjectManagerServer *manager = NULL;
static HostControl *host_control = NULL;

static guint gpio_fsiclk = 27;
static guint gpio_fsidat = 28;

static gboolean
on_boot         (HostControl        *host,
                GDBusMethodInvocation  *invocation,
                gpointer                user_data)
{
  // TODO: Implement gpio toggling
  g_print("Boot");
  host_control_complete_boot(host,invocation);
  host_control_emit_booted(host);
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

  manager = g_dbus_object_manager_server_new (dbus_object_path);

  gchar *s;
  s = g_strdup_printf ("%s/0",dbus_object_path);
  object = object_skeleton_new (s);
  g_free (s);
  host_control = host_control_skeleton_new ();
  object_skeleton_set_host_control (object, host_control);
  g_object_unref (host_control);

  //define method callbacks here
  g_signal_connect (host_control,
                    "handle-boot",
                    G_CALLBACK (on_boot),
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
