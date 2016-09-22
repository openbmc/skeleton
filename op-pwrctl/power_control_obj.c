#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <syslog.h>
#include <openbmc_intf.h>
#include <openbmc.h>
#include <gpio.h>
#include <power_gpio.h>

/* ------------------------------------------------------------------------- */
static const gchar* dbus_object_path = "/org/openbmc/control";
static const gchar* instance_name = "power0";
static const gchar* dbus_name = "org.openbmc.control.Power";

static PowerGpio g_power_gpio;

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
		g_print("ERROR PowerControl: Poll interval cannot be 0\n");
		return FALSE;
	}
	//handle timeout
	time_t current_time = time(NULL);
	if(difftime(current_time,pgood_timeout_start) > control_power_get_pgood_timeout(control_power)
			&& pgood_timeout_start != 0)
	{
		g_print("ERROR PowerControl: Pgood poll timeout\n");
		// set timeout to 0 so timeout doesn't happen again
		control_power_set_pgood_timeout(control_power,0);
		pgood_timeout_start = 0;
		return TRUE;
	}
	uint8_t pgood_state;

	int rc = gpio_open(&g_power_gpio.power_good_in);
	rc = gpio_read(&g_power_gpio.power_good_in, &pgood_state);
	gpio_close(&g_power_gpio.power_good_in);
	if(rc == GPIO_OK)
	{
		//if changed, set property and emit signal
		if(pgood_state != control_power_get_pgood(control_power))
		{
			int i;
			uint8_t reset_state;
			control_power_set_pgood(control_power, pgood_state);
			if(pgood_state == 0)
			{
				control_power_emit_power_lost(control_power);
				control_emit_goto_system_state(control,"HOST_POWERED_OFF");
			}
			else
			{
				control_power_emit_power_good(control_power);
				control_emit_goto_system_state(control,"HOST_POWERED_ON");
			}

			for(i = 0; i < g_power_gpio.num_reset_outs; i++)
			{
				GPIO *reset_out = &g_power_gpio.reset_outs[i];
				rc = gpio_open(reset_out);
				if(rc != GPIO_OK)
				{
					g_print("ERROR PowerControl: GPIO open error (gpio=%s,rc=%d)\n",
							reset_out->name, rc);
					continue;
				}

				reset_state = pgood_state ^ g_power_gpio.reset_pols[i];
				g_print("PowerControl: setting reset %s to %d\n", reset_out->name,
						(int)reset_state);
				gpio_write(reset_out, reset_state);
				gpio_close(reset_out);
			}
		}
	} else {
		g_print("ERROR PowerControl: GPIO read error (gpio=%s,rc=%d)\n",
				g_power_gpio.power_good_in.name, rc);
		//return false so poll won't get called anymore
		return FALSE;
	}
	//pgood is not at desired state yet
	if(pgood_state != control_power_get_state(control_power) &&
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
			int i;
			uint8_t power_up_out;
			if(state == 1) {
				control_emit_goto_system_state(control,"HOST_POWERING_ON");
			} else {
				control_emit_goto_system_state(control,"HOST_POWERING_OFF");
			}
			for (i = 0; i < g_power_gpio.num_power_up_outs; i++) {
				GPIO *power_pin = &g_power_gpio.power_up_outs[i];
				error = gpio_open(power_pin);
				if(error != GPIO_OK) {
					g_print("ERROR PowerControl: GPIO open error (gpio=%s,rc=%d)\n",
							g_power_gpio.power_up_outs[i].name, error);
					continue;
				}
				power_up_out = state ^ g_power_gpio.power_up_pols[i];
				g_print("PowerControl: setting power up %s to %d\n",
						g_power_gpio.power_up_outs[i].name, (int)power_up_out);
				error = gpio_write(power_pin, power_up_out);
				if(error != GPIO_OK) {
					continue;
				}
				gpio_close(power_pin);
			}
			if(error != GPIO_OK) { break;	}
			control_power_set_state(pwr,state);
		} while(0);
		if(error != GPIO_OK)
		{
			g_print("ERROR PowerControl: GPIO set power state (rc=%d)\n",error);
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

static int
set_up_gpio(GDBusConnection *connection,
		PowerGpio *power_gpio,
		ControlPower* control_power)
{
	int error = GPIO_OK;
	int rc;
	int i;
	uint8_t pgood_state;

	// get gpio device paths
	rc = gpio_init(connection, &power_gpio->power_good_in);
	if(rc != GPIO_OK) {
		error = rc;
	}
	for(int i = 0; i < power_gpio->num_power_up_outs; i++) {
		rc = gpio_init(connection, &power_gpio->power_up_outs[i]);
		if(rc != GPIO_OK) {
			error = rc;
		}
	}
	for(int i = 0; i < power_gpio->num_reset_outs; i++) {
		rc = gpio_init(connection, &power_gpio->reset_outs[i]);
		if(rc != GPIO_OK) {
			error = rc;
		}
	}

	rc = gpio_open(&power_gpio->power_good_in);
	if(rc != GPIO_OK) {
		return rc;
	}
	rc = gpio_read(&power_gpio->power_good_in, &pgood_state);
	if(rc != GPIO_OK) {
		return rc;
	}
	gpio_close(&power_gpio->power_good_in);
	control_power_set_pgood(control_power, pgood_state);
	control_power_set_state(control_power, pgood_state);
	g_print("Pgood state: %d\n", pgood_state);

	return error;
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

	if(read_power_gpio(connection, &g_power_gpio) != TRUE) {
		g_print("ERROR PowerControl: could not read power GPIO configuration\n");
	}

	int rc = set_up_gpio(connection, &g_power_gpio, control_power);
	if(rc != GPIO_OK) {
		g_print("ERROR PowerControl: GPIO setup (rc=%d)\n",rc);
	}
	//start poll
	pgood_timeout_start = 0;
	int poll_interval = atoi(cmd->argv[1]);
	int pgood_timeout = atoi(cmd->argv[2]);
	if(poll_interval < 1000 || pgood_timeout <5) {
		g_print("ERROR PowerControl: poll_interval < 1000 or pgood_timeout < 5\n");
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
	free_power_gpio(&g_power_gpio);
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
