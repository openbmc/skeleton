#!/usr/bin/python -u

import sys
import os
import gobject
import dbus
import dbus.service
import dbus.mainloop.glib
import obmc.sensors
from obmc.dbuslib.bindings import DbusProperties, DbusObjectManager, get_dbus
import obmc_system_config as System

DBUS_NAME = 'org.openbmc.Sensors'
OBJ_PATH = '/org/openbmc/sensors'


class SensorManager(DbusProperties,DbusObjectManager):
	def __init__(self,bus,name):
		DbusProperties.__init__(self)
		DbusObjectManager.__init__(self)
		dbus.service.Object.__init__(self,bus,name)
		self.InterfacesAdded(name,self.properties)

	@dbus.service.method(DBUS_NAME,
		in_signature='ss', out_signature='')
	def register(self,object_name,obj_path):
		if (self.objects.has_key(obj_path) == False):
			print "Register: "+object_name+" : "+obj_path
			sensor = eval('obmc.sensors.'+object_name+'(bus,obj_path)')
			self.add(obj_path,sensor)

	@dbus.service.method(DBUS_NAME,
		in_signature='s', out_signature='')
	def delete(self,obj_path):
		if (self.objects.has_key(obj_path) == True):
			print "Delete: "+obj_path
			self.remove(obj_path)
	
	def SensorChange(self,value,path=None):
		if (self.objects.has_key(path)):
			self.objects[path].setValue(value)
		else:
			print "ERROR: Sensor not found: "+path
			
if __name__ == '__main__':
	dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
	bus = get_dbus()
	name = dbus.service.BusName(DBUS_NAME,bus)
	root_sensor = SensorManager(bus,OBJ_PATH)


	## instantiate non-polling sensors
	## these don't need to be in seperate process
	for (id, the_sensor) in System.MISC_SENSORS.items():
		sensor_class = the_sensor['class']
		obj_path = System.ID_LOOKUP['SENSOR'][id]
		sensor_obj = getattr(obmc.sensors, sensor_class)(bus, obj_path)
		if 'os_path' in the_sensor:
			sensor_obj.sysfs_attr = the_sensor['os_path']
		root_sensor.add(obj_path, sensor_obj)

	mainloop = gobject.MainLoop()
	print "Starting sensor manager"
	mainloop.run()

