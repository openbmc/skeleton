#include "interfaces/openbmc_intf.h"
#include "openbmc.h"


/* ---------------------------------------------------------------------------------------------------- */

static const gchar* dbus_object_path = "/org/openbmc/sensors";
static const gchar* dbus_name        = "org.openbmc.sensors.Occ";
static guint heartbeat = 0;

static GDBusObjectManagerServer *manager = NULL;
static gchar *instance_name = NULL;

static gboolean poll_occ(gpointer user_data)
{
	//g_dbus_object_get_object_path(G_DBUS_OBJECT(user_data)),
	Occ* occ = object_get_occ((Object*)user_data);

	gchar *s;
	s = g_strdup_printf ("%s/Temperature/P8_%s_Core_%d",
			dbus_object_path,occ_get_instance_name(occ),1);
	g_print("%s\n",s);

	GDBusInterface* interface = g_dbus_object_manager_get_interface((GDBusObjectManager*)manager,s,
		"org.openbmc.SensorValue");

	if (interface != NULL)
	{
		SensorValue* sensor = (SensorValue*) interface;
		GVariant *value = NEW_VARIANT_U(10);
		sensor_value_set_value(sensor,value);
		const gchar* units = sensor_value_get_units(sensor);
		sensor_value_emit_changed(sensor,sensor_value_get_value(sensor),units);
	}
	g_free (s);
	//g_free(interface);
	return TRUE;
}

static gboolean
on_init (Occ         *occ,
         GDBusMethodInvocation  *invocation,
         gpointer                user_data)
{
	guint poll_interval = occ_get_poll_interval(occ);
	g_timeout_add(poll_interval, poll_occ, user_data);
	occ_complete_init(occ,invocation);
	return TRUE;
}
static gboolean
on_init_sensor (SensorValue         *sensor,
         GDBusMethodInvocation  *invocation,
         gpointer                user_data)
{
	//must implement init method
	sensor_value_complete_init(sensor,invocation);
	return TRUE;
}



static void 
on_bus_acquired (GDBusConnection *connection,
                 const gchar     *name,
                 gpointer         user_data)
{
  	//g_print ("Acquired a message bus connection: %s\n",name);

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

		Occ *occ = occ_skeleton_new ();
  		object_skeleton_set_occ (object, occ);
  		g_object_unref (occ);
		occ_set_instance_name(occ,cmd->argv[i]);

		SensorI2c *i2c = sensor_i2c_skeleton_new ();
  		object_skeleton_set_sensor_i2c (object, i2c);
  		g_object_unref (i2c);

		g_signal_connect (occ,
               	    "handle-init",
               	    G_CALLBACK (on_init),
               	    object); /* user_data */
	
		//g_timeout_add(3000, poll_occ, object);

  		/* Export the object (@manager takes its own reference to @object) */
  		g_dbus_object_manager_server_export (manager, G_DBUS_OBJECT_SKELETON (object));
  		g_object_unref (object);
	
		int c;	
		for (c=0;c<12;c++)
		{
 			s = g_strdup_printf ("%s/Temperature/P8_%s_Core_%d",dbus_object_path,cmd->argv[i],c);
			ObjectSkeleton *object = object_skeleton_new (s);
			g_free (s);

			SensorValue *sensor = sensor_value_skeleton_new ();
  			object_skeleton_set_sensor_value (object, sensor);
  			g_object_unref (sensor);
			GVariant* v_new_value = NEW_VARIANT_U(c);
			sensor_value_set_value(sensor,v_new_value);
			sensor_value_set_units(sensor,"C");

			g_signal_connect (sensor,
        	       	    "handle-init",
               		    G_CALLBACK (on_init_sensor),
               	    	    NULL); /* user_data */

			//emit changed signal so sensor manager sees initial value
			sensor_value_emit_changed(sensor,sensor_value_get_value(sensor),sensor_value_get_units(sensor));
  			/* Export the object (@manager takes its own reference to @object) */
  			g_dbus_object_manager_server_export (manager, G_DBUS_OBJECT_SKELETON (object));
  			g_object_unref (object);

		}
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
	g_print("shutting down: %s\n",name);
	cmdline *cmd = user_data;
	g_main_loop_quit(cmd->loop);	
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
  cmd.loop = loop;

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
