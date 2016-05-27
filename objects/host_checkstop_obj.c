#include "interfaces/openbmc_intf.h"
#include "openbmc.h"
#include "gpio.h"

static const gchar* dbus_object_path = "/org/openbmc/control";
static const gchar* instance_name = "checkstop0";
static const gchar* dbus_name = "org.openbmc.control.Checkstop";

static GDBusObjectManagerServer *manager = NULL;

GPIO checkstop = (GPIO){ "CHECKSTOP" };


static void
chassis_reboot(GDBusConnection* connection)
{
    GDBusProxy *proxy;
    GError *error;
    GVariant *parm = NULL;
    GVariant *result = NULL;

    error = NULL;
    proxy = g_dbus_proxy_new_sync(connection,
            G_DBUS_PROXY_FLAGS_NONE,
            NULL, /* GDBusInterfaceInfo* */
            "org.openbmc.control.Chassis", /* name */
            "/org/openbmc/control/chassis0", /* object path */
            "org.openbmc.Control", /* interface name */
            NULL, /* GCancellable */
            &error);
    g_assert_no_error(error);

    error = NULL;
    result = g_dbus_proxy_call_sync(proxy,
            "reboot",
            parm,
            G_DBUS_CALL_FLAGS_NONE,
            -1,
            NULL,
            &error);
    g_assert_no_error(error);
}

static bool
is_host_booted(GDBusConnection* connection)
{
    GDBusProxy *proxy;
    GError *error;
    GVariant *parm = NULL;
    GVariant *result = NULL;

    error = NULL;
    proxy = g_dbus_proxy_new_sync(connection,
            G_DBUS_PROXY_FLAGS_NONE,
            NULL, /* GDBusInterfaceInfo* */
            "org.openbmc.managers.System", /* name */
            "/org/openbmc/managers/System", /* object path */
            "org.openbmc.managers.System", /* interface name */
            NULL, /* GCancellable */
            &error);
    g_assert_no_error(error);

    error = NULL;
    result = g_dbus_proxy_call_sync(proxy,
            "getSystemState",
            parm,
            G_DBUS_CALL_FLAGS_NONE,
            -1,
            NULL,
            &error);
    g_assert_no_error(error);

    gchar *system_state;
    g_variant_get(result,"(s)",&system_state);
    g_variant_unref(result);

    if (strcmp(system_state, "HOST_BOOTED") == 0) {
        return true;
    }

    return false;
}

static gboolean
on_checkstop_interrupt(GIOChannel *channel,
        GIOCondition condition,
        gpointer connection)
{
    GError *error = 0;
    gsize bytes_read = 0;
    gchar buf[2];
    buf[1] = '\0';

    g_io_channel_seek_position( channel, 0, G_SEEK_SET, 0 );
    g_io_channel_read_chars(channel,
            buf, 1,
            &bytes_read,
            &error );
    printf("%s\n",buf);

    if(checkstop.irq_inited) {
        // Check system is powered on. GPIO line may flip during power on/off
        if (is_host_booted(connection)) {
            // Need to wait at least 10s for the SBE to gather failure data.
            // Also the user may be monitoring the system and reset the system
            // themselves. So wait an arbitrary time of 30s before reboot.
            printf("Host Checkstop, rebooting host in 30s\n");
            sleep(30);
            chassis_reboot(connection);
        }
    }
    else {
        checkstop.irq_inited = true;
    }

    return TRUE;
}

static void
on_bus_acquired(GDBusConnection *connection,
        const gchar *name,
        gpointer user_data)
{
    int rc = GPIO_OK;
    ObjectSkeleton *object;
    manager = g_dbus_object_manager_server_new(dbus_object_path);

    gchar *s;
    s = g_strdup_printf("%s/%s",dbus_object_path,instance_name);
    object = object_skeleton_new(s);
    g_free(s);

    ControlCheckstop* control_checkstop = control_checkstop_skeleton_new();
    object_skeleton_set_control_checkstop(object, control_checkstop);
    g_object_unref(control_checkstop);

    g_dbus_object_manager_server_set_connection(manager, connection);
    g_dbus_object_manager_server_export(manager, G_DBUS_OBJECT_SKELETON(object));
    g_object_unref(object);

    rc = gpio_init(connection, &checkstop);
    if (rc == GPIO_OK) { 
        rc = gpio_open_interrupt(&checkstop, on_checkstop_interrupt, connection);
    }
    if (rc != GPIO_OK) {
        printf("ERROR Checkstop: GPIO setup (rc=%d)\n", rc);
    }
}

static void
on_name_acquired(GDBusConnection *connection,
        const gchar *name,
        gpointer user_data)
{
}

static void
on_name_lost(GDBusConnection *connection,
        const gchar *name,
        gpointer user_data)
{
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

