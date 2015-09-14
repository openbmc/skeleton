#!/usr/bin/env python

import sys
import subprocess
from gi.repository import GObject

import dbus
import dbus.service
import dbus.mainloop.glib
import PropertyManager

if (len(sys.argv) < 2):
	print "Usage:  ipmi_manager.py [system name]"
	exit(1)

System = __import__(sys.argv[1])
import Openbmc

DBUS_NAME = 'org.openbmc.managers.Ipmi'
OBJ_NAME = '/org/openbmc/managers/Ipmi'


class IpmiManager(dbus.service.Object):
	def __init__(self,bus,name):
		dbus.service.Object.__init__(self,bus,name)

	## IPMI commands
	@dbus.service.method(DBUS_NAME,
		in_signature='yv', out_signature='')
	def setSensor(self,sensor_id,value):
		intf_sens = Openbmc.getManagerInterface(bus,"Sensors")
		intf_sens.setSensorFromId(sensor_id,value)

	@dbus.service.method(DBUS_NAME,
		in_signature='y', out_signature='v')
	def getSensor(self,sensor_id):
		intf_sens = Openbmc.getManagerInterface(bus,"Sensors")
		return intf_sens.getSensorFromId(sensor_id)

	@dbus.service.method(DBUS_NAME,
		in_signature='ia{sv}', out_signature='')
	def updateFru(self,fru_id,data):
		intf_fru = Openbmc.getManagerInterface(bus,"Frus")
		intf_fru.updateFru(fru_id,data)

	@dbus.service.method(DBUS_NAME,
		in_signature='', out_signature='s')
	def getFrus(self):
		intf_fru = Openbmc.getManagerInterface(bus,"Frus")
		return intf_fru.getFrus()


	@dbus.service.method(DBUS_NAME,
		in_signature='', out_signature='')
	def pokeHostWatchdog(self):
		## TODO don't do hardcoding
		obj =  bus.get_object('org.openbmc.watchdog.Host',
				'/org/openbmc/watchdog/HostWatchdog_0')
		intf = dbus.Interface(obj, 'org.openbmc.Watchdog' )
		intf.poke()
		return None

	@dbus.service.method(DBUS_NAME,
		in_signature='', out_signature='')
	def startHostWatchdog(self):
		## TODO don't do hardcoding
		obj =  bus.get_object('org.openbmc.watchdog.Host',
				'/org/openbmc/watchdog/HostWatchdog_0')
		intf = dbus.Interface(obj, 'org.openbmc.Watchdog' )
		intf.start()
		return None

	@dbus.service.method(DBUS_NAME,
		in_signature='', out_signature='')
	def powerOn(self):
		## TODO don't do hardcoding
		obj =  bus.get_object('org.openbmc.control.Chassis',
				'/org/openbmc/control/Chassis')
		intf = dbus.Interface(obj, 'org.openbmc.control.Chassis' )
		intf.powerOn()
		return None

	@dbus.service.method(DBUS_NAME,
		in_signature='', out_signature='')
	def powerOff(self):
		## TODO don't do hardcoding
		obj =  bus.get_object('org.openbmc.control.Chassis',
				'/org/openbmc/control/Chassis')
		intf = dbus.Interface(obj, 'org.openbmc.control.Chassis' )
		intf.powerOff()
		return None




if __name__ == '__main__':
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
    bus = dbus.SessionBus()
    name = dbus.service.BusName(DBUS_NAME,bus)
    obj = IpmiManager(bus,OBJ_NAME)
    mainloop = GObject.MainLoop()

    print "Running IpmiManager"
    mainloop.run()

