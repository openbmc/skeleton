#!/usr/bin/python -u

import sys
import os
import gobject
import dbus
import dbus.service
import dbus.mainloop.glib
import Openbmc
import Sensors

DBUS_NAME = 'org.openbmc.Sensors'
OBJ_PATH = '/org/openbmc/sensors'


class SensorManager(Openbmc.DbusProperties,Openbmc.DbusObjectManager):
	def __init__(self,bus,name):
		Openbmc.DbusProperties.__init__(self)
		Openbmc.DbusObjectManager.__init__(self)
		dbus.service.Object.__init__(self,bus,name)
		self.InterfacesAdded(name,self.properties)

	@dbus.service.method(DBUS_NAME,
		in_signature='ss', out_signature='')
	def register(self,object_name,obj_path):
		if (self.objects.has_key(obj_path) == False):
			print "Register: "+object_name+" : "+obj_path
			sensor = eval('Sensors.'+object_name+'(bus,obj_path)')
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
	bus = Openbmc.getDBus()
	name = dbus.service.BusName(DBUS_NAME,bus)
	root_sensor = SensorManager(bus,OBJ_PATH)


	## instantiate non-polling sensors
	## these don't need to be in seperate process
	## TODO: this should not be hardcoded

	obj_path = OBJ_PATH+"/host/PowerCap"
	root_sensor.add(obj_path,Sensors.PowerCap(bus,obj_path))
	obj_path = OBJ_PATH+"/host/BootProgress"
	root_sensor.add(obj_path,Sensors.BootProgressSensor(bus,obj_path))
	obj_path = OBJ_PATH+"/host/OccStatus"
	root_sensor.add(obj_path,Sensors.OccStatusSensor(bus,obj_path))
	obj_path = OBJ_PATH+"/host/BootCount"
	root_sensor.add(obj_path,Sensors.BootCountSensor(bus,obj_path))
	obj_path = OBJ_PATH+"/host/OperatingSystemStatus"
	root_sensor.add(obj_path,Sensors.OperatingSystemStatusSensor(bus,obj_path))

	mainloop = gobject.MainLoop()
	print "Starting sensor manager"
	mainloop.run()

