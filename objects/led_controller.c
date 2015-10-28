#include "interfaces/openbmc_intf.h"
#include <stdio.h>
#include "openbmc.h"
#include "gpio.h"

/* ---------------------------------------------------------------------------------------------------- */
static const gchar* dbus_object_path = "/org/openbmc/control/led";
static const gchar* dbus_name        = "org.openbmc.control.led";

static GDBusObjectManagerServer *manager = NULL;

#define  NUM_GPIO 2

GPIO led_gpio[NUM_GPIO] = { 
	(GPIO){"IDENTIFY"},
	(GPIO){"BMC_READY"}
};


static gboolean
on_set_on       (Led          *led,
                GDBusMethodInvocation  *invocation,
                gpointer                user_data)
{
	GPIO* mygpio = (GPIO*)user_data;
	g_print("Turn on LED: %s\n",mygpio->name);
	led_complete_set_on(led,invocation);
	int rc = GPIO_OK;
	do {
		rc = gpio_open(mygpio);
		if (rc != GPIO_OK) { break; }
		rc = gpio_write(mygpio,0); 
		if (rc != GPIO_OK) { break; }
	} while(0);
	gpio_close(mygpio);
	if (rc != GPIO_OK)
	{
		printf("ERROR ledcontrol: GPIO error %s (rc=%d)\n",mygpio->name,rc);
	}

	return TRUE;

}

static gboolean
on_set_off       (Led          *led,
                GDBusMethodInvocation  *invocation,
                gpointer                user_data)
{
	GPIO* mygpio = (GPIO*)user_data;
	g_print("Turn off LED: %s\n",mygpio->name);
	led_complete_set_off(led,invocation);
	int rc = GPIO_OK;
	do {
		rc = gpio_open(mygpio);
		if (rc != GPIO_OK) { break; }
		rc = gpio_write(mygpio,1); 
		if (rc != GPIO_OK) { break; }
	} while(0);
	gpio_close(mygpio);
	if (rc != GPIO_OK)
	{
		printf("ERROR ChassisIdentify: GPIO error %s (rc=%d)\n",mygpio->name,rc);
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

	manager = g_dbus_object_manager_server_new (dbus_object_path);
	int i = 0;
	for (i=0;i<NUM_GPIO;i++)
  	{
		gchar *s;
		s = g_strdup_printf ("%s/%s",dbus_object_path,led_gpio[i].name);
		object = object_skeleton_new (s);
		g_free (s);

		Led *led = led_skeleton_new ();
		object_skeleton_set_led (object, led);
		g_object_unref (led);

		//define method callbacks
		g_signal_connect (led,
                    "handle-set-on",
                    G_CALLBACK (on_set_on),
                    &led_gpio[i]); /* user_data */
		g_signal_connect (led,
                    "handle-set-off",
                    G_CALLBACK (on_set_off),
                    &led_gpio[i]);

		led_set_color(led,0);
		led_set_function(led,led_gpio[i].name);
 
		gpio_init(connection,&led_gpio[i]);
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
