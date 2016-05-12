#!/usr/bin/python -u

import sys
#from gi.repository import GObject
import gobject
import dbus
import dbus.service
import dbus.mainloop.glib
from obmc.dbuslib.bindings import get_dbus, DbusProperties, DbusObjectManager

DBUS_NAME = 'org.openbmc.control.Fans'
OBJ_PATH = '/org/openbmc/control/fans'
IFACE_NAME = 'org.openbmc.control.Fans'

FAN_BUS = 'org.openbmc.Sensors'
FAN_OBJS = [
	'/org/openbmc/sensors/speed/fan0',
	'/org/openbmc/sensors/speed/fan1',
	'/org/openbmc/sensors/speed/fan2',
	'/org/openbmc/sensors/speed/fan3',
	'/org/openbmc/sensors/speed/fan4',
	'/org/openbmc/sensors/speed/fan5',
]
FAN_IFACE = 'org.openbmc.SensorValue'

class FanControl(DbusProperties,DbusObjectManager):
	def __init__(self,bus,name):
		DbusProperties.__init__(self)
		DbusObjectManager.__init__(self)
		dbus.service.Object.__init__(self,bus,name)
		self.Set(IFACE_NAME,"floor",250)
		self.Set(IFACE_NAME,"ceiling",255)
		self.fan_intf = []
		## create interface proxies to all fans
		for fan in FAN_OBJS:
			print "Initializing fan: "+fan
			obj = bus.get_object(FAN_BUS,fan,introspect=False)
			self.fan_intf.append(dbus.Interface(obj,FAN_IFACE))
		self.InterfacesAdded(name,self.properties)
			
	@dbus.service.method(DBUS_NAME,
		in_signature='', out_signature='')
	def setMax(self):
		print "Setting fans to max"
		for intf in self.fan_intf:
			intf.setValue(dbus.UInt32(255))
		

if __name__ == '__main__':
	
	dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
	bus = get_dbus()
	name = dbus.service.BusName(DBUS_NAME,bus)
	fan_control = FanControl(bus,OBJ_PATH)
	mainloop = gobject.MainLoop()
   
	print "Starting fan control"
	fan_control.setMax()
	mainloop.run()

