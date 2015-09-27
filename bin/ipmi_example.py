#!/usr/bin/env python

import sys
import subprocess
import gobject
import dbus
import dbus.service
import dbus.mainloop.glib
import PropertyManager

import Openbmc



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
 	bus = dbus.SessionBus()
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
		intf_sens = Openbmc.getManagerInterface(bus,"Sensors")
		intf_sens.setSensorFromId(ipmi_id,data)
	elif (cmd == "getsensor"):
		intf_sens = Openbmc.getManagerInterface(bus,"Sensors")
		print intf_sens.getSensorFromId(ipmi_id)
	elif (cmd == "getsensors"):
		intf_sens = Openbmc.getManagerInterface(bus,"Sensors")
		data = intf_sens.getSensors()
		prettyPrint(data)
	elif (cmd == "updatefru"):
		intf_fru = Openbmc.getManagerInterface(bus,"Inventory")
		intf_fru.updateFruFromId(ipmi_id,data)
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






