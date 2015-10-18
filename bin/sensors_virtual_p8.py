#!/usr/bin/env python

import sys
#from gi.repository import GObject
import gobject
import dbus
import dbus.service
import dbus.mainloop.glib
import Openbmc

DBUS_NAME = 'org.openbmc.sensor.Power8Virtual'
OBJ_NAME = '/org/openbmc/sensor/virtual/'

class SensorValue(Openbmc.DbusProperties):
	IFACE_NAME = 'org.openbmc.SensorValue'
	def __init__(self):
		Openbmc.DbusProperties.__init__(self)
		self.Set(SensorValue.IFACE_NAME,'units',"")
		
	@dbus.service.method(IFACE_NAME,
		in_signature='v', out_signature='')
	def setValue(self,value):
		changed = False
		try:
			old_value = self.Get(SensorValue.IFACE_NAME,'value')
			if (value != old_value):
				changed = True
		except:
			changed = True

		if (changed == True):
			self.Set(SensorValue.IFACE_NAME,'value',value)
			self.Changed(self.getValue(),self.getUnits())



	@dbus.service.method(IFACE_NAME,
		in_signature='', out_signature='v')
	def getValue(self):
		return self.Get(SensorValue.IFACE_NAME,'value')

	@dbus.service.method(IFACE_NAME,
		in_signature='', out_signature='s')
	def getUnits(self):
		return self.Get(SensorValue.IFACE_NAME,'units')

	@dbus.service.signal(IFACE_NAME,signature='vs')
	def Changed(self,value,units):
		pass





class BootProgress(SensorValue):
	def __init__(self,bus,name):
		SensorValue.__init__(self)
		self.object_name = "BootProgress"
		dbus.service.Object.__init__(self,bus,name+self.object_name)
		
class HostStatus(SensorValue):
	def __init__(self,bus,name):
		SensorValue.__init__(self)
		self.object_name = "HostStatus"
		dbus.service.Object.__init__(self,bus,name+self.object_name)

class OsStatus(SensorValue):
	def __init__(self,bus,name):
		SensorValue.__init__(self)
		self.object_name = "OperatingSystemStatus"
		dbus.service.Object.__init__(self,bus,name+self.object_name)

class BootCount(SensorValue):
	def __init__(self,bus,name):
		SensorValue.__init__(self)
		self.object_name = "BootCount"
		dbus.service.Object.__init__(self,bus,name+self.object_name)

class OccStatus(SensorValue):
	def __init__(self,bus,name):
		SensorValue.__init__(self)
		self.object_name = "OccStatus"
		dbus.service.Object.__init__(self,bus,name+self.object_name)

				
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
    
    boot_progress.setValue("INACTIVE")
    host_status.setValue("OFF")
    os_status.setValue("OFF")
    boot_count.setValue(0)
    occ_status.setValue(0)

    print "Starting virtual sensors"
    mainloop.run()

