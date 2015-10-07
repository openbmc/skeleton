#include "interfaces/openbmc_intf.h"
#include "pflash/pflash.c"
#include "openbmc.h"

/* ---------------------------------------------------------------------------------------------------- */
static const gchar* dbus_object_path = "/org/openbmc/flash";
static const gchar* dbus_name        = "org.openbmc.flash.Bios";

static GDBusObjectManagerServer *manager = NULL;

void update(Flash *flash, const gchar* write_file)
{
	printf("Flashing: %s\n",write_file);
	// get size from file
	struct stat stbuf;
	uint32_t address = 0, read_size = 0, write_size = 0;

#ifdef __arm__
	if (stat(write_file, &stbuf))
 	{
 		printf("ERROR:  Invalid flash file: %s\n",write_file);
	}
	write_size = stbuf.st_size;
	// TODO: need to change pflash to return error instead of exit
	erase_chip();
	program_file(write_file, address, write_size);
#endif
  	flash_emit_updated(flash);
}

static gboolean
on_init (Flash          *f,
                GDBusMethodInvocation  *invocation,
                gpointer                user_data)
{
	flash_complete_init(f,invocation);

	#ifdef __arm__
		printf("Tuning BIOS Flash\n");
		flash_access_setup_pnor(true, false, false);
	#endif

	return TRUE;
}

static gboolean
on_update_via_tftp (Flash          *flash,
                GDBusMethodInvocation  *invocation,
                gchar*                  url,
                gchar*                  write_file,
                gpointer                user_data)
{
	printf("Flashing BIOS from TFTP: %s,%s\n",url,write_file);
	flash_emit_download(flash,url,write_file);
	flash_complete_update_via_tftp(flash,invocation);
	return TRUE;
}

static gboolean
on_update (Flash          *flash,
                GDBusMethodInvocation  *invocation,
                gchar*                  write_file,
                gpointer                user_data)
{
	printf("Flashing BIOS from file\n");
	flash_complete_update(flash,invocation);
	update(flash,write_file);
  	flash_emit_updated(flash);
	return TRUE;
}

static void
on_download_complete (GDBusConnection* connection,
               const gchar* sender_name,
               const gchar* object_path,
               const gchar* interface_name,
               const gchar* signal_name,
               GVariant* parameters,
               gpointer user_data) 
{
	Flash *flash = object_get_flash((Object*)user_data);
	
	GVariantIter *iter = g_variant_iter_new(parameters);
	GVariant* value = g_variant_iter_next_value(iter);
	const gchar* write_file;
	gsize size;
	write_file = g_variant_get_string(value,&size);
	update(flash,write_file);
	
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
                    "handle-update",
                    G_CALLBACK (on_update),
                    NULL); /* user_data */
		g_signal_connect (flash,
                    "handle-update-via-tftp",
                    G_CALLBACK (on_update_via_tftp),
                    NULL); /* user_data */

		g_signal_connect (flash,
                    "handle-init",
                    G_CALLBACK (on_init),
                    NULL); /* user_data */

		g_dbus_connection_signal_subscribe(connection,
                                   "org.openbmc.managers.Download",
                                   "org.openbmc.managers.Download",
                                   "DownloadComplete",
                                   "/org/openbmc/managers/Download",
                                   NULL,
                                   G_DBUS_SIGNAL_FLAGS_NONE,
                                   (GDBusSignalCallback) on_download_complete,
                                   object,
                                   NULL );

 
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
