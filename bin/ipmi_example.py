#!/usr/bin/env python

import sys
import subprocess
import gobject
import dbus
import dbus.service
import dbus.mainloop.glib
import PropertyManager

import Openbmc

DBUS_NAME = 'org.openbmc.sensors.IpmiBt'
OBJ_NAME = '/org/openbmc/sensors/IpmiBt'

class IpmiBt(dbus.service.Object):
	def __init__(self,bus,name):
		dbus.service.Object.__init__(self,bus,name)

	@dbus.service.signal('org.openbmc.sensors.IpmiBt')
	def SetSensor(self, ipmi_id, value):
        	pass

	@dbus.service.signal('org.openbmc.sensors.IpmiBt')
	def UpdateFru(self, ipmi_id, data):
        	pass


def getWatchdog():
	obj =  bus.get_object('org.openbmc.watchdog.Host',
			'/org/openbmc/watchdog/HostWatchdog_0')
	intf = dbus.Interface(obj, 'org.openbmc.Watchdog' )
	return intf

def getChassisControl():
	obj =  bus.get_object('org.openbmc.control.Chassis',
			'/org/openbmc/control/Chassis')
	intf = dbus.Interface(obj, 'org.openbmc.control.Chassis' )
	return intf

def prettyPrint(data):
	for k in data.keys():
		print k
		for k2 in data[k].keys():
			print "\t"+k2+" = "+str(data[k][k2])



if __name__ == '__main__':
	dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
 	bus = dbus.SessionBus()
	name = dbus.service.BusName(DBUS_NAME,bus)
	obj = IpmiBt(bus,OBJ_NAME)
	mainloop = gobject.MainLoop()

	cmd = sys.argv[1]
	data = None
	ipmi_id = dbus.Byte(0)
	if (len(sys.argv) > 2):
		ipmi_id = dbus.Byte(int(sys.argv[2]))
	if (len(sys.argv)>3):
		data = sys.argv[3]

	if (cmd == "poweron"):
		intf = getChassisControl()
		intf.powerOn()
	elif (cmd == "poweroff"):
		intf = getChassisControl()
		intf.powerOff()
	elif (cmd == "setsensor"):
		obj.SetSensor(ipmi_id,dbus.Byte(int(data)))
	elif (cmd == "getsensors"):
		intf_sens = Openbmc.getManagerInterface(bus,"Sensors")
		data = intf_sens.getSensors()
		prettyPrint(data)
	elif (cmd == "updatefru"):
		d = { 'manufacturer' : data }	
		obj.UpdateFru(ipmi_id,d)
	elif (cmd == "getfrus"):
		intf_fru = Openbmc.getManagerInterface(bus,"Inventory")
		data = intf_fru.getFrus()
		prettyPrint(data)
	elif (cmd == "pokewatchdog"):
		intf = self.getWatchdog()
		intf.poke()
	elif (cmd == "statewatchdog"):
		intf = self.getWatchdog()
		intf.start()
	else:
		print "Unsupported command"






