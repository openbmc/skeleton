#!/usr/bin/env python

import sys
#from gi.repository import GObject
import gobject
import dbus
import dbus.service
import dbus.mainloop.glib
import Openbmc

DBUS_NAME = 'org.openbmc.sensor.Power8Virtual'
OBJ_PATH = '/org/openbmc/sensor/virtual/'

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

class VirtualSensor(SensorValue):
	def __init__(self,bus,name):
		SensorValue.__init__(self)
		dbus.service.Object.__init__(self,bus,name)
		
HS_IFACE = 'org.openbmc.HostStatus'
class HostStatusSensor(SensorValue):
	def __init__(self,bus,name):
		SensorValue.__init__(self)
		dbus.service.Object.__init__(self,bus,name)

	##override setValue method
	@dbus.service.method(SensorValue.IFACE_NAME,
		in_signature='v', out_signature='')
	def setValue(self,value):
		SensorValue.setValue(self,value)
		if (value == "BOOTED"):
			self.Booted()
			
	@dbus.service.signal(HS_IFACE,signature='')
	def Booted(self):
		pass
		

				
if __name__ == '__main__':
	
	sensors = {
		'BootProgress' : None,
		'OperatingSystemStatus' : None,
		'BootCount' : None,
		'OccStatus' : None,
	}
	dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
	bus = Openbmc.getDBus()
	name = dbus.service.BusName(DBUS_NAME,bus)
	for instance in sensors.keys():
		sensors[instance]= VirtualSensor(bus,OBJ_PATH+instance)

	sensors['HostStatus'] = HostStatusSensor(bus,OBJ_PATH+"HostStatus")
	mainloop = gobject.MainLoop()
   
	## Initialize sensors 
	sensors['BootProgress'].setValue("INACTIVE")
 	sensors['HostStatus'].setValue("OFF")
	sensors['OperatingSystemStatus'].setValue("OFF")
	sensors['BootCount'].setValue(0)
	sensors['OccStatus'].setValue(0)

	print "Starting virtual sensors"
	mainloop.run()

