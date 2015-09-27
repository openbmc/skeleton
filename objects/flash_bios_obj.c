#include "interfaces/openbmc_intf.h"
#include "pflash/pflash.c"
#include "openbmc.h"

/* ---------------------------------------------------------------------------------------------------- */
static const gchar* dbus_object_path = "/org/openbmc/flash";
static const gchar* dbus_name        = "org.openbmc.flash.BIOS";

static GDBusObjectManagerServer *manager = NULL;

static gboolean
on_init (Flash          *f,
                GDBusMethodInvocation  *invocation,
                gpointer                user_data)
{
	flash_complete_init(f,invocation);
	//tune flash
	g_print("Tuning BIOS Flash\n");
	flash_access_setup_pnor(true, false, false);
	return TRUE;
}

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
		s = g_strdup_printf ("%s/%s",dbus_object_path,cmd->argv[i]);
		object = object_skeleton_new (s);
		g_free (s);

		Flash* flash = flash_skeleton_new ();
		object_skeleton_set_flash (object, flash);
 		g_object_unref (flash);

		//define method callbacks here
		g_signal_connect (flash,
                    "handle-update-via-file",
                    G_CALLBACK (on_update_via_file),
                    NULL); /* user_data */
 
		g_signal_connect (flash,
                    "handle-init",
                    G_CALLBACK (on_init),
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
