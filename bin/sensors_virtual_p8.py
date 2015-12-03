#!/usr/bin/python -u

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
	def __init__(self,bus,name):
		Openbmc.DbusProperties.__init__(self)
		self.Set(SensorValue.IFACE_NAME,'units',"")
		dbus.service.Object.__init__(self,bus,name)
		self.ObjectAdded(name,SensorValue.IFACE_NAME)
		
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
		SensorValue.__init__(self,bus,name)

		
CONTROL_IFACE = 'org.openbmc.Control'
class HostStatusSensor(VirtualSensor):
	def __init__(self,bus,name):
		VirtualSensor.__init__(self,bus,name)

	##override setValue method
	@dbus.service.method(SensorValue.IFACE_NAME,
		in_signature='v', out_signature='')
	def setValue(self,value):
		SensorValue.setValue(self,value)
		if (value == "BLAH"):
			self.GotoSystemState("OS_BOOTED")
			
	@dbus.service.signal(CONTROL_IFACE,signature='s')
	def GotoSystemState(self,state):
		pass
		
class BootProgressSensor(VirtualSensor):
	def __init__(self,bus,name):
		VirtualSensor.__init__(self,bus,name)
		self.setValue("Off")
		bus.add_signal_receiver(self.SystemStateHandler,signal_name = "GotoSystemState")

	def SystemStateHandler(self,state):
		if (state == "HOST_POWERED_OFF"):
			self.setValue("Off")


	##override setValue method
	@dbus.service.method(SensorValue.IFACE_NAME,
		in_signature='v', out_signature='')
	def setValue(self,value):
		SensorValue.setValue(self,value)
		if (value == "FW Progress, Starting OS"):
			self.GotoSystemState("HOST_BOOTED")
			
	@dbus.service.signal(CONTROL_IFACE,signature='s')
	def GotoSystemState(self,state):
		pass
		
class OccActiveSensor(VirtualSensor):
	def __init__(self,bus,name):
		VirtualSensor.__init__(self,bus,name)
		self.setValue("Disabled")
		bus.add_signal_receiver(self.SystemStateHandler,signal_name = "GotoSystemState")

	def SystemStateHandler(self,state):
		if (state == "HOST_POWERED_OFF"):
			self.setValue("Disabled")
			

	##override setValue method
	@dbus.service.method(SensorValue.IFACE_NAME,
		in_signature='v', out_signature='')
	def setValue(self,value):
		SensorValue.setValue(self,value)
			
	@dbus.service.signal(CONTROL_IFACE,signature='s')
	def GotoSystemState(self,state):
		pass
		
				
if __name__ == '__main__':
	
	sensors = {
		'OperatingSystemStatus' : None,
		'BootCount' : None,
	}
	dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
	bus = Openbmc.getDBus()
	name = dbus.service.BusName(DBUS_NAME,bus)
	for instance in sensors.keys():
		sensors[instance]= VirtualSensor(bus,OBJ_PATH+instance)

	sensors['HostStatus'] = HostStatusSensor(bus,OBJ_PATH+"HostStatus")
	sensors['BootProgress'] = BootProgressSensor(bus,OBJ_PATH+"BootProgress")
	sensors['OccStatus'] = OccActiveSensor(bus,OBJ_PATH+"OccStatus")
	mainloop = gobject.MainLoop()
   
	## Initialize sensors 
 	sensors['HostStatus'].setValue("OFF")
	sensors['OperatingSystemStatus'].setValue("OFF")
	sensors['BootCount'].setValue(0)

	print "Starting virtual sensors"
	mainloop.run()

