#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "interfaces/power_control.h"
#include "openbmc.h"
#include "gpio.h"

/* ---------------------------------------------------------------------------------------------------- */
static const gchar* dbus_object_path = "/org/openbmc/control/Power";
static const gchar* dbus_name        = "org.openbmc.control.Power";

GPIO power_pin    = (GPIO){ "POWER_PIN" };
GPIO pgood        = (GPIO){ "PGOOD" };

static GDBusObjectManagerServer *manager = NULL;

static gboolean
on_set_power_state (ControlPower          *pwr,
                GDBusMethodInvocation  *invocation,
                guint                   state,
                gpointer                user_data)
{
	if (state > 1)
	{
		g_dbus_method_invocation_return_dbus_error (invocation,
                                                "org.openbmc.ControlPower.Error.Failed",
                                                "Invalid power state");
		return TRUE;
	}
	if (state == control_power_get_state(pwr))
	{
		g_dbus_method_invocation_return_dbus_error (invocation,
                                                "org.openbmc.ControlPower.Error.Failed",
                                                "Power Control is already at requested state");
		return TRUE;     
	}

	//go ahead and return from method call
	control_power_complete_set_power_state(pwr,invocation);

	g_print("Set power state: %d\n",state);
	gpio_open(&power_pin);
	gpio_write(&power_pin,!state); 
	gpio_close(&power_pin);

	control_power_set_state(pwr,state);
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
  		object = object_skeleton_new (s);
  		g_free (s);

		ControlPower* control_power = control_power_skeleton_new ();
		object_skeleton_set_control_power (object, control_power);
		g_object_unref (control_power);

		//define method callbacks here
		g_signal_connect (control_power,
        	            "handle-set-power-state",
                	    G_CALLBACK (on_set_power_state),
                	    NULL); /* user_data */

		g_signal_connect (control_power,
                	    "handle-get-power-state",
                	    G_CALLBACK (on_get_power_state),
                	    NULL); /* user_data */

		/* Export the object (@manager takes its own reference to @object) */
		g_dbus_object_manager_server_export (manager, G_DBUS_OBJECT_SKELETON (object));
		g_object_unref (object);
	}
	/* Export all objects */
	g_dbus_object_manager_server_set_connection (manager, connection);

	// get gpio device paths
	gpio_init(connection,&power_pin);
	gpio_init(connection,&pgood);
	gpio_open(&pgood);
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

static gboolean poll_pgood(gpointer user_data)
{
	ControlPower *control_power = object_get_control_power((Object*)user_data);

	if (pgood.fd >= 0)
	{
		uint8_t gpio = gpio_read(&pgood);

		//if changed, set property and emit signal
		if (gpio != control_power_get_pgood(control_power))
		{
 			control_power_set_pgood(control_power,gpio);
 			if (gpio==0)
 			{
 				control_power_emit_power_lost(control_power);
 			}
 			else
 			{
 				control_power_emit_power_good(control_power);
 			}
		}
	}
	else
	{
		//TODO: error handling
		printf("Unable to read pgood\n");
	}
	return TRUE;
}



/*----------------------------------------------------------------*/
/* Main Event Loop                                                */

gint
main (gint argc, gchar *argv[])
{
  GMainLoop *loop;

  guint id;
  //g_type_init ();
  loop = g_main_loop_new (NULL, FALSE);

  id = g_bus_own_name (G_BUS_TYPE_SESSION,
                       dbus_name,
                       G_BUS_NAME_OWNER_FLAGS_ALLOW_REPLACEMENT |
                       G_BUS_NAME_OWNER_FLAGS_REPLACE,
                       on_bus_acquired,
                       on_name_acquired,
                       on_name_lost,
                       loop,
                       NULL);

  g_timeout_add(5000, poll_pgood, NULL);
  g_main_loop_run (loop);
  
  g_bus_unown_name (id);
  g_main_loop_unref (loop);
  return 0;
}
