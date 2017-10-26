#!/usr/bin/env python

import gobject
import dbus
import dbus.service
import dbus.mainloop.glib
import obmc.sensors
from obmc.dbuslib.bindings import DbusProperties, DbusObjectManager, get_dbus

try:
    import obmc_system_config as System
    has_system = True
except ImportError:
    has_system = False

DBUS_NAME = 'org.openbmc.Sensors'
OBJ_PATH = '/org/openbmc/sensors'


class SensorManager(DbusProperties, DbusObjectManager):
    def __init__(self, bus, name):
        super(SensorManager, self).__init__(
            conn=bus,
            object_path=name)

    @dbus.service.method(
        DBUS_NAME, in_signature='ss', out_signature='')
    def register(self, object_name, obj_path):
        if obj_path not in self.objects:
            print "Register: "+object_name+" : "+obj_path
            sensor = eval('obmc.sensors.'+object_name+'(bus,obj_path)')
            self.add(obj_path, sensor)

    @dbus.service.method(
        DBUS_NAME, in_signature='s', out_signature='')
    def delete(self, obj_path):
        if obj_path in self.objects:
            print "Delete: "+obj_path
            self.remove(obj_path)

    def SensorChange(self, value, path=None):
        if path in self.objects:
            self.objects[path].setValue(value)
        else:
            print "ERROR: Sensor not found: "+path

if __name__ == '__main__':
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
    bus = get_dbus()
    root_sensor = SensorManager(bus, OBJ_PATH)

    ## instantiate non-polling sensors
    ## these don't need to be in separate process
    if has_system:
        for (id, the_sensor) in System.MISC_SENSORS.items():
            sensor_class = the_sensor['class']
            obj_path = System.ID_LOOKUP['SENSOR'][id]
            sensor_obj = getattr(obmc.sensors, sensor_class)(bus, obj_path)
            if 'os_path' in the_sensor:
                sensor_obj.sysfs_attr = the_sensor['os_path']
            root_sensor.add(obj_path, sensor_obj)

    mainloop = gobject.MainLoop()

    root_sensor.unmask_signals()
    name = dbus.service.BusName(DBUS_NAME, bus)
    print "Starting sensor manager"
    mainloop.run()

# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
