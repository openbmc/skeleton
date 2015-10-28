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
static const gchar* instance_name = "host0";
static const gchar* dbus_name        = "org.openbmc.control.Host";

static GDBusObjectManagerServer *manager = NULL;

GPIO fsi_data     = (GPIO){ "FSI_DATA" };
GPIO fsi_clk      = (GPIO){ "FSI_CLK" };
GPIO fsi_enable   = (GPIO){ "FSI_ENABLE" };
GPIO cronus_sel   = (GPIO){ "CRONUS_SEL" };

static gboolean
on_init         (Control       *control,
                GDBusMethodInvocation  *invocation,
                gpointer                user_data)
{
	control_complete_init(control,invocation);
	return TRUE;
}
static gboolean
on_boot         (ControlHost        *host,
                GDBusMethodInvocation  *invocation,
                gpointer                user_data)
{
	// TODO: Add error checking
	g_print("Do Boot\n");
	int rc = GPIO_OK;

	Control* control = object_get_control((Object*)user_data);
	control_host_complete_boot(host,invocation);
	do {	
		rc |= gpio_open(&fsi_clk);
		rc |= gpio_open(&fsi_data);
		rc |= gpio_open(&fsi_enable);
		rc |= gpio_open(&cronus_sel);
		if (rc!=GPIO_OK) { break; }
		
		rc = gpio_write(&cronus_sel,1);
		//putcfam pu 281c 30000000 -p0
		char a[] = "000011111111110101111000111001100111111111111111111111111111101111111111";
		//putcfam pu 281c B0000000 -p0
		char b[] = "000011111111110101111000111000100111111111111111111111111111101101111111";

		gpio_write(&fsi_enable,1);
		gpio_write(&fsi_clk,1);
		gpio_write(&fsi_data,1);
		gpio_clock_cycle(&fsi_clk,5000);
		gpio_write(&fsi_data,0);
		gpio_clock_cycle(&fsi_clk,256);
		gpio_write(&fsi_data,1);
		gpio_clock_cycle(&fsi_clk,50);
		uint16_t i=0;
		for(i=0;i<strlen(a);i++) {
			gpio_writec(&fsi_data,a[i]);
			gpio_clock_cycle(&fsi_clk,1);
		}
		gpio_write(&fsi_data,1); /* Data standby state */
		gpio_clock_cycle(&fsi_clk,5000);

		for(i=0;i<strlen(b);i++) {
			gpio_writec(&fsi_data,b[i]);
			gpio_clock_cycle(&fsi_clk,1);
		}
		gpio_write(&fsi_data,1); /* Data standby state */
		gpio_clock_cycle(&fsi_clk,2);

	        gpio_write(&fsi_clk,0); /* hold clk low for clock mux */
	        gpio_write(&fsi_enable,0);
	        gpio_clock_cycle(&fsi_clk,16);
	        gpio_write(&fsi_clk,0); /* Data standby state */
	
	} while(0);
	if (rc != GPIO_OK)
	{
		printf("ERROR HostControl: GPIO sequence failed (rc=%d)\n",rc);
	}	
	gpio_close(&fsi_clk);
	gpio_close(&fsi_data);
	gpio_close(&fsi_enable);
	gpio_close(&cronus_sel);

	control_emit_goto_system_state(control,"HOST_BOOTING");
	
	control_host_emit_booted(host);
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
  	manager = g_dbus_object_manager_server_new (dbus_object_path);

		gchar *s;
		s = g_strdup_printf ("%s/%s",dbus_object_path,instance_name);
		object = object_skeleton_new (s);
		g_free (s);
		ControlHost* control_host = control_host_skeleton_new ();
		object_skeleton_set_control_host (object, control_host);
		g_object_unref (control_host);

		Control* control = control_skeleton_new ();
		object_skeleton_set_control (object, control);
		g_object_unref (control);

		//define method callbacks here
		g_signal_connect (control_host,
                  "handle-boot",
                  G_CALLBACK (on_boot),
                  object); /* user_data */

		g_signal_connect (control,
                  "handle-init",
                  G_CALLBACK (on_init),
                  NULL); /* user_data */

		/* Export the object (@manager takes its own reference to @object) */
		g_dbus_object_manager_server_export (manager, G_DBUS_OBJECT_SKELETON (object));
		g_object_unref (object);

	/* Export all objects */
	g_dbus_object_manager_server_set_connection (manager, connection);
	
	gpio_init(connection,&fsi_data);
	gpio_init(connection,&fsi_clk);
	gpio_init(connection,&fsi_enable);
	gpio_init(connection,&cronus_sel);

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
