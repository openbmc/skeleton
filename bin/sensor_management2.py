#!/usr/bin/env python


import gobject
import dbus
import dbus.service
import dbus.mainloop.glib
import xml.etree.ElementTree as ET
import System

DBUS_NAME = 'org.openbmc.managers.Sensors'
OBJ_NAME = '/org/openbmc/managers/Sensors'

NORMAL   = 0
LOWER_WARNING  = 1
LOWER_CRITICAL = 2
UPPER_WARNING  = 3
UPPER_CRITICAL = 4


sensor_config = System.BarreleyeSensors()

## finds objects held by Dbus ObjectManager
def get_interface(bus_name):
	obj_name = "/"+bus_name.replace('.','/')
	obj = bus.get_object(bus_name, obj_name)
	
	#Find object in object manager and retrieve interface name
	manager = dbus.Interface(obj,'org.freedesktop.DBus.ObjectManager')
	objects = manager.GetManagedObjects()
	obj_path = None
	interface = None

	for o in objects:
		for intf in objects[o].keys():
			if (intf.find('Sensor') > 0):
				interface = intf
				obj_path = o

	if (interface == None):
		raise Exception("Unable to find sensor: "+obj_path)	
	
	return [obj_path, interface]


class SensorManagement(dbus.service.Object):
	def __init__(self,bus,name):
		dbus.service.Object.__init__(self,bus,name)
		self.sensor_cache = {}
		for bus_name in bus.list_names():
			if (bus_name.find('org.openbmc.sensors')==0):
				self.request_name(bus_name,"",bus_name)

		bus.add_signal_receiver(self.request_name,
			dbus_interface = 'org.freedesktop.DBus', 
					signal_name = "NameOwnerChanged")


	def request_name(self, bus_name, a, b):
		if (len(b) > 0 and bus_name.find('org.openbmc.sensors') == 0):
			if (sensor_config.has_key(bus_name) == True):
				try:
					print "Loading: "+bus_name
					obj_info = get_interface(bus_name)
					obj = bus.get_object(bus_name,obj_info[0])
					intf = dbus.Interface(obj,obj_info[1])
					intf.setThresholds(sensor_config[bus_name]['lower_critical'],
						sensor_config[bus_name]['lower_warning'],
						sensor_config[bus_name]['upper_warning'],
						sensor_config[bus_name]['upper_critical'])
					if (sensor_config[bus_name].has_key('parameters')):
						intf.setConfigData(sensor_config[bus_name]['parameters'])
	
				except dbus.exceptions.DBusException, e:
					# TODO: not sure what to do if can't find other services
					print "Unable to find dependent services: ",e
			else:
				print "Sensor found on bus but no config: "+bus_name
		if (len(b) == 0  and bus_name.find('org.openbmc') ==0):
			print "Sensor stopped: "+bus_name

	@dbus.service.method(DBUS_NAME,
		in_signature='s', out_signature='a{ss}')
	def getAllSensors(self,obj_name):
		return None
		
	@dbus.service.method(DBUS_NAME,
		in_signature='s', out_signature='i')
	def getSensorValue(self,obj_name):
		sensor = self.sensor_cache[obj_name]
		return sensor.get_value()

	@dbus.service.signal(DBUS_NAME)
	def CriticalThreshold(self, obj):
		print "Critical: "+obj

	@dbus.service.signal(DBUS_NAME)
	def WarningThreshold(self, obj):
		print "Warning: "+obj


if __name__ == '__main__':
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
    bus = dbus.SessionBus()
    name = dbus.service.BusName(DBUS_NAME,bus)
    obj = SensorManagement(bus,OBJ_NAME)
    mainloop = gobject.MainLoop()
    
    print "Running SensorManagerService"
    mainloop.run()

