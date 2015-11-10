#include "interfaces/openbmc_intf.h"
#include <stdio.h>
#include <fcntl.h>
#include "openbmc.h"
#include "gpio.h"
#include "object_mapper.h"

/* ---------------------------------------------------------------------------------------------------- */
static const gchar* dbus_object_path = "/org/openbmc/sensors";
static const gchar* dbus_name        = "org.openbmc.sensors.hwmon";

static GDBusObjectManagerServer *manager = NULL;

typedef struct {
  gchar* filename;
  gchar* name;
  int poll_interval;
  int fd;
} HWMON;

#define  NUM_HWMONS 7

// TODO: Don't hardcode
//Hardcoded for barreleye
HWMON hwmons[NUM_HWMONS] = { 
	(HWMON){"/sys/class/hwmon/hwmon0/temp1_input","temperature/ambient",3000},
	(HWMON){"/sys/class/hwmon/hwmon1/pwm1","speed/fan0",30000},
	(HWMON){"/sys/class/hwmon/hwmon1/pwm2","speed/fan1",30000},
	(HWMON){"/sys/class/hwmon/hwmon1/pwm3","speed/fan2",30000},
	(HWMON){"/sys/class/hwmon/hwmon2/pwm1","speed/fan3",30000},
	(HWMON){"/sys/class/hwmon/hwmon2/pwm2","speed/fan4",30000},
	(HWMON){"/sys/class/hwmon/hwmon2/pwm3","speed/fan5",30000},
};

// Gets the gpio device path from gpio manager object
int hwmon_init(GDBusConnection *connection, HWMON* hwmon)
{
	int rc = GPIO_OK;
	GDBusProxy *proxy;
	GError *error;
	GVariant *result;

	error = NULL;
	g_assert_no_error (error);
	error = NULL;

	proxy = g_dbus_proxy_new_sync (connection,
                                 G_DBUS_PROXY_FLAGS_NONE,
                                 NULL,                      /* GDBusInterfaceInfo */
                                 "org.openbmc.managers.System", /* name */
                                 "/org/openbmc/managers/System", /* object path */
                                 "org.openbmc.managers.System",        /* interface */
                                 NULL, /* GCancellable */
                                 &error);
	if (error != NULL) {
		return 1;
	}

	result = g_dbus_proxy_call_sync (proxy,
                                   "hwmonInit",
                                   g_variant_new ("(s)", hwmon->filename),
                                   G_DBUS_CALL_FLAGS_NONE,
                                   -1,
                                   NULL,
                                   &error);
  
	if (error != NULL) {
		return 1;
	}
	g_assert (result != NULL);
	g_variant_get (result, "(&si)", &hwmon->name,&hwmon->poll_interval);
	g_print("HWMON Lookup:  %s = %s,%d\n",hwmon->filename,hwmon->name,hwmon->poll_interval);
}	

static gboolean poll_hwmon(gpointer user_data)
{
	Hwmon *hwmon = object_get_hwmon((Object*)user_data);
	SensorValue *sensor = object_get_sensor_value((Object*)user_data);
	const gchar* filename = hwmon_get_sysfs_path(hwmon);

	int fd = open(filename, O_RDONLY);
	if (fd != -1)
	{
		char buf[255];
		if (read(fd,&buf,255) == -1)
		{
			g_print("ERROR: Unable to read value: %s\n",filename);
		} else {
			guint32 value = atoi(buf);
			//g_print("%s = %d\n",filename,value);
			GVariant* v = NEW_VARIANT_U(value);
			sensor_value_set_value(sensor,v);
		}
		close(fd);
	} else {
		g_print("ERROR - hwmons: File %s doesn't exist\n",filename);
	}

	return TRUE;
}
static gboolean
on_set_value   (SensorValue            *sensor,
                GDBusMethodInvocation  *invocation,
		GVariant*                v_value,	
                gpointer                user_data)
{
	Hwmon *hwmon = object_get_hwmon((Object*)user_data);
	const gchar* filename = hwmon_get_sysfs_path(hwmon);

	int fd = open(filename, O_WRONLY);
	if (fd != -1)
	{
		char buf[255];
		guint32 value = GET_VARIANT_U(v_value);
		sprintf(buf,"%d",value);
		if (write(fd, buf, 255) == -1)
		{
			g_print("ERROR: Unable to read value: %s\n",filename);
		}
		close(fd);
	}
	sensor_value_complete_set_value(sensor,invocation);
	return TRUE;
}



static void 
on_bus_acquired (GDBusConnection *connection,
                 const gchar     *name,
                 gpointer         user_data)
{
	ObjectSkeleton *object;

	cmdline *cmd = user_data;


	manager = g_dbus_object_manager_server_new (dbus_object_path);
	int i = 0;
	for (i=0;i<NUM_HWMONS;i++)
  	{
		//hwmon_init(connection,&hwmons[i]);

		gchar *s;
		s = g_strdup_printf ("%s/%s",dbus_object_path,hwmons[i].name);
		object = object_skeleton_new (s);
		g_free (s);

		Hwmon *hwmon = hwmon_skeleton_new ();
		object_skeleton_set_hwmon (object, hwmon);
		g_object_unref (hwmon);

		SensorValue *sensor = sensor_value_skeleton_new ();
		object_skeleton_set_sensor_value (object, sensor);
		g_object_unref (sensor);

		ObjectMapper* mapper = object_mapper_skeleton_new ();
		object_skeleton_set_object_mapper (object, mapper);
		g_object_unref (mapper);

		hwmon_set_sysfs_path(hwmon,hwmons[i].filename);

		//define method callbacks here
		g_signal_connect (sensor,
       	            "handle-set-value",
               	    G_CALLBACK (on_set_value),
               	    object); /* user_data */
		

		if (hwmons[i].poll_interval > 0) {
			g_timeout_add(hwmons[i].poll_interval, poll_hwmon, object);
		}
		/* Export the object (@manager takes its own reference to @object) */
		g_dbus_object_manager_server_export (manager, G_DBUS_OBJECT_SKELETON (object));
		g_object_unref (object);
	}
	/* Export all objects */
 	g_dbus_object_manager_server_set_connection (manager, connection);
	emit_object_added((GDBusObjectManager*)manager); 
}


static void
on_name_acquired (GDBusConnection *connection,
                  const gchar     *name,
                  gpointer         user_data)
{
}

static void
on_name_lost (GDBusConnection *connection,
              const gchar     *name,
              gpointer         user_data)
{
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
