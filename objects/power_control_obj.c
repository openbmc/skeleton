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

/* ---------------------------------------------------------------------------------------------------- */
static const gchar* dbus_object_path = "/org/openbmc/control";
static const gchar* dbus_name        = "org.openbmc.control.Power";

//This object will use these GPIOs
GPIO power_pin    = (GPIO){ "POWER_PIN" };
GPIO pgood        = (GPIO){ "PGOOD" };

static GDBusObjectManagerServer *manager = NULL;

guint tmp_pgood = 0;
guint last_pgood = 0;
guint pgood_timeout_count = 0;

static gboolean poll_pgood(gpointer user_data)
{
	g_print("polling\n");
	ControlPower *control_power = object_get_control_power((Object*)user_data);
	Control* control = object_get_control((Object*)user_data);
	EventLog* event_log = object_get_event_log((Object*)user_data);
	control_emit_heartbeat(control,dbus_name);
	const gchar* obj_path = g_dbus_object_get_object_path((GDBusObject*)user_data);

	guint poll_int = control_get_poll_interval(control);
	if (poll_int == 0)
	{
		event_log_emit_event_log(event_log, LOG_ALERT, "Poll interval cannot be 0");
		return FALSE;
	}
	guint pgood_timeout = control_power_get_pgood_timeout(control_power)/poll_int;

	if (pgood_timeout_count > pgood_timeout)
	{
		event_log_emit_event_log(event_log, LOG_ALERT, "Pgood poll timeout");
		control_power_set_pgood_timeout(control_power,0);
		pgood_timeout_count = 0;
		return TRUE;
	}
	//For simulation, remove
	if (tmp_pgood!=last_pgood) {
		if (tmp_pgood == 1) {
			control_emit_goto_system_state(control,"POWERED_ON");
		} else {
			control_emit_goto_system_state(control,"POWERED_OFF");
		}
	}

	last_pgood = tmp_pgood;
	uint8_t gpio;
	int rc = gpio_read(&pgood,&gpio);
	if (rc == GPIO_OK)
	{
		//if changed, set property and emit signal
		if (gpio != control_power_get_pgood(control_power))
		{
 			control_power_set_pgood(control_power,gpio);
 			if (gpio==0)
 			{
 				control_power_emit_power_lost(control_power);
				control_emit_goto_system_state(control,"POWERED_OFF");
 			}
 			else
 			{
 				control_power_emit_power_good(control_power);
				control_emit_goto_system_state(control,"POWERED_ON");
 			}
		}
	} else {
		event_log_emit_event_log(event_log, LOG_ALERT, "GPIO read error");
		//return FALSE;
	}
	//pgood is not at desired state yet
	if (gpio != control_power_get_state(control_power) &&
		control_power_get_pgood_timeout(control_power) != 0)
	{
		pgood_timeout_count++;
	}
	else 
	{
		pgood_timeout_count = 0;
	}
	return TRUE;
}



static gboolean
on_set_power_state (ControlPower          *pwr,
                GDBusMethodInvocation  *invocation,
                guint                   state,
                gpointer                user_data)
{
	Control* control = object_get_control((Object*)user_data);
	EventLog* event_log = object_get_event_log((Object*)user_data);
	const gchar* obj_path = g_dbus_object_get_object_path((GDBusObject*)user_data);
	if (state > 1)
	{
		g_dbus_method_invocation_return_dbus_error (invocation,
                                                "org.openbmc.ControlPower.Error.Failed",
                                                "Invalid power state");
		return TRUE;
	}
	// return from method call
	control_power_complete_set_power_state(pwr,invocation);
	if (state == control_power_get_state(pwr))
	{
		g_print("Power already at requested state: %d\n",state);
	}
	else
	{
		g_print("Set power state: %d\n",state);
		//temporary until real hardware works
		tmp_pgood = state;
		int error = 0;
		do {
			error = gpio_open(&power_pin);
			if (error != GPIO_OK) {	break;	}
			error = gpio_write(&power_pin,!state);
			if (error != GPIO_OK) {	break;	}
			gpio_close(&power_pin);
			control_power_set_state(pwr,state);
			if (state == 1) {
				control_emit_goto_system_state(control,"POWERING_ON");
			} else {
				control_emit_goto_system_state(control,"POWERING_OFF");
			}
		} while(0);
		if (error != GPIO_OK)
		{
			event_log_emit_event_log(event_log, LOG_ALERT, "GPIO setup error");
		}
	}
	return TRUE;
}

static gboolean
on_init (Control         *control,
         GDBusMethodInvocation  *invocation,
         gpointer                user_data)
{
	guint poll_interval = control_get_poll_interval(control);
	g_timeout_add(poll_interval, poll_pgood, user_data);
	control_complete_init(control,invocation);
	return TRUE;
}

static gboolean
on_get_power_state (ControlPower          *pwr,
                GDBusMethodInvocation  *invocation,
                gpointer                user_data)
{
	guint pgood = control_power_get_pgood(pwr);
	control_power_complete_get_power_state(pwr,invocation,pgood);
	return TRUE;
}

static void 
on_bus_acquired (GDBusConnection *connection,
                 const gchar     *name,
                 gpointer         user_data)
{
	ObjectSkeleton *object;
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
		g_print("%s\n",s);
  		object = object_skeleton_new (s);
  		g_free (s);

		ControlPower* control_power = control_power_skeleton_new ();
		object_skeleton_set_control_power (object, control_power);
		g_object_unref (control_power);

		Control* control = control_skeleton_new ();
		object_skeleton_set_control (object, control);
		g_object_unref (control);

		EventLog* event_log = event_log_skeleton_new ();
		object_skeleton_set_event_log (object, event_log);
		g_object_unref (event_log);

		//define method callbacks here
		g_signal_connect (control_power,
        	            "handle-set-power-state",
                	    G_CALLBACK (on_set_power_state),
                	    object); /* user_data */

		g_signal_connect (control_power,
                	    "handle-get-power-state",
                	    G_CALLBACK (on_get_power_state),
                	    NULL); /* user_data */

		g_signal_connect (control,
                	    "handle-init",
                	    G_CALLBACK (on_init),
                	    object); /* user_data */



		/* Export the object (@manager takes its own reference to @object) */
		g_dbus_object_manager_server_export (manager, G_DBUS_OBJECT_SKELETON (object));
		g_object_unref (object);
	}
	/* Export all objects */
	g_dbus_object_manager_server_set_connection (manager, connection);

	// get gpio device paths
	do {
		int rc = GPIO_OK;
		rc = gpio_init(connection,&power_pin);
		if (rc != GPIO_OK) { break; }
		rc = gpio_init(connection,&pgood);
		if (rc != GPIO_OK) { break; }
		rc = gpio_open(&pgood);
		if (rc != GPIO_OK) { break; }
	} while(0);
}

static void
on_name_acquired (GDBusConnection *connection,
                  const gchar     *name,
                  gpointer         user_data)
{
  //g_print ("Acquired the name %s\n", name);
}

static void
on_name_lost (GDBusConnection *connection,
              const gchar     *name,
              gpointer         user_data)
{
  //g_print ("Lost the name %s\n", name);
}




/*----------------------------------------------------------------*/
/* Main Event Loop                                                */

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
