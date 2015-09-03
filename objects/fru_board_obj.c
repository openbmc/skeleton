#include "interfaces/fru.h"
#include "openbmc.h"


/* ---------------------------------------------------------------------------------------------------- */

static const gchar* dbus_object_path = "/org/openbmc/frus/Board";
static const gchar* dbus_name        = "org.openbmc.frus.Board";
static const guint poll_interval = 5000;
static guint heartbeat = 0;

static GDBusObjectManagerServer *manager = NULL;

static gboolean
on_init         (Fru *fru,
                GDBusMethodInvocation  *invocation,
                gpointer                user_data)
{

	FruEeprom *eeprom = object_get_fru_eeprom((Object*)user_data);

	const gchar* dev_path = fru_eeprom_get_i2c_dev_path(eeprom);
	const gchar* addr = fru_eeprom_get_i2c_address(eeprom);
	g_print("Reading VPD EERPROM at: %s, %s\n",dev_path, addr);
	fru_complete_init(fru,invocation);
	fru_set_part_num(fru,"test part num");
	
	// add eeprom read code here
	fru_emit_cache_me(fru,dbus_name);

	return TRUE;
}

static void 
on_bus_acquired (GDBusConnection *connection,
                 const gchar     *name,
                 gpointer         user_data)
{
  	g_print ("Acquired a message bus connection: %s\n",name);

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
		ObjectSkeleton *object = object_skeleton_new (s);
		g_free (s);

		Fru *fru = fru_skeleton_new ();
  		object_skeleton_set_fru (object, fru);
  		g_object_unref (fru);

		FruEeprom *eeprom = fru_eeprom_skeleton_new ();
  		object_skeleton_set_fru_eeprom (object, eeprom);
  		g_object_unref (eeprom);

		g_signal_connect (fru,
                    "handle-init",
                    G_CALLBACK (on_init),
                    object); /* user_data */
 

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
