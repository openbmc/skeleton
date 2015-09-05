#!/usr/bin/env python

import sys
from gi.repository import GObject
import dbus
import dbus.service
import dbus.mainloop.glib

if (len(sys.argv) < 2):
	print "Usage:  sensor_manager.py [system name]"
	exit(1)
System = __import__(sys.argv[1])
import Openbmc

DBUS_NAME = 'org.openbmc.managers.Sensors'
OBJ_NAME = '/org/openbmc/managers/Sensors/'+sys.argv[1]

class SensorManager(dbus.service.Object):
	def __init__(self,bus,name):
		dbus.service.Object.__init__(self,bus,name)
		bus.add_signal_receiver(self.UpdateSensor,
					dbus_interface = 'org.freedesktop.DBus.Properties', 
					signal_name = 'PropertiesChanged', path_keyword='path')
		bus.add_signal_receiver(self.NormalThreshold,
					dbus_interface = 'org.openbmc.SensorThreshold', 
					signal_name = 'Normal', path_keyword='path')
		bus.add_signal_receiver(self.WarningThreshold,
					dbus_interface = 'org.openbmc.SensorThreshold', 
					signal_name = 'Warning', path_keyword='path')
		bus.add_signal_receiver(self.CriticalThreshold,
					dbus_interface = 'org.openbmc.SensorThreshold', 
					signal_name = 'Critical', path_keyword='path')

		self.sensor_cache = {}
		
	@dbus.service.method(DBUS_NAME,
		in_signature='s', out_signature='v')
	def getSensor(self,path):
		val = None
		if (self.sensor_cache.has_key(path) == True):
			val = self.sensor_cache[path]['value']
		return val
		
	def UpdateSensor(self,interface,prop_dict,props, path = None):
		if (interface == "org.openbmc.SensorValue"):
			self.initSensorEntry(path)
			for p in prop_dict.keys():	
				self.sensor_cache[path][p] = prop_dict[p]

	@dbus.service.signal(DBUS_NAME)
	def CriticalThreshold(self, path = None):
		print "Critical: "+path
		self.initSensorEntry(path)
		self.sensor_cache[path]['threshold'] = "CRITICAL"

	@dbus.service.signal(DBUS_NAME)
	def WarningThreshold(self, path = None):
		print "Warning:"+path
		self.initSensorEntry(path)
		self.sensor_cache[path]['threshold'] = "WARNING"

	@dbus.service.signal(DBUS_NAME)
	def NormalThreshold(self, path = None):
		print "Normal: "+path
		self.initSensorEntry(path)
		self.sensor_cache[path]['threshold'] = "NORMAL"

	def initSensorEntry(self,path):
		if (self.sensor_cache.has_key(path) == False):
			self.sensor_cache[path] = {}
			obj = bus.get_object(Openbmc.object_to_bus_name(path),path)
			intf = dbus.Interface(obj,'org.freedesktop.DBus.Properties')
			self.sensor_cache[path]['units'] = intf.Get('org.openbmc.SensorValue','units')
			

				
if __name__ == '__main__':
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
    bus = dbus.SessionBus()
    name = dbus.service.BusName(DBUS_NAME,bus)
    obj = SensorManager(bus,OBJ_NAME)
    mainloop = GObject.MainLoop()

    print "Running Sensor Manager"
    mainloop.run()

