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

/* ------------------------------------------------------------------------- */
static const gchar* dbus_object_path = "/org/openbmc/control";
static const gchar* instance_name = "host0";
static const gchar* dbus_name = "org.openbmc.control.Host";

static GDBusObjectManagerServer *manager = NULL;

GPIO fsi_data     = (GPIO){ "FSI_DATA" };
GPIO fsi_clk      = (GPIO){ "FSI_CLK" };
GPIO fsi_enable   = (GPIO){ "FSI_ENABLE" };
GPIO cronus_sel   = (GPIO){ "CRONUS_SEL" };
GPIO Throttle     = (GPIO){ "BMC_THROTTLE" };
GPIO idbtn     	  = (GPIO){ "IDBTN" };

/* Bit bang patterns */

//putcfam pu 281c 30000000 -p0 (Primary Side Select)
static const char* primary = "000011111111110101111000111001100111111111111111111111111111101111111111";
//putcfam pu 281c B0000000 -p0
static const char* go = "000011111111110101111000111000100111111111111111111111111111101101111111";
//putcfam pu 0x281c 30900000 (Golden Side Select)
static const char* golden = "000011111111110101111000111001100111101101111111111111111111101001111111";

/* Setup attentions */
//putcfam pu 0x081C 20000000
static const char* attnA = "000011111111111101111110001001101111111111111111111111111111110001111111";
//putcfam pu 0x100D 40000000
static const char* attnB = "000011111111111011111100101001011111111111111111111111111111110001111111";
//putcfam pu  0x100B FFFFFFFF
static const char* attnC = "000011111111111011111101001000000000000000000000000000000000001011111111";



static gboolean
on_init(Control *control,
		GDBusMethodInvocation *invocation,
		gpointer user_data)
{
	control_complete_init(control,invocation);
	return TRUE;
}

int
fsi_bitbang(const char* pattern)
{
	int rc=GPIO_OK;
	int i;
	for(i=0;i<strlen(pattern);i++) {
		rc = gpio_writec(&fsi_data,pattern[i]);
		if(rc!=GPIO_OK) { break; }
		rc = gpio_clock_cycle(&fsi_clk,1);
		if(rc!=GPIO_OK) { break; }
	}
	return rc;
}

int
fsi_standby()
{
	int rc=GPIO_OK;
	rc = gpio_write(&fsi_data,1);
	if(rc!=GPIO_OK) { return rc; }
	rc = gpio_clock_cycle(&fsi_clk,5000);
	if(rc!=GPIO_OK) { return rc; }
	return rc;
}


static gboolean
on_boot(ControlHost *host,
		GDBusMethodInvocation *invocation,
		gpointer user_data)
{
	int rc = GPIO_OK;

	if(control_host_get_debug_mode(host)==1)
	{
		g_print("Enabling debug mode; not booting host\n");
		rc |= gpio_open(&fsi_enable);
		rc |= gpio_open(&cronus_sel);
		rc |= gpio_write(&fsi_enable,1);
		rc |= gpio_write(&cronus_sel,0);
		if(rc!=GPIO_OK) {
			g_print("ERROR enabling debug mode: %d\n",rc);
		}
		return TRUE;
	}
	g_print("Booting host\n");
	Control* control = object_get_control((Object*)user_data);
	control_host_complete_boot(host,invocation);
	do {
		rc = gpio_open(&fsi_clk);
		rc |= gpio_open(&fsi_data);
		rc |= gpio_open(&fsi_enable);
		rc |= gpio_open(&cronus_sel);
		rc |= gpio_open(&Throttle);
		rc |= gpio_open(&idbtn);
		if(rc!=GPIO_OK) { break; }

		//setup dc pins
		rc = gpio_write(&cronus_sel,1);
		rc |= gpio_write(&fsi_enable,1);
		rc |= gpio_write(&fsi_clk,1);
		rc |= gpio_write(&Throttle,1);
		rc |= gpio_write(&idbtn,0);
		if(rc!=GPIO_OK) { break; }

		//data standy state
		rc = fsi_standby();

		//clear out pipes
		rc |= gpio_write(&fsi_data,0);
		rc |= gpio_clock_cycle(&fsi_clk,256);
		rc |= gpio_write(&fsi_data,1);
		rc |= gpio_clock_cycle(&fsi_clk,50);
		if(rc!=GPIO_OK) { break; }

		rc = fsi_bitbang(attnA);
		rc |= fsi_standby();

		rc |= fsi_bitbang(attnB);
		rc |= fsi_standby();

		rc |= fsi_bitbang(attnC);
		rc |= fsi_standby();
		if(rc!=GPIO_OK) { break; }

		const gchar* flash_side = control_host_get_flash_side(host);
		g_print("Using %s side of the bios flash\n",flash_side);
		if(strcmp(flash_side,"primary")==0) {
			rc |= fsi_bitbang(primary);
		} else if(strcmp(flash_side,"golden") == 0) {
			rc |= fsi_bitbang(golden);
		} else {
			g_print("ERROR: Invalid flash side: %s\n",flash_side);
			rc = 0xff;

		}
		rc |= fsi_standby();
		if(rc!=GPIO_OK) { break; }

		rc = fsi_bitbang(go);

		rc |= gpio_write(&fsi_data,1); /* Data standby state */
		rc |= gpio_clock_cycle(&fsi_clk,2);

		rc |= gpio_write(&fsi_clk,0); /* hold clk low for clock mux */
		rc |= gpio_write(&fsi_enable,0);
		rc |= gpio_clock_cycle(&fsi_clk,16);
		rc |= gpio_write(&fsi_clk,0); /* Data standby state */

	} while(0);
	if(rc != GPIO_OK)
	{
		g_print("ERROR HostControl: GPIO sequence failed (rc=%d)\n",rc);
	} else {
		control_emit_goto_system_state(control,"HOST_BOOTING");
	}
	gpio_close(&fsi_clk);
	gpio_close(&fsi_data);
	gpio_close(&fsi_enable);
	gpio_close(&cronus_sel);
	gpio_close(&Throttle);
	gpio_close(&idbtn);

	control_host_emit_booted(host);
	return TRUE;
}

static void
on_bus_acquired(GDBusConnection *connection,
		const gchar *name,
		gpointer user_data)
{
	ObjectSkeleton *object;
	//g_print ("Acquired a message bus connection: %s\n",name);
	manager = g_dbus_object_manager_server_new(dbus_object_path);

	gchar *s;
	s = g_strdup_printf("%s/%s",dbus_object_path,instance_name);
	object = object_skeleton_new(s);
	g_free(s);

	ControlHost* control_host = control_host_skeleton_new();
	object_skeleton_set_control_host(object, control_host);
	g_object_unref(control_host);

	Control* control = control_skeleton_new();
	object_skeleton_set_control(object, control);
	g_object_unref(control);

	//define method callbacks here
	g_signal_connect(control_host,
			"handle-boot",
			G_CALLBACK(on_boot),
			object); /* user_data */
	g_signal_connect(control,
			"handle-init",
			G_CALLBACK(on_init),
			NULL); /* user_data */

	control_host_set_debug_mode(control_host,0);
	control_host_set_flash_side(control_host,"primary");

	/* Export the object (@manager takes its own reference to @object) */
	g_dbus_object_manager_server_set_connection(manager, connection);
	g_dbus_object_manager_server_export(manager, G_DBUS_OBJECT_SKELETON(object));
	g_object_unref(object);

	gpio_init(connection,&fsi_data);
	gpio_init(connection,&fsi_clk);
	gpio_init(connection,&fsi_enable);
	gpio_init(connection,&cronus_sel);
	gpio_init(connection,&Throttle);
	gpio_init(connection,&idbtn);
}

static void
on_name_acquired(GDBusConnection *connection,
		const gchar *name,
		gpointer user_data)
{
	//  g_print ("Acquired the name %s\n", name);
}

static void
on_name_lost(GDBusConnection *connection,
		const gchar *name,
		gpointer user_data)
{
	//  g_print ("Lost the name %s\n", name);
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
