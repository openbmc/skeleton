#include "interfaces/flash.h"
#include "pflash/pflash.c"

/* ---------------------------------------------------------------------------------------------------- */
static const gchar* dbus_object_path = "/org/openbmc/flash/BIOS";
static const gchar* dbus_name        = "org.openbmc.flash.BIOS";

static GDBusObjectManagerServer *manager = NULL;
static Flash *flash = NULL;

static gboolean
on_update_via_file (Flash          *f,
                GDBusMethodInvocation  *invocation,
                gchar*                  write_file,
                gpointer                user_data)
{
  g_print("Flashing BIOS from file\n");
  // get size from file
  struct stat stbuf;
  uint32_t address = 0, read_size = 0, write_size = 0;

  if (stat(write_file, &stbuf))
  {
    g_print("Failed to get file size");
    //TODO: Error handling
  }
  write_size = stbuf.st_size;
  erase_chip();
  program_file(write_file, address, write_size);
  flash_complete_update_via_file(f,invocation);
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

  flash = flash_skeleton_new ();
  object_skeleton_set_flash (object, flash);
  g_object_unref (flash);

  //define method callbacks here
  g_signal_connect (flash,
                    "handle-update-via-file",
                    G_CALLBACK (on_update_via_file),
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
