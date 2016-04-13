#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <syslog.h>
#include "interfaces/openbmc_intf.h"
#include "openbmc.h"
#include "gpio.h"

/* ------------------------------------------------------------------------- */
static const gchar* dbus_object_path = "/org/openbmc/control";
static const gchar* instance_name = "power0";
static const gchar* dbus_name = "org.openbmc.control.Power";

//This object will use these GPIOs
GPIO power_pin    = (GPIO){ "POWER_PIN" };
GPIO pgood        = (GPIO){ "PGOOD" };
GPIO usb_reset    = (GPIO){ "USB_RESET" };
GPIO pcie_reset   = (GPIO){ "PCIE_RESET" };


static GDBusObjectManagerServer *manager = NULL;

time_t pgood_timeout_start = 0;

// TODO:  Change to interrupt driven instead of polling
static gboolean
poll_pgood(gpointer user_data)
{
	ControlPower *control_power = object_get_control_power((Object*)user_data);
	Control* control = object_get_control((Object*)user_data);

	//send the heartbeat
	guint poll_int = control_get_poll_interval(control);
	if(poll_int == 0)
	{
		printf("ERROR PowerControl: Poll interval cannot be 0\n");
		return FALSE;
	}
	//handle timeout
	time_t current_time = time(NULL);
	if(difftime(current_time,pgood_timeout_start) > control_power_get_pgood_timeout(control_power)
			&& pgood_timeout_start != 0)
	{
		printf("ERROR PowerControl: Pgood poll timeout\n");
		// set timeout to 0 so timeout doesn't happen again
		control_power_set_pgood_timeout(control_power,0);
		pgood_timeout_start = 0;
		return TRUE;
	}
	uint8_t gpio;

	int rc = gpio_open(&pgood);
	rc = gpio_read(&pgood,&gpio);
	gpio_close(&pgood);
	if(rc == GPIO_OK)
	{
		//if changed, set property and emit signal
		if(gpio != control_power_get_pgood(control_power))
		{
			control_power_set_pgood(control_power,gpio);
			if(gpio==0)
			{
				control_power_emit_power_lost(control_power);
				control_emit_goto_system_state(control,"HOST_POWERED_OFF");
				rc = gpio_open(&pcie_reset);
				rc = gpio_write(&pcie_reset,0);
				gpio_close(&pcie_reset);

				rc = gpio_open(&usb_reset);
				rc = gpio_write(&usb_reset,0);
				gpio_close(&usb_reset);

			}
			else
			{
				control_power_emit_power_good(control_power);
				control_emit_goto_system_state(control,"HOST_POWERED_ON");
				rc = gpio_open(&pcie_reset);
				rc = gpio_write(&pcie_reset,1);
				gpio_close(&pcie_reset);

				rc = gpio_open(&usb_reset);
				rc = gpio_write(&usb_reset,1);
				gpio_close(&usb_reset);
			}
		}
	} else {
		printf("ERROR PowerControl: GPIO read error (gpio=%s,rc=%d)\n",pgood.name,rc);
		//return false so poll won't get called anymore
		return FALSE;
	}
	//pgood is not at desired state yet
	if(gpio != control_power_get_state(control_power) &&
			control_power_get_pgood_timeout(control_power) > 0)
	{
		if(pgood_timeout_start == 0 ) {
			pgood_timeout_start = current_time;
		}
	}
	else
	{
		pgood_timeout_start = 0;
	}
	return TRUE;
}

static gboolean
on_set_power_state(ControlPower *pwr,
		GDBusMethodInvocation *invocation,
		guint state,
		gpointer user_data)
{
	Control* control = object_get_control((Object*)user_data);
	if(state > 1)
	{
		g_dbus_method_invocation_return_dbus_error(invocation,
				"org.openbmc.ControlPower.Error.Failed",
				"Invalid power state");
		return TRUE;
	}
	// return from method call
	control_power_complete_set_power_state(pwr,invocation);
	if(state == control_power_get_state(pwr))
	{
		g_print("Power already at requested state: %d\n",state);
	}
	else
	{
		int error = 0;
		do {
			if(state == 1) {
				control_emit_goto_system_state(control,"HOST_POWERING_ON");
			} else {
				control_emit_goto_system_state(control,"HOST_POWERING_OFF");
			}
			error = gpio_open(&power_pin);
			if(error != GPIO_OK) { break;	}
			error = gpio_write(&power_pin,!state);
			if(error != GPIO_OK) { break;	}
			gpio_close(&power_pin);
			control_power_set_state(pwr,state);
		} while(0);
		if(error != GPIO_OK)
		{
			printf("ERROR PowerControl: GPIO set power state (rc=%d)\n",error);
		}
	}
	return TRUE;
}

static gboolean
on_init(Control *control,
		GDBusMethodInvocation *invocation,
		gpointer user_data)
{
	pgood_timeout_start = 0;
	//guint poll_interval = control_get_poll_interval(control);
	//g_timeout_add(poll_interval, poll_pgood, user_data);
	control_complete_init(control,invocation);
	return TRUE;
}

static gboolean
on_get_power_state(ControlPower *pwr,
		GDBusMethodInvocation *invocation,
		gpointer user_data)
{
	guint pgood = control_power_get_pgood(pwr);
	control_power_complete_get_power_state(pwr,invocation,pgood);
	return TRUE;
}

static void
on_bus_acquired(GDBusConnection *connection,
		const gchar *name,
		gpointer user_data)
{
	ObjectSkeleton *object;
	cmdline *cmd = user_data;
	if(cmd->argc < 3)
	{
		g_print("Usage: power_control.exe [poll interval] [timeout]\n");
		return;
	}
	manager = g_dbus_object_manager_server_new(dbus_object_path);
	gchar *s;
	s = g_strdup_printf("%s/%s",dbus_object_path,instance_name);
	object = object_skeleton_new(s);
	g_free(s);

	ControlPower* control_power = control_power_skeleton_new();
	object_skeleton_set_control_power(object, control_power);
	g_object_unref(control_power);

	Control* control = control_skeleton_new();
	object_skeleton_set_control(object, control);
	g_object_unref(control);

	//define method callbacks here
	g_signal_connect(control_power,
			"handle-set-power-state",
			G_CALLBACK(on_set_power_state),
			object); /* user_data */

	g_signal_connect(control_power,
			"handle-get-power-state",
			G_CALLBACK(on_get_power_state),
			NULL); /* user_data */

	g_signal_connect(control,
			"handle-init",
			G_CALLBACK(on_init),
			object); /* user_data */


	/* Export the object (@manager takes its own reference to @object) */
	g_dbus_object_manager_server_set_connection(manager, connection);
	g_dbus_object_manager_server_export(manager, G_DBUS_OBJECT_SKELETON(object));
	g_object_unref(object);

	// get gpio device paths
	int rc = GPIO_OK;
	do {
		rc = gpio_init(connection,&power_pin);
		if(rc != GPIO_OK) { break; }
		rc = gpio_init(connection,&pgood);
		if(rc != GPIO_OK) { break; }
		rc = gpio_init(connection,&pcie_reset);
		if(rc != GPIO_OK) { break; }
		rc = gpio_init(connection,&usb_reset);
		if(rc != GPIO_OK) { break; }

		uint8_t gpio;
		rc = gpio_open(&pgood);
		if(rc != GPIO_OK) { break; }
		rc = gpio_read(&pgood,&gpio);
		if(rc != GPIO_OK) { break; }
		gpio_close(&pgood);
		control_power_set_pgood(control_power,gpio);
		control_power_set_state(control_power,gpio);
		printf("Pgood state: %d\n",gpio);

	} while(0);
	if(rc != GPIO_OK)
	{
		printf("ERROR PowerControl: GPIO setup (rc=%d)\n",rc);
	}
	//start poll
	pgood_timeout_start = 0;
	int poll_interval = atoi(cmd->argv[1]);
	int pgood_timeout = atoi(cmd->argv[2]);
	if(poll_interval < 1000 || pgood_timeout <5) {
		printf("ERROR PowerControl: poll_interval < 1000 or pgood_timeout < 5\n");
	} else {
		control_set_poll_interval(control,poll_interval);
		control_power_set_pgood_timeout(control_power,pgood_timeout);
		g_timeout_add(poll_interval, poll_pgood, object);
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

/*----------------------------------------------------------------*/
/* Main Event Loop                                                */

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
