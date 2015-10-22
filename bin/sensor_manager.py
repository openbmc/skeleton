#!/usr/bin/python -u

import sys
#from gi.repository import GObject
import gobject
import dbus
import dbus.service
import dbus.mainloop.glib

if (len(sys.argv) < 2):
	print "Usage:  sensor_manager.py [system name]"
	exit(1)
System = __import__(sys.argv[1])
import Openbmc

DBUS_NAME = 'org.openbmc.managers.Sensors'
OBJ_NAME = '/org/openbmc/sensors'
ENUM_INTF = 'org.openbmc.Object.Enumerate'

class SensorManager(dbus.service.Object):
	def __init__(self,bus,name):
		dbus.service.Object.__init__(self,bus,name)
		bus.add_signal_receiver(self.SensorChangedHandler,
					dbus_interface = 'org.openbmc.SensorValue', 
					signal_name = 'Changed', path_keyword='path')
		bus.add_signal_receiver(self.SensorErrorHandler,
					dbus_interface = 'org.openbmc.SensorValue', 
					signal_name = 'Error', path_keyword='path')
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

	@dbus.service.method(ENUM_INTF,
		in_signature='', out_signature='a{sa{sv}}')
	def enumerate(self):
		return self.sensor_cache;
	
	
	@dbus.service.method(DBUS_NAME,
		in_signature='s', out_signature='v')
	def getSensor(self,path):
		val = 0
		if (self.sensor_cache.has_key(path) == True):
			val = self.sensor_cache[path]['value']
		else:
			print "ERROR: Unknown sensor: "+path
		return val
	
	## Signal handlers
	def SensorErrorHandler(self,path = None):
		self.initSensorEntry(path)
		self.sensor_cache[path]['error'] = True

	def SensorChangedHandler(self,value,units,path = None):
		self.initSensorEntry(path)
		self.sensor_cache[path]['value'] = value
		self.sensor_cache[path]['units'] = units

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

				
if __name__ == '__main__':
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
    bus = Openbmc.getDBus()
    name = dbus.service.BusName(DBUS_NAME,bus)
    obj = SensorManager(bus,OBJ_NAME)
    mainloop = gobject.MainLoop()

    print "Running Sensor Manager"
    mainloop.run()

