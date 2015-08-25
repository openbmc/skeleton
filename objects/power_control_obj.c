#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "interfaces/power_control.h"

/* ---------------------------------------------------------------------------------------------------- */
static const gchar* dbus_object_path = "/org/openbmc/control/Power";
static const gchar* dbus_name        = "org.openbmc.control.Power";

// Platform specific config

#define FSI_CLK		4	//GPIOA4
#define FSI_DAT		5	//GPIOA5
#define CRONUS_SEL	6	//GPIOA6
#define PCIE_RST_N	13	//GPIOB5
#define PEX_PERST_N	14	//GPIOB6
#define POWER		33    	//GPIOE1
#define PGOOD		23    	//GPIOC7
#define FSI_ENABLE      24      //GPIOD0

/* Where to put the firmware image if booting from memory */
#define MEM_IMG_BASE (0x54000000)

/* Start of flash memory if booting from flash */
#define FLASH_IMG_BASE (0x30000000)

/* LPC registers */
#define LPC_BASE		0x1e789000
#define LPC_HICR6		0x80
#define LPC_HICR7		0x88
#define LPC_HICR8		0x8c




static GDBusObjectManagerServer *manager = NULL;
static PowerControl *power_control = NULL;

static guint tmp_pgood = 0;

static gboolean
on_set_power_state (PowerControl          *pwr,
                GDBusMethodInvocation  *invocation,
                guint                   state,
                gpointer                user_data)
{
  if (state > 1)
  {
      g_dbus_method_invocation_return_dbus_error (invocation,
                                                  "org.openbmc.PowerControl.Error.Failed",
                                                  "Invalid power state");
      return TRUE;
  }
  if (state == power_control_get_state(pwr))
  {
      g_dbus_method_invocation_return_dbus_error (invocation,
                                                  "org.openbmc.PowerControl.Error.Failed",
                                                  "Power Control is already at requested state");
      return TRUE;     
  }

  // TODO: Implement gpio toggling
  g_print("Set power state: %d\n",state);
  if (state==1)
  {
    g_print("Turn on\n");
    tmp_pgood=1;
  }
  else
  {
    g_print("Turn off\n");
    tmp_pgood=0;
  }
  power_control_set_state(pwr,state);
  power_control_complete_set_power_state(pwr,invocation);
  return TRUE;
}

static gboolean
on_get_power_state (PowerControl          *pwr,
                GDBusMethodInvocation  *invocation,
                gpointer                user_data)
{
  guint pgood = power_control_get_pgood(pwr);
  power_control_complete_get_power_state(pwr,invocation,pgood);
  return TRUE;
}

static void 
on_bus_acquired (GDBusConnection *connection,
                 const gchar     *name,
                 gpointer         user_data)
{
  ObjectSkeleton *object;
  guint n;

  g_print ("Acquired a message bus connection: %s\n",name);

  manager = g_dbus_object_manager_server_new (dbus_object_path);

  gchar *s;
  s = g_strdup_printf ("%s/0",dbus_object_path);
  object = object_skeleton_new (s);
  g_free (s);

  power_control = power_control_skeleton_new ();
  object_skeleton_set_power_control (object, power_control);
  g_object_unref (power_control);

  //define method callbacks here
  g_signal_connect (power_control,
                    "handle-set-power-state",
                    G_CALLBACK (on_set_power_state),
                    NULL); /* user_data */

  g_signal_connect (power_control,
                    "handle-get-power-state",
                    G_CALLBACK (on_get_power_state),
                    NULL); /* user_data */

  /* Export the object (@manager takes its own reference to @object) */
  g_dbus_object_manager_server_export (manager, G_DBUS_OBJECT_SKELETON (object));
  g_object_unref (object);

  /* Export all objects */
  g_dbus_object_manager_server_set_connection (manager, connection);
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

static gboolean
poll_pgood()
{
  guint pgood = power_control_get_pgood(power_control);
  //TOOD:  Change to actually read gpio
  guint gpio = tmp_pgood;

  g_print("Polling pgood:  %d\n",gpio);

  //if changed, set property and emit signal
  if (gpio != power_control_get_pgood(power_control))
  {
     power_control_set_pgood(power_control,gpio);
     if (gpio==0)
     {
       power_control_emit_power_lost(power_control);
     }
     else
     {
       power_control_emit_power_good(power_control);
     }
  }
  return TRUE;
}

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
