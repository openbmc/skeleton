#!/usr/bin/env python

import sys
#from gi.repository import GObject
import gobject
import dbus
import dbus.service
import dbus.mainloop.glib
import Openbmc

SENSOR_INTERFACE = 'org.openbmc.SensorValue'
DBUS_NAME = 'org.openbmc.sensor.Power8Virtual'
OBJ_NAME = '/org/openbmc/sensor/virtual/'

class BootProgress(dbus.service.Object):
	def __init__(self,bus,name):
		self.object_name = "BootProgress"
		self.value = 0
		self.units = ""
		dbus.service.Object.__init__(self,bus,name+self.object_name)

	@dbus.service.method(SENSOR_INTERFACE,
		in_signature='v', out_signature='')
	def setValue(self,value):
		if (value != self.value):
			self.value=value
			self.Changed()

	@dbus.service.method(SENSOR_INTERFACE,
		in_signature='', out_signature='v')
	def getValue(self):
		return self.value;

	@dbus.service.method(SENSOR_INTERFACE,
		in_signature='', out_signature='s')
	def getUnits(self):
		return self.units;

	@dbus.service.signal(SENSOR_INTERFACE,signature='vs')
	def Changed(self,value,units):
		pass
		
class HostStatus(dbus.service.Object):
	def __init__(self,bus,name):
		self.object_name = "HostStatus"
		self.value = 0
		self.units = ""
		dbus.service.Object.__init__(self,bus,name+self.object_name)

	@dbus.service.method(SENSOR_INTERFACE,
		in_signature='v', out_signature='')
	def setValue(self,value):
		if (value != self.value):
			self.value=value
			self.Changed(self.value,self.units)

	@dbus.service.method(SENSOR_INTERFACE,
		in_signature='', out_signature='v')
	def getValue(self):
		return self.value;

	@dbus.service.method(SENSOR_INTERFACE,
		in_signature='', out_signature='s')
	def getUnits(self):
		return self.units;
		
	@dbus.service.signal(SENSOR_INTERFACE,signature='vs')
	def Changed(self,value,units):
		pass

class OsStatus(dbus.service.Object):
	def __init__(self,bus,name):
		self.object_name = "OperatingSystemStatus"
		self.value = 0
		self.units = ""
		dbus.service.Object.__init__(self,bus,name+self.object_name)

	@dbus.service.method(SENSOR_INTERFACE,
		in_signature='v', out_signature='')
	def setValue(self,value):
		if (value != self.value):
			self.value=value
			self.Changed(self.value,self.units)

	@dbus.service.method(SENSOR_INTERFACE,
		in_signature='', out_signature='v')
	def getValue(self):
		return self.value;

	@dbus.service.method(SENSOR_INTERFACE,
		in_signature='', out_signature='s')
	def getUnits(self):
		return self.units;
		
	@dbus.service.signal(SENSOR_INTERFACE,signature='vs')
	def Changed(self,value,units):
		pass
		
class BootCount(dbus.service.Object):
	def __init__(self,bus,name):
		self.object_name = "BootCount"
		self.value = 0
		self.units = ""
		dbus.service.Object.__init__(self,bus,name+self.object_name)

	@dbus.service.method(SENSOR_INTERFACE,
		in_signature='v', out_signature='')
	def setValue(self,value):
		if (value != self.value):
			self.value=value
			self.Changed(self.value,self.units)

	@dbus.service.method(SENSOR_INTERFACE,
		in_signature='', out_signature='v')
	def getValue(self):
		return self.value;

	@dbus.service.method(SENSOR_INTERFACE,
		in_signature='', out_signature='s')
	def getUnits(self):
		return self.units;
		
	@dbus.service.signal(SENSOR_INTERFACE,signature='vs')
	def Changed(self,value,units):
		pass


class OccStatus(dbus.service.Object):
	def __init__(self,bus,name):
		self.object_name = "OccStatus"
		self.value = 0
		self.units = ""
		dbus.service.Object.__init__(self,bus,name+self.object_name)

	@dbus.service.method(SENSOR_INTERFACE,
		in_signature='v', out_signature='')
	def setValue(self,value):
		if (value != self.value):
			self.value=value
			self.Changed(self.value,self.units)

	@dbus.service.method(SENSOR_INTERFACE,
		in_signature='', out_signature='v')
	def getValue(self):
		return self.value;

	@dbus.service.method(SENSOR_INTERFACE,
		in_signature='', out_signature='s')
	def getUnits(self):
		return self.units;
		
	@dbus.service.signal(SENSOR_INTERFACE,signature='vs')
	def Changed(self,value,units):
		pass

				
if __name__ == '__main__':
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
    bus = Openbmc.getDBus()
    name = dbus.service.BusName(DBUS_NAME,bus)
    boot_progress = BootProgress(bus,OBJ_NAME)
    host_status = HostStatus(bus,OBJ_NAME)
    os_status = OsStatus(bus,OBJ_NAME)
    boot_count = BootCount(bus,OBJ_NAME)
    occ_status = OccStatus(bus,OBJ_NAME)
    mainloop = gobject.MainLoop()

    print "Starting virtual sensors"
    mainloop.run()

