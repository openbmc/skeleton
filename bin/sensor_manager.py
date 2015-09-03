#!/usr/bin/env python

import sys
import gobject
import dbus
import dbus.service
import dbus.mainloop.glib

if (len(sys.argv) < 2):
	print "Usage:  sensor_manager.py [system name]"
	exit(1)

import Openbmc

DBUS_NAME = 'org.openbmc.managers.Sensors'
OBJ_NAME = '/org/openbmc/managers/Sensors/'+sys.argv[1]

class SensorManager(dbus.service.Object):
	def __init__(self,bus,name):
		dbus.service.Object.__init__(self,bus,name)
		bus.add_signal_receiver(self.UpdateSensor,
					dbus_interface = 'org.openbmc.SensorInteger', 
					signal_name = 'Changed', path_keyword='path')
		bus.add_signal_receiver(self.NormalThreshold,
					dbus_interface = 'org.openbmc.SensorIntegerThreshold', 
					signal_name = 'Normal', path_keyword='path')
		bus.add_signal_receiver(self.WarningThreshold,
					dbus_interface = 'org.openbmc.SensorIntegerThreshold', 
					signal_name = 'Warning', path_keyword='path')
		bus.add_signal_receiver(self.CriticalThreshold,
					dbus_interface = 'org.openbmc.SensorIntegerThreshold', 
					signal_name = 'Critical', path_keyword='path')

		self.sensor_cache = {}
		
	@dbus.service.method(DBUS_NAME,
		in_signature='s', out_signature='i')
	def getSensor(self,path):
		val = None
		if (self.sensor_cache.has_key(path) == True):
			val = self.sensor_cache[path]['value']
		return val
		
	def UpdateSensor(self,value, units, path = None):
		if (self.sensor_cache.has_key(path) == False):
			self.sensor_cache[path] = {}
		self.sensor_cache[path]['value'] = value
		self.sensor_cache[path]['units'] = units

	@dbus.service.signal(DBUS_NAME)
	def CriticalThreshold(self, path = None):
		print "Critical: "+path
		if (self.sensor_cache.has_key(path) == False):
			self.sensor_cache[path] = {}
		self.sensor_cache[path]['threshold'] = "CRITICAL"


	@dbus.service.signal(DBUS_NAME)
	def WarningThreshold(self, path = None):
		print "Warning: "+path
		if (self.sensor_cache.has_key(path) == False):
			self.sensor_cache[path] = {}
		self.sensor_cache[path]['threshold'] = "WARNING"


	@dbus.service.signal(DBUS_NAME)
	def NormalThreshold(self, path = None):
		print "Normal: "+path
		if (self.sensor_cache.has_key(path) == False):
			self.sensor_cache[path] = {}
		self.sensor_cache[path]['threshold'] = "NORMAL"




if __name__ == '__main__':
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
    bus = dbus.SessionBus()
    name = dbus.service.BusName(DBUS_NAME,bus)
    obj = SensorManager(bus,OBJ_NAME)
    mainloop = gobject.MainLoop()

    print "Running Sensor Manager"
    mainloop.run()

