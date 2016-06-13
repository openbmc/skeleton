#include "interfaces/openbmc_intf.h"
#include <stdio.h>
#include <fcntl.h>
#include "openbmc.h"
#include "gpio.h"

/* ------------------------------------------------------------------------- */
static const gchar* dbus_object_path = "/org/openbmc/sensors";
static const gchar* dbus_name = "org.openbmc.sensors.hwmon";

static GDBusObjectManagerServer *manager = NULL;

typedef struct {
	const gchar* filename;
	const gchar* name;
	int poll_interval;
	const gchar* units;
	int scale;
	int fd;
} HWMON;

#define NUM_HWMONS 1

// TODO: Don't hardcode
//Hardcoded for palmetto
HWMON hwmons[NUM_HWMONS] = {
	(HWMON){"/sys/class/hwmon/hwmon0/temp1_input","temperature/ambient",3000,"C",1000},
};

bool
is_hwmon_valid(HWMON* hwmon)
{
	int fd = open(hwmon->filename, O_RDONLY);
	if(fd == -1)
	{
		g_print("ERROR hwmon is not valid: %s\n",hwmon->filename);
		return false;
	}
	close(fd);
	return true;
}

static gboolean
poll_hwmon(gpointer user_data)
{
	Hwmon *hwmon = object_get_hwmon((Object*)user_data);
	SensorValue *sensor = object_get_sensor_value((Object*)user_data);
	const gchar* filename = hwmon_get_sysfs_path(hwmon);

	int fd = open(filename, O_RDONLY);
	if(fd != -1)
	{
		char buf[255];
		if(read(fd,&buf,255) == -1)
		{
			g_print("ERROR: Unable to read value: %s\n",filename);
		} else {
			guint32 scale = hwmon_get_scale(hwmon);
			if(scale == 0)
			{
				g_print("ERROR: Invalid scale value of 0\n");
				scale = 1;

			}
			guint32 value = atoi(buf)/scale;
			GVariant* v = NEW_VARIANT_U(value);
			GVariant* old_value = sensor_value_get_value(sensor);
			bool do_set = false;
			if(old_value == NULL)
			{
				do_set = true;
			}
			else
			{
				if(GET_VARIANT_U(old_value) != value) { do_set = true; }
			}
			if(do_set)
			{
				g_print("Sensor changed: %s,%d\n",filename,value);
				GVariant* v = NEW_VARIANT_U(value);
				const gchar* units = sensor_value_get_units(sensor);
				sensor_value_set_value(sensor,v);
				sensor_value_emit_changed(sensor,v,units);
			}
		}
		close(fd);
	} else {
		g_print("ERROR - hwmons: File %s doesn't exist\n",filename);
	}

	return TRUE;
}

static gboolean
on_set_value(SensorValue *sensor,
		GDBusMethodInvocation *invocation,
		GVariant* v_value,
		gpointer user_data)
{
	Hwmon *hwmon = object_get_hwmon((Object*)user_data);
	const gchar* filename = hwmon_get_sysfs_path(hwmon);

	int fd = open(filename, O_WRONLY);
	if(fd != -1)
	{
		char buf[255];
		guint32 value = GET_VARIANT_U(v_value);
		sprintf(buf,"%d",value);
		if(write(fd, buf, 255) == -1)
		{
			g_print("ERROR: Unable to read value: %s\n",filename);
		}
		close(fd);
	}
	sensor_value_complete_set_value(sensor,invocation);
	return TRUE;
}

static void
on_bus_acquired(GDBusConnection *connection,
		const gchar *name,
		gpointer user_data)
{
	ObjectSkeleton *object;

	cmdline *cmd = user_data;

	manager = g_dbus_object_manager_server_new(dbus_object_path);
	int i = 0;
	for(i=0;i<NUM_HWMONS;i++)
	{
		if(!is_hwmon_valid(&hwmons[i])) { continue; }
		gchar *s;
		s = g_strdup_printf("%s/%s",dbus_object_path,hwmons[i].name);
		object = object_skeleton_new(s);
		g_free(s);

		Hwmon *hwmon = hwmon_skeleton_new();
		object_skeleton_set_hwmon(object, hwmon);
		g_object_unref(hwmon);

		SensorValue *sensor = sensor_value_skeleton_new();
		object_skeleton_set_sensor_value(object, sensor);
		g_object_unref(sensor);

		hwmon_set_sysfs_path(hwmon,hwmons[i].filename);
		hwmon_set_scale(hwmon,hwmons[i].scale);
		sensor_value_set_units(sensor,hwmons[i].units);

		//define method callbacks here
		g_signal_connect(sensor,
				"handle-set-value",
				G_CALLBACK(on_set_value),
				object); /* user_data */


		if(hwmons[i].poll_interval > 0) {
			g_timeout_add(hwmons[i].poll_interval, poll_hwmon, object);
		}
		/* Export the object (@manager takes its own reference to @object) */
		g_dbus_object_manager_server_set_connection(manager, connection);
		g_dbus_object_manager_server_export(manager, G_DBUS_OBJECT_SKELETON(object));
		g_object_unref(object);
	}
}

static void
on_name_acquired(GDBusConnection *connection,
		const gchar *name,
		gpointer user_data)
{
}

static void
on_name_lost(GDBusConnection *connection,
		const gchar *name,
		gpointer user_data)
{
}

gint
main(gint argc, gchar *argv[])
{
	GMainLoop *loop;
	cmdline cmd;
	cmd.argc = argc;
	cmd.argv = argv;

	guint id;
	loop = g_main_loop_new(NULL, FALSE);

	id = g_bus_own_name(DBUS_TYPE,
			dbus_name,
			G_BUS_NAME_OWNER_FLAGS_ALLOW_REPLACEMENT |
			G_BUS_NAME_OWNER_FLAGS_REPLACE,
			on_bus_acquired,
			on_name_acquired,
			on_name_lost,
			&cmd,
			NULL);

	g_main_loop_run(loop);

	g_bus_unown_name(id);
	g_main_loop_unref(loop);
	return 0;
}
