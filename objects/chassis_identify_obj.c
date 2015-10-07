#include "interfaces/openbmc_intf.h"
#include <stdio.h>
#include "openbmc.h"
#include "gpio.h"

/* ---------------------------------------------------------------------------------------------------- */
static const gchar* dbus_object_path = "/org/openbmc/leds";
static const gchar* dbus_name        = "org.openbmc.leds.ChassisIdentify";

static GDBusObjectManagerServer *manager = NULL;

GPIO led_gpio = (GPIO){"IDENTIFY"};

static gboolean
on_set_on       (Led          *led,
                GDBusMethodInvocation  *invocation,
                gpointer                user_data)
{
	printf("Turn on chassis identify led\n");
	led_complete_set_on(led,invocation);
	int rc = GPIO_OK;
	do {
		rc = gpio_open(&led_gpio);
		if (rc != GPIO_OK) { break; }
		rc = gpio_write(&led_gpio,1); 
		if (rc != GPIO_OK) { break; }
	} while(0);
	gpio_close(&led_gpio);
	if (rc != GPIO_OK)
	{
		printf("ERROR ChassisIdentify: GPIO error %s (rc=%d)\n",led_gpio.name,rc);
	}

	return TRUE;

}

static gboolean
on_set_off       (Led          *led,
                GDBusMethodInvocation  *invocation,
                gpointer                user_data)
{
	g_print("Turn off chassis identify led\n");
	led_complete_set_off(led,invocation);
	int rc = GPIO_OK;
	do {
		rc = gpio_open(&led_gpio);
		if (rc != GPIO_OK) { break; }
		rc = gpio_write(&led_gpio,0); 
		if (rc != GPIO_OK) { break; }
	} while(0);
	gpio_close(&led_gpio);
	if (rc != GPIO_OK)
	{
		printf("ERROR ChassisIdentify: GPIO error %s (rc=%d)\n",led_gpio.name,rc);
	}
	return TRUE;
}

static void 
on_bus_acquired (GDBusConnection *connection,
                 const gchar     *name,
                 gpointer         user_data)
{
	ObjectSkeleton *object;

	cmdline *cmd = user_data;
	if (cmd->argc < 2)
	{
		g_print("No objects created.  Put object name(s) on command line\n");
		return;
	}	

	manager = g_dbus_object_manager_server_new (dbus_object_path);
	int i = 0;
	for (i=1;i<cmd->argc;i++)
  	{
		gchar *s;
		s = g_strdup_printf ("%s/%s",dbus_object_path,cmd->argv[i]);
		object = object_skeleton_new (s);
		g_free (s);

		Led *led = led_skeleton_new ();
		object_skeleton_set_led (object, led);
		g_object_unref (led);

		//define method callbacks
		g_signal_connect (led,
                    "handle-set-on",
                    G_CALLBACK (on_set_on),
                    NULL); /* user_data */
		g_signal_connect (led,
                    "handle-set-off",
                    G_CALLBACK (on_set_off),
                    NULL);

		led_set_color(led,0);
		led_set_function(led,"CHASSIS_IDENTIFY");
 
		/* Export the object (@manager takes its own reference to @object) */
		g_dbus_object_manager_server_export (manager, G_DBUS_OBJECT_SKELETON (object));
		g_object_unref (object);
	}
	/* Export all objects */
 	g_dbus_object_manager_server_set_connection (manager, connection);
	gpio_init(connection,&led_gpio);

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
