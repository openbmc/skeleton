#include "object_mapper.h"


void
emit_object_added(GDBusObjectManager *manager)
{
    GList *objects;
    GList *l;

    objects = g_dbus_object_manager_get_objects(manager);
    for (l = objects; l != NULL; l = l->next)
    {
        GDBusObject *object = l->data;
	ObjectMapper* map = object_get_object_mapper((Object*)object);

        GList *interfaces;
        GList *ll;
	const gchar *object_path = g_dbus_object_get_object_path(G_DBUS_OBJECT(object));

        interfaces = g_dbus_object_get_interfaces(G_DBUS_OBJECT(object));
        for (ll = interfaces; ll != NULL; ll = ll->next)
        {
            GDBusInterface *interface = G_DBUS_INTERFACE(ll->data);
            object_mapper_emit_object_added(map,object_path,
		g_dbus_interface_get_info(interface)->name);
        }
        g_list_free_full(interfaces, g_object_unref);
    }
    g_list_free_full(objects, g_object_unref);
}

