#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "interfaces/openbmc_intf.h"
#include "openbmc.h"

/* ---------------------------------------------------------------------------------------------------- */
static const gchar* dbus_object_path = "/org/openbmc/flash";
static const gchar* dbus_name        = "org.openbmc.flash.Bios";
static const gchar* FLASHER_BIN      = "flasher.exe";
static const gchar* DLOAD_BUS = "org.openbmc.managers.Download";
static const gchar* DLOAD_OBJ = "/org/openbmc/managers/Download";

static GDBusObjectManagerServer *manager = NULL;

int update(Flash* flash)
{
	pid_t pid;
	int status=-1;
	pid = fork();
	if (pid == 0)
	{
		const gchar* path = flash_get_flasher_path(flash);
		const gchar* name = flash_get_flasher_name(flash);
		const gchar* inst = flash_get_flasher_instance(flash);
		const gchar* filename = flash_get_filename(flash);
		status = execl(path, name, inst, filename, NULL);
		return status;
	}
	return 0;
}

static gboolean
on_init (Flash          *f,
                GDBusMethodInvocation  *invocation,
                gpointer                user_data)
{
	flash_complete_init(f,invocation);

	//tune flash
	flash_set_filename(f,"");
	int rc = update(f);
	if (rc==-1)
	{
		printf("ERROR FlashControl_0: Unable to init\n");
	}
	return TRUE;
}

static gboolean
on_lock (SharedResource          *lock,
                GDBusMethodInvocation  *invocation,
		gchar*                  name,
                gpointer                user_data)
{
	gboolean locked = shared_resource_get_lock(lock);
	if (locked)
	{
		const gchar* name = shared_resource_get_name(lock);
		printf("ERROR: BIOS Flash is already locked: %s\n",name);
	}
	else
	{	
		printf("Locking BIOS Flash: %s\n",name);
		shared_resource_set_lock(lock,true);
		shared_resource_set_name(lock,name);
	}
	shared_resource_complete_lock(lock,invocation);
	return TRUE;
}
static gboolean
on_is_locked (SharedResource          *lock,
                GDBusMethodInvocation  *invocation,
                gpointer                user_data)
{
	gboolean locked = shared_resource_get_lock(lock);
	const gchar* name = shared_resource_get_name(lock);
	shared_resource_complete_is_locked(lock,invocation,locked,name);
	return TRUE;
}


static gboolean
on_unlock (SharedResource          *lock,
                GDBusMethodInvocation  *invocation,
                gpointer                user_data)
{
	printf("Unlocking BIOS Flash\n");
	shared_resource_set_lock(lock,false);
	shared_resource_set_name(lock,"");
	shared_resource_complete_unlock(lock,invocation);
	return TRUE;
}

static gboolean
on_update_via_tftp (Flash          *flash,
                GDBusMethodInvocation  *invocation,
                gchar*                  url,
                gchar*                  write_file,
                gpointer                user_data)
{
	SharedResource *lock = object_get_shared_resource((Object*)user_data);
	gboolean locked = shared_resource_get_lock(lock);
	flash_complete_update_via_tftp(flash,invocation);
	if (locked)
	{
		const gchar* name = shared_resource_get_name(lock);
		printf("BIOS Flash is locked: %s\n",name);
	}
	else
	{	
		printf("Flashing BIOS from TFTP: %s,%s\n",url,write_file);
		shared_resource_set_lock(lock,true);
		shared_resource_set_name(lock,dbus_object_path);
		flash_set_filename(flash,write_file);
		flash_emit_download(flash,url,write_file);
	}
	return TRUE;
}

static gboolean
on_update (Flash          *flash,
                GDBusMethodInvocation  *invocation,
                gchar*                  write_file,
                gpointer                user_data)
{
	int rc = 0;
	SharedResource *lock = object_get_shared_resource((Object*)user_data);
	gboolean locked = shared_resource_get_lock(lock);
	flash_complete_update(flash,invocation);
	if (locked)
	{
		const gchar* name = shared_resource_get_name(lock);
		printf("BIOS Flash is locked: %s\n",name);
	}
	else
	{	
		printf("Flashing BIOS from: %s\n",write_file);
		shared_resource_set_lock(lock,true);
		shared_resource_set_name(lock,dbus_object_path);
		flash_set_filename(flash,write_file);
		rc = update(flash);
		if (!rc)
		{
			shared_resource_set_lock(lock,false);
			shared_resource_set_name(lock,"");
		}
	}
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
	SharedResource *lock = object_get_shared_resource((Object*)user_data);

	GVariantIter *iter = g_variant_iter_new(parameters);
	GVariant* v_fullname = g_variant_iter_next_value(iter);
	gsize size;
	const gchar* fullname = g_variant_get_string(v_fullname,&size);
	int rc;
	flash_set_filename(flash,fullname);
	rc = update(flash);
	if (!rc)
	{
		shared_resource_set_lock(lock,false);
		shared_resource_set_name(lock,"");
	}
}

static void
on_flash_progress (GDBusConnection* connection,
               const gchar* sender_name,
               const gchar* object_path,
               const gchar* interface_name,
               const gchar* signal_name,
               GVariant* parameters,
               gpointer user_data) 
{
	Flash *flash = object_get_flash((Object*)user_data);
	SharedResource *lock = object_get_shared_resource((Object*)user_data);
	GVariantIter *iter = g_variant_iter_new(parameters);
	GVariant* v_filename = g_variant_iter_next_value(iter);
	GVariant* v_progress = g_variant_iter_next_value(iter);
	
	uint8_t progress = g_variant_get_byte(v_progress);
	printf("Progress: %d\n",progress);
}

static void
on_flash_done (GDBusConnection* connection,
               const gchar* sender_name,
               const gchar* object_path,
               const gchar* interface_name,
               const gchar* signal_name,
               GVariant* parameters,
               gpointer user_data) 
{
	Flash *flash = object_get_flash((Object*)user_data);
	SharedResource *lock = object_get_shared_resource((Object*)user_data);
	printf("Flash succeeded; unlocking flash\n");
	shared_resource_set_lock(lock,false);
	shared_resource_set_name(lock,"");
}

static void
on_flash_error (GDBusConnection* connection,
               const gchar* sender_name,
               const gchar* object_path,
               const gchar* interface_name,
               const gchar* signal_name,
               GVariant* parameters,
               gpointer user_data) 
{
	Flash *flash = object_get_flash((Object*)user_data);
	SharedResource *lock = object_get_shared_resource((Object*)user_data);
	printf("Flash Error; unlocking flash\n");
	shared_resource_set_lock(lock,false);
	shared_resource_set_name(lock,"");
}

static void
on_download_error (GDBusConnection* connection,
               const gchar* sender_name,
               const gchar* object_path,
               const gchar* interface_name,
               const gchar* signal_name,
               GVariant* parameters,
               gpointer user_data) 
{
	Flash *flash = object_get_flash((Object*)user_data);
	SharedResource *lock = object_get_shared_resource((Object*)user_data);
	printf("ERROR: FlashBios:  Download error; clearing flash lock\n");
	shared_resource_set_lock(lock,false);
	shared_resource_set_name(lock,"");
}

static void 
on_bus_acquired (GDBusConnection *connection,
                 const gchar     *name,
                 gpointer         user_data)
{
	ObjectSkeleton *object;
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

		SharedResource* lock = shared_resource_skeleton_new ();
		object_skeleton_set_shared_resource (object, lock);
 		g_object_unref (lock);

		shared_resource_set_lock(lock,false);
		shared_resource_set_name(lock,"");

		int c = strlen(cmd->argv[0]);

		//TODO: don't use fixed buffer
		char buf[512];
		memset(buf, '\0', sizeof(buf));
		bool found = false;
		while(c>0)
		{
			if (cmd->argv[0][c] == '/')
			{
				strncpy(buf,cmd->argv[0],c);
				s = g_strdup_printf ("%s/%s",buf,FLASHER_BIN);
				break;
			}
			c--;
		}
		if (c==0)
		{
			printf("ERROR FlashBios: Invalid Path = %s\n",cmd->argv[0]);
		}
		else
		{
			flash_set_flasher_path(flash,s);
			flash_set_flasher_name(flash,FLASHER_BIN);
			flash_set_flasher_instance(flash,cmd->argv[i]);
		}
		g_free (s);


		//define method callbacks here
		g_signal_connect (lock,
                    "handle-lock",
                    G_CALLBACK (on_lock),
                    NULL); /* user_data */
		g_signal_connect (lock,
                    "handle-unlock",
                    G_CALLBACK (on_unlock),
                    NULL); /* user_data */
		g_signal_connect (lock,
                    "handle-is-locked",
                    G_CALLBACK (on_is_locked),
                    NULL); /* user_data */

		g_signal_connect (flash,
                    "handle-update",
                    G_CALLBACK (on_update),
                    object); /* user_data */
		g_signal_connect (flash,
                    "handle-update-via-tftp",
                    G_CALLBACK (on_update_via_tftp),
                    object); /* user_data */

		g_signal_connect (flash,
                    "handle-init",
                    G_CALLBACK (on_init),
                    NULL); /* user_data */

		g_dbus_connection_signal_subscribe(connection,
				   DLOAD_BUS, DLOAD_BUS, "DownloadComplete",
                                   DLOAD_OBJ,NULL,G_DBUS_SIGNAL_FLAGS_NONE,
                                   (GDBusSignalCallback) on_download_complete,
                                   object,
                                   NULL );
		g_dbus_connection_signal_subscribe(connection,
				   DLOAD_BUS, DLOAD_BUS, "DownloadError",
                                   DLOAD_OBJ,NULL,G_DBUS_SIGNAL_FLAGS_NONE,
                                   (GDBusSignalCallback) on_download_error,
                                   object,
                                   NULL );

		s = g_strdup_printf ("/org/openbmc/control/%s",cmd->argv[i]);
		g_dbus_connection_signal_subscribe(connection,
                                   NULL,
                                   "org.openbmc.FlashControl",
                                   "Done",
                                   s,
                                   NULL,
                                   G_DBUS_SIGNAL_FLAGS_NONE,
                                   (GDBusSignalCallback) on_flash_done,
                                   object,
                                   NULL );
		g_free(s);
		s = g_strdup_printf ("/org/openbmc/control/%s",cmd->argv[i]);
		g_dbus_connection_signal_subscribe(connection,
                                   NULL,
                                   "org.openbmc.FlashControl",
                                   "Error",
                                   s,
                                   NULL,
                                   G_DBUS_SIGNAL_FLAGS_NONE,
                                   (GDBusSignalCallback) on_flash_error,
                                   object,
                                   NULL );

		g_free(s);
		s = g_strdup_printf ("/org/openbmc/control/%s",cmd->argv[i]);
		g_dbus_connection_signal_subscribe(connection,
                                   NULL,
                                   "org.openbmc.FlashControl",
                                   "Progress",
                                   s,
                                   NULL,
                                   G_DBUS_SIGNAL_FLAGS_NONE,
                                   (GDBusSignalCallback) on_flash_progress,
                                   object,
                                   NULL );

		g_free (s);

 
		flash_set_filename(flash,"");
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

  id = g_bus_own_name (DBUS_TYPE,
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
