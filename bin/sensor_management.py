#!/usr/bin/env python


import gobject
import dbus
import dbus.service
import dbus.mainloop.glib
import xml.etree.ElementTree as ET
import System

NORMAL   = 0
LOWER_WARNING  = 1
LOWER_CRITICAL = 2
UPPER_WARNING  = 3
UPPER_CRITICAL = 4


sensor_config = System.Barreleye()

## finds objects held by Dbus ObjectManager
def get_interface(obj_path):
	bus_name = sensor_config[obj_path]['bus_name']
	obj_name = "/"+bus_name.replace('.','/')
	obj = bus.get_object(bus_name, obj_name)
	#Find object in object manager and retrieve interface name
	manager = dbus.Interface(obj,'org.freedesktop.DBus.ObjectManager')
	objects = manager.GetManagedObjects()
	interface = None
	if (objects.has_key(obj_path)):
		for intf in objects[obj_path].keys():
			if (intf.find('Sensor') > 0):
				interface = intf

	if (interface == None):
		raise Exception("Unable to find sensor: "+obj_path)	
	
	return interface


## Maintains last value, handles sensor changed events, applies thresholds
## The Sensor class is not exported onto dbus
class Sensor:

	def __init__(self, bus, obj_name, warning_callback, critical_callback):
		obj = bus.get_object(sensor_config[obj_name]['bus_name'],obj_name)

		## member variables
		self.object_name = obj_name
		interface_name = get_interface(obj_name)
		self.interface = dbus.Interface(obj,interface_name)
		self.value = self.interface.getValue()
		self.upper_critical_threshold = None
		self.lower_critical_threshold = None
		self.upper_warning_threshold = None
		self.lower_warning_threshold = None
		self.threshold_state = NORMAL
		self.emit_warning = warning_callback
		self.emit_critical = critical_callback
		
		## add signal handler to update cached value when sensor changes
		bus.add_signal_receiver(self.sensor_changed_signal_handler,
					dbus_interface = interface_name, signal_name = "Changed")
 						
	def set_thresholds(self,lc,lw,uw,uc):
		self.upper_critical_threshold = uc
		self.lower_critical_threshold = lc
		self.upper_warning_threshold = uw
		self.lower_warning_threshold = lw
	
	def set_config(self,data):
		self.interface.setConfigData(data)

	## called when associated sensor dbus object emits a Changed signal
	def sensor_changed_signal_handler(self,value):
		## update value from signal data
		self.value = value
		
		## check thresholds
		state = NORMAL
		if (value < self.lower_critical_threshold):
			state = LOWER_CRITICAL
		elif (value < self.lower_warning_threshold):
			state = LOWER_WARNING
		elif (value > self.upper_critical_threshold):
			state = UPPER_CRITICAL
		elif (value > self.upper_warning_threshold):
			state = UPPER_WARNING
		## only emit signal if threshold state has changed
		if (state != self.threshold_state):
			self.threshold_state = state
			if (state == LOWER_CRITICAL or state == UPPER_CRITICAL):
				self.emit_critical(self.object_name)
			if (state == LOWER_WARNING or state == UPPER_WARNING):
				self.emit_warning(self.object_name)


	def get_value(self):
		return self.value	


class SensorManagement(dbus.service.Object):
	def __init__(self,bus,name):
		dbus.service.Object.__init__(self,bus,name)
		self.sensor_cache = {}
		try:
			for objname in sensor_config.keys():
				print "Loading: "+objname
				sensor_new = Sensor(bus,objname,
						self.WarningThreshold,self.CriticalThreshold)
				sensor_new.set_thresholds(sensor_config[objname]['lower_critical'],
						 	  sensor_config[objname]['lower_warning'],
							  sensor_config[objname]['upper_warning'],
							  sensor_config[objname]['upper_critical'])
				if (sensor_config[objname].has_key('parameters')):
					sensor_new.set_config(sensor_config[objname]['parameters'])
				self.sensor_cache[objname] = sensor_new

		except dbus.exceptions.DBusException, e:
			# TODO: not sure what to do if can't find other services
			print "Unable to find dependent services: ",e


	@dbus.service.method("org.openbmc.SensorManagement",
		in_signature='s', out_signature='a{ss}')
	def getAllSensors(self,obj_name):
		return None
		
	@dbus.service.method("org.openbmc.SensorManagement",
		in_signature='s', out_signature='i')
	def getSensorValue(self,obj_name):
		sensor = self.sensor_cache[obj_name]
		return sensor.get_value()

	@dbus.service.signal('org.openbmc.SensorManagement')
	def CriticalThreshold(self, obj):
		print "Critical: "+obj

	@dbus.service.signal('org.openbmc.SensorManagement')
	def WarningThreshold(self, obj):
		print "Warning: "+obj


if __name__ == '__main__':
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
    bus = dbus.SessionBus()
    name = dbus.service.BusName("org.openbmc.SensorManagement",bus)
    obj = SensorManagement(bus,'/org/openbmc/SensorManagement')
    mainloop = gobject.MainLoop()
    
    print "Running SensorManagementService"
    mainloop.run()

