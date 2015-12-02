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
static const gchar* instance_name = "bmc0";
static const gchar* dbus_name        = "org.openbmc.control.Bmc";
static const gchar* i2c_dev = "/sys/bus/i2c/devices";

//this probably should come from some global SOC config

#define LPC_BASE		(off_t)0x1e789000
#define LPC_HICR6		0x80
#define LPC_HICR7		0x88
#define LPC_HICR8		0x8c
#define SPI_BASE		(off_t)0x1e630000
#define SCU_BASE                (off_t)0x1e780000
#define UART_BASE               (off_t)0x1e783000
#define COM_BASE                (off_t)0x1e789000
#define COM_BASE2               (off_t)0x1e789100
#define GPIO_BASE		(off_t)0x1e6e2000

static GDBusObjectManagerServer *manager = NULL;

void* memmap(int mem_fd,off_t base)
{
	void* bmcreg;
	bmcreg = mmap(NULL, getpagesize(),
			PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, base);

	if (bmcreg == MAP_FAILED) {
		printf("ERROR: Unable to map LPC register memory");
		exit(1);
	}
	return bmcreg;
}

void reg_init()
{
	g_print("BMC init\n");
	// BMC init done here

	void *bmcreg;
	int mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
	if (mem_fd < 0) {
		printf("ERROR: Unable to open /dev/mem");
		exit(1);
	}

	bmcreg = memmap(mem_fd,LPC_BASE);
	devmem(bmcreg+LPC_HICR6,0x00000500); //Enable LPC FWH cycles, Enable LPC to AHB bridge
	devmem(bmcreg+LPC_HICR7,0x30000C00); //32M PNOR
	devmem(bmcreg+LPC_HICR8,0xFC0003FF);

	//flash controller
	bmcreg = memmap(mem_fd,SPI_BASE);
	devmem(bmcreg+0x00,0x00000003);
	devmem(bmcreg+0x04,0x00002404);

	//UART

	
	bmcreg = memmap(mem_fd,UART_BASE);
	devmem(bmcreg+0x00,0x00000000);  //Set Baud rate divisor -> 13 (Baud 115200)
	devmem(bmcreg+0x04,0x00000000);  //Set Baud rate divisor -> 13 (Baud 115200)
	devmem(bmcreg+0x08,0x000000c1);  //Disable Parity, 1 stop bit, 8 bits
	bmcreg = memmap(mem_fd,COM_BASE);
	devmem(bmcreg+0x9C,0x00000000);  //Set UART routing

	bmcreg = memmap(mem_fd,SCU_BASE);
	devmem(bmcreg+0x00,0x9e82fce7);
	//devmem(bmcreg+0x00,0x9f82fce7); // B2?
	devmem(bmcreg+0x04,0x0370e677);
	
	// do not modify state of power pin, otherwise 
	// if this is a reboot, host will shutdown
	uint32_t reg_20 = devmem_read(bmcreg+0x20);
	reg_20 = reg_20 & 0x00000002;	
	devmem(bmcreg+0x20,0xcfc8f7fd | reg_20);
	devmem(bmcreg+0x24,0xc738f20a);
	devmem(bmcreg+0x80,0x0031ffaf);


	//GPIO
	bmcreg = memmap(mem_fd,GPIO_BASE);
	devmem(bmcreg+0x84,0x00fff0c0);  //Enable UART1
	devmem(bmcreg+0x80,0xCB000000);
	devmem(bmcreg+0x88,0x01C000FF);
	devmem(bmcreg+0x8c,0xC1C000FF);
	devmem(bmcreg+0x90,0x003FA009);
	devmem(bmcreg+0x88,0x01C0007F);


	bmcreg = memmap(mem_fd,COM_BASE);
	devmem(bmcreg+0x170,0x00000042);
	devmem(bmcreg+0x174,0x00004000);

	close(mem_fd);
}

int init_i2c_driver(int i2c_bus, const char* device, int address,bool delete)
{
	char dev[255];
	g_print("Initing: device = %s, address = %02x\n",device,address);
	if (!delete) {
		sprintf(dev,"%s/i2c-%d/new_device",i2c_dev,i2c_bus);
	} else {
		sprintf(dev,"%s/i2c-%d/delete_device",i2c_dev,i2c_bus);
	}
	int fd = open(dev, O_WRONLY);
	if (fd == -1) {
		g_print("ERROR control_bmc: Unable to open device %s\n",dev);
		return 1;
	}
	if (!delete) {
		sprintf(dev,"%s 0x%02x",device,address);
	} else {
		sprintf(dev,"0x%02x",address);
	}
	int rc = write(fd,dev,strlen(dev));
	close(fd);
	if (rc != strlen(dev)) {
		g_print("ERROR control_bmc: Unable to write %s\n",dev);
		return 2;
	}
	return 0;
}


static gboolean
on_init (Control          *control,
         GDBusMethodInvocation  *invocation,
         gpointer                user_data)
{
	control_complete_init(control,invocation);
	
	return TRUE;
}
gboolean go(gpointer user_data)
{
 	cmdline *cmd = user_data;
	Control* control = object_get_control((Object*)cmd->user_data);
	#ifdef __arm__
	reg_init();
	#endif
	control_emit_goto_system_state(control,"BMC_STARTING");
	
	return FALSE;
}

static void 
on_bus_acquired (GDBusConnection *connection,
                 const gchar     *name,
                 gpointer         user_data)
{
	ObjectSkeleton *object;
 	cmdline *cmd = user_data;
  	manager = g_dbus_object_manager_server_new (dbus_object_path);

	gchar *s;
	s = g_strdup_printf ("%s/%s",dbus_object_path,instance_name);
	object = object_skeleton_new (s);
	g_free (s);

	ControlBmc* control_bmc = control_bmc_skeleton_new ();
	object_skeleton_set_control_bmc (object, control_bmc);
	g_object_unref (control_bmc);

	Control* control = control_skeleton_new ();
	object_skeleton_set_control (object, control);
	g_object_unref (control);

	//define method callbacks here
	g_signal_connect (control,
       	            "handle-init",
               	    G_CALLBACK (on_init),
               	    NULL); /* user_data */

	/* Export the object (@manager takes its own reference to @object) */
	g_dbus_object_manager_server_export (manager, G_DBUS_OBJECT_SKELETON (object));
	g_object_unref (object);

	/* Export all objects */
	g_dbus_object_manager_server_set_connection (manager, connection);

	//TODO:  This is a bad hack to wait for object to be on bus
	//sleep(1);
	cmd->user_data = object;
	g_idle_add(go,cmd);
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
  cmd.loop = loop;

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
