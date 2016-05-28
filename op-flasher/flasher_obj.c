#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <byteswap.h>
#include <stdint.h>
#include <stdbool.h>
#include <getopt.h>
#include <limits.h>
#include <arpa/inet.h>
#include <assert.h>

#include <libflash/libflash.h>
#include <libflash/libffs.h>
#include "progress.h"
#include "io.h"
#include "ast.h"
#include "sfc-ctrl.h"
#include "interfaces/openbmc_intf.h"
#include "openbmc.h"

static const gchar* dbus_object_path = "/org/openbmc/control";
static const gchar* dbus_name = "org.openbmc.control.Flasher";

static GDBusObjectManagerServer *manager = NULL;

#define __aligned(x)			__attribute__((aligned(x)))

#define PFLASH_VERSION	"0.8.6"

static bool need_relock;
#ifdef __powerpc__
static bool using_sfc;
#endif

#define FILE_BUF_SIZE	0x10000
static uint8_t file_buf[FILE_BUF_SIZE] __aligned(0x1000);

static struct spi_flash_ctrl	*fl_ctrl;
static struct flash_chip	*fl_chip;
static struct ffs_handle	*ffsh;
static uint32_t			fl_total_size, fl_erase_granule;
static const char		*fl_name;
static int32_t			ffs_index = -1;

static uint8_t FLASH_OK = 0;
static uint8_t FLASH_ERROR = 0x01;
static uint8_t FLASH_SETUP_ERROR = 0x02;

static int
erase_chip(void)
{
	int rc = 0;

	printf("Erasing... (may take a while !) ");
	fflush(stdout);

	rc = flash_erase_chip(fl_chip);
	if(rc) {
		fprintf(stderr, "Error %d erasing chip\n", rc);
		return(rc);
	}

	printf("done !\n");
	return(rc);
}

void
flash_message(GDBusConnection* connection,char* obj_path,char* method, char* error_msg)
{
	GDBusProxy *proxy;
	GError *error;
	GVariant *parm = NULL;
	error = NULL;
	proxy = g_dbus_proxy_new_sync(connection,
			G_DBUS_PROXY_FLAGS_NONE,
			NULL, /* GDBusInterfaceInfo* */
			"org.openbmc.control.Flash", /* name */
			obj_path, /* object path */
			"org.openbmc.Flash", /* interface name */
			NULL, /* GCancellable */
			&error);
	g_assert_no_error(error);

	error = NULL;
	if(strcmp(method,"error")==0) {
		parm = g_variant_new("(s)",error_msg);
	}
	g_dbus_proxy_call_sync(proxy,
			method,
			parm,
			G_DBUS_CALL_FLAGS_NONE,
			-1,
			NULL,
			&error);

	g_assert_no_error(error);
}

static int
program_file(FlashControl* flash_control, const char *file, uint32_t start, uint32_t size)
{
	int fd, rc;
	ssize_t len;
	uint32_t actual_size = 0;

	fd = open(file, O_RDONLY);
	if(fd == -1) {
		perror("Failed to open file");
		return(fd);
	}
	printf("About to program \"%s\" at 0x%08x..0x%08x !\n",
			file, start, size);

	printf("Programming & Verifying...\n");
	//progress_init(size >> 8);
	unsigned int save_size = size;
	uint8_t last_progress = 0;
	while(size) {
		len = read(fd, file_buf, FILE_BUF_SIZE);
		if(len < 0) {
			perror("Error reading file");
			return(1);
		}
		if(len == 0)
			break;
		if(len > size)
			len = size;
		size -= len;
		actual_size += len;
		rc = flash_write(fl_chip, start, file_buf, len, true);
		if(rc) {
			if(rc == FLASH_ERR_VERIFY_FAILURE)
				fprintf(stderr, "Verification failed for"
						" chunk at 0x%08x\n", start);
			else
				fprintf(stderr, "Flash write error %d for"
						" chunk at 0x%08x\n", rc, start);
			return(rc);
		}
		start += len;
		unsigned int percent = (100*actual_size/save_size);
		uint8_t progress = (uint8_t)(percent);
		if(progress != last_progress) {
			flash_control_emit_progress(flash_control,file,progress);
			last_progress = progress;
		}
	}
	close(fd);

	/* If this is a flash partition, adjust its size */
	if(ffsh && ffs_index >= 0) {
		printf("Updating actual size in partition header...\n");
		ffs_update_act_size(ffsh, ffs_index, actual_size);
	}
	return(0);
}

static void
flash_access_cleanup_bmc(void)
{
	if(ffsh)
		ffs_close(ffsh);
	flash_exit(fl_chip);
	ast_sf_close(fl_ctrl);
	close_devs();
}

static int
flash_access_setup_bmc(bool use_lpc, bool need_write)
{
	int rc;
	printf("Setting up BMC flash\n");
	/* Open and map devices */
	open_devs(use_lpc, true);

	/* Create the AST flash controller */
	rc = ast_sf_open(AST_SF_TYPE_BMC, &fl_ctrl);
	if(rc) {
		fprintf(stderr, "Failed to open controller\n");
		return FLASH_SETUP_ERROR;
	}

	/* Open flash chip */
	rc = flash_init(fl_ctrl, &fl_chip);
	if(rc) {
		fprintf(stderr, "Failed to open flash chip\n");
		return FLASH_SETUP_ERROR;
	}

	/* Setup cleanup function */
	atexit(flash_access_cleanup_bmc);
	return FLASH_OK;
}

static void
flash_access_cleanup_pnor(void)
{
	/* Re-lock flash */
	if(need_relock)
		set_wrprotect(true);

	if(ffsh)
		ffs_close(ffsh);
	flash_exit(fl_chip);
#ifdef __powerpc__
	if(using_sfc)
		sfc_close(fl_ctrl);
	else
		ast_sf_close(fl_ctrl);
#else
	ast_sf_close(fl_ctrl);
#endif
	close_devs();
}

static int
flash_access_setup_pnor(bool use_lpc, bool use_sfc, bool need_write)
{
	int rc;
	printf("Setting up BIOS flash\n");

	/* Open and map devices */
	open_devs(use_lpc, false);

#ifdef __powerpc__
	if(use_sfc) {
		/* Create the SFC flash controller */
		rc = sfc_open(&fl_ctrl);
		if(rc) {
			fprintf(stderr, "Failed to open controller\n");
			return FLASH_SETUP_ERROR;
		}
		using_sfc = true;
	} else {
#endif
		/* Create the AST flash controller */
		rc = ast_sf_open(AST_SF_TYPE_PNOR, &fl_ctrl);
		if(rc) {
			fprintf(stderr, "Failed to open controller\n");
			return FLASH_SETUP_ERROR;
		}
#ifdef __powerpc__
	}
#endif

	/* Open flash chip */
	rc = flash_init(fl_ctrl, &fl_chip);
	if(rc) {
		fprintf(stderr, "Failed to open flash chip\n");
		return FLASH_SETUP_ERROR;
	}

	/* Unlock flash (PNOR only) */
	if(need_write)
		need_relock = set_wrprotect(false);

	/* Setup cleanup function */
	atexit(flash_access_cleanup_pnor);
	return FLASH_OK;
}

uint8_t
flash(FlashControl* flash_control,bool bmc_flash, uint32_t address, char* write_file, char* obj_path)
{
	bool has_sfc = false, has_ast = false, use_lpc = true;
	bool erase = true, program = true;

	int rc;
	printf("flasher: %s, BMC = %d, address = 0x%x\n",write_file,bmc_flash,address);
#ifdef __arm__
	/* Check platform */
	check_platform(&has_sfc, &has_ast);

	/* Prepare for access */
	if(bmc_flash) {
		if(!has_ast) {
			fprintf(stderr, "No BMC on this platform\n");
			return FLASH_SETUP_ERROR;
		}
		rc = flash_access_setup_bmc(use_lpc, erase || program);
		if(rc) {
			return FLASH_SETUP_ERROR;
		}
	} else {
		if(!has_ast && !has_sfc) {
			fprintf(stderr, "No BMC nor SFC on this platform\n");
			return FLASH_SETUP_ERROR;
		}
		rc = flash_access_setup_pnor(use_lpc, has_sfc, erase || program);
		if(rc) {
			return FLASH_SETUP_ERROR;
		}
	}

	rc = flash_get_info(fl_chip, &fl_name,
			&fl_total_size, &fl_erase_granule);
	if(rc) {
		fprintf(stderr, "Error %d getting flash info\n", rc);
		return FLASH_SETUP_ERROR;
	}
#endif
	if(strcmp(write_file,"")!=0)
	{
		// If file specified but not size, get size from file
		struct stat stbuf;
		if(stat(write_file, &stbuf)) {
			perror("Failed to get file size");
			return FLASH_ERROR;
		}
		uint32_t write_size = stbuf.st_size;
#ifdef __arm__
		rc = erase_chip();
		if(rc) {
			return FLASH_ERROR;
		}
		rc = program_file(flash_control, write_file, address, write_size);
		if(rc) {
			return FLASH_ERROR;
		}
#endif

		printf("Flash done\n");
	}
	else
	{
		printf("Flash tuned\n");
	}
	return FLASH_OK;
}

static void
on_bus_acquired(GDBusConnection *connection,
		const gchar *name,
		gpointer user_data)
{
	cmdline *cmd = user_data;
	if(cmd->argc < 4)
	{
		g_print("flasher [flash name] [filename] [source object]\n");
		g_main_loop_quit(cmd->loop);
		return;
	}
	printf("Starting flasher: %s,%s,%s,\n",cmd->argv[1],cmd->argv[2],cmd->argv[3]);
	ObjectSkeleton *object;
	manager = g_dbus_object_manager_server_new(dbus_object_path);
	gchar *s;
	s = g_strdup_printf("%s/%s",dbus_object_path,cmd->argv[1]);

	object = object_skeleton_new(s);
	g_free(s);

	FlashControl* flash_control = flash_control_skeleton_new();
	object_skeleton_set_flash_control(object, flash_control);
	g_object_unref(flash_control);

	/* Export the object (@manager takes its own reference to @object) */
	g_dbus_object_manager_server_export(manager, G_DBUS_OBJECT_SKELETON(object));
	g_object_unref(object);

	/* Export all objects */
	g_dbus_object_manager_server_set_connection(manager, connection);
	bool bmc_flash = false;
	uint32_t address = 0;
	if(strcmp(cmd->argv[1],"bmc")==0) {
		bmc_flash = true;
	}
	if(strcmp(cmd->argv[1],"bmc_ramdisk")==0) {
		bmc_flash = true;
		address = 0x20300000;
	}
	if(strcmp(cmd->argv[1],"bmc_kernel")==0) {
		bmc_flash = true;
		address = 0x20080000;
	}

	int rc = flash(flash_control,bmc_flash,address,cmd->argv[2],cmd->argv[3]);
	if(rc) {
		flash_message(connection,cmd->argv[3],"error","Flash Error");
	} else {
		flash_message(connection,cmd->argv[3],"done","");
	}

	//Object exits when done flashing
	g_main_loop_quit(cmd->loop);
}

int
main(int argc, char *argv[])
{
	GMainLoop *loop;
	cmdline cmd;
	cmd.argc = argc;
	cmd.argv = argv;

	guint id;
	loop = g_main_loop_new(NULL, FALSE);
	cmd.loop = loop;

	id = g_bus_own_name(DBUS_TYPE,
			dbus_name,
			G_BUS_NAME_OWNER_FLAGS_ALLOW_REPLACEMENT |
			G_BUS_NAME_OWNER_FLAGS_REPLACE,
			on_bus_acquired,
			NULL,
			NULL,
			&cmd,
			NULL);

	g_main_loop_run(loop);

	g_bus_unown_name(id);
	g_main_loop_unref(loop);

	return 0;
}
