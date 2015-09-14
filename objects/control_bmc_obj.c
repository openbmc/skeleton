#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "interfaces/openbmc_intf.h"
#include "openbmc.h"
#include "gpio.h"

/* ---------------------------------------------------------------------------------------------------- */
static const gchar* dbus_object_path = "/org/openbmc/control";
static const gchar* dbus_name        = "org.openbmc.control.Bmc";


static GDBusObjectManagerServer *manager = NULL;

static gboolean
on_init (Control          *control,
         GDBusMethodInvocation  *invocation,
         gpointer                user_data)
{
	g_print("BMC init\n");
	// BMC init done here
	/*
	devmem(0x1e789080,0x00000500); //Enable LPC FWH cycles, Enable LPC to AHB bridge
	devmem(0x1e789088,0x30000E00); //32M PNOR
	devmem(0x1e78908C,0xFE0001FF);

	//flash controller
	devmem(0x1e630000,0x00000003);
	devmem(0x1e630004,0x00002404);

	//UART
	devmem(0x1e6e2084,0x00fff0c0);  //Enable UART1
	devmem(0x1e783000,0x00000000);  //Set Baud rate divisor -> 13 (Baud 115200)
	devmem(0x1e783004,0x00000000);  //Set Baud rate divisor -> 13 (Baud 115200)
	devmem(0x1e783008,0x000000c1);  //Disable Parity, 1 stop bit, 8 bits
	devmem(0x1E78909C,0x00000000);  //Set routing UART1 -> COM 1


	//GPIO
	devmem(0x1e6e2070,0x120CE406);
	devmem(0x1e6e2080,0xCB000000);
	devmem(0x1e6e2088,0x01C000FF);
	devmem(0x1e6e208c,0xC1C000FF);
	devmem(0x1e6e2090,0x003FA009);

	devmem(0x1E780000,0x13008CE7);
	devmem(0x1E780004,0x0370E677);
	devmem(0x1E780020,0xDF48F7FF);
	devmem(0x1E780024,0xC738F202);
*/

	control_complete_init(control,invocation);
	control_emit_goto_system_state(control,"STANDBY");
	
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
  		object = object_skeleton_new (s);
  		g_free (s);

		ControlBmc* control_bmc = control_bmc_skeleton_new ();
		object_skeleton_set_control_bmc (object, control_bmc);
		g_object_unref (control_bmc);

		Control* control = control_skeleton_new ();
		object_skeleton_set_control (object, control);
		g_object_unref (control);

		EventLog* event_log = event_log_skeleton_new ();
		object_skeleton_set_event_log (object, event_log);
		g_object_unref (event_log);

		//define method callbacks here
		g_signal_connect (control,
        	            "handle-init",
                	    G_CALLBACK (on_init),
                	    NULL); /* user_data */

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
//  g_print ("Lost the name %s\n", name);
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
