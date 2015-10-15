#!/usr/bin/env python

import sys
import subprocess
import gobject
import dbus
import dbus.service
import dbus.mainloop.glib
import PropertyManager

import Openbmc

SENSOR_INTERFACE = "org.openbmc.SensorValue"

class IpmiBt(dbus.service.Object):
	def __init__(self,bus,name):
		dbus.service.Object.__init__(self,bus,name)


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
	#dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
	bus = Openbmc.getDBus()

	#name = dbus.service.BusName(DBUS_NAME,bus)
	#mainloop = gobject.MainLoop()

	cmd = sys.argv[1]
	data = None
	ipmi_id = dbus.Byte(0)
	if (len(sys.argv) > 2):
		ipmi_id = sys.argv[2]
	if (len(sys.argv)>3):
		data = sys.argv[3]

	if (cmd == "poweron"):
		intf = getChassisControl()
		intf.powerOn()
	elif (cmd == "poweroff"):
		intf = getChassisControl()
		intf.powerOff()
	elif (cmd == "setsensor"):
		intf_sys = Openbmc.getManagerInterface(bus,"System")
		obj_info = intf_sys.getObjectFromByteId("SENSOR",chr(int(ipmi_id)))
		obj_path = obj_info[1]
		bus_name = obj_info[0]
		if (obj_path != "" and bus_name != ""):
			obj = bus.get_object(bus_name,obj_path)
			intf = dbus.Interface(obj,)
			intf.setValue(dbus.Byte(int(data)))	
			
	elif (cmd == "getsensors"):
		intf_sens = Openbmc.getManagerInterface(bus,"Sensors")
		data = intf_sens.getSensors()
		prettyPrint(data)
	elif (cmd == "updatefru"):
		d = { 'manufacturer' : data }	
		intf_sys = Openbmc.getManagerInterface(bus,"System")
		c = chr(int(ipmi_id))
		obj_info = intf_sys.getObjectFromByteId("FRU",chr(int(ipmi_id)))
		obj_path = obj_info[1]
		bus_name = obj_info[0]
		if (obj_path != "" and bus_name != ""):
			obj = bus.get_object(bus_name,obj_path)
			intf = dbus.Interface(obj,"org.openbmc.InventoryItem")
			intf.update(d)	

	elif (cmd == "getfrus"):
		intf_fru = Openbmc.getManagerInterface(bus,"Inventory")
		data = intf_fru.getItems()
		for i in data:
			for k in data[i].keys():
				print k+" = "+str(data[i][k]) 
	elif (cmd == "updatefwftp"):
		obj = bus.get_object('org.openbmc.flash.Bios','/org/openbmc/flash/Bios_0')
		intf = dbus.Interface(obj,"org.openbmc.Flash")
		intf.updateViaTftp(sys.argv[2],sys.argv[3])
	elif (cmd == "updatefwfile"):
		obj = bus.get_object('org.openbmc.flash.Bios','/org/openbmc/flash/Bios_0')
		intf = dbus.Interface(obj,"org.openbmc.Flash")
		intf.update(sys.argv[2])
	elif (cmd == "fwstatus"):
		intf = Openbmc.getManagerInterface(bus,"Flash")
		status = intf.getStatus()
		for i in status:
			print i+" = "+status[i]
	elif (cmd == "pokewatchdog"):
		intf = self.getWatchdog()
		intf.poke()
	elif (cmd == "statewatchdog"):
		intf = self.getWatchdog()
		intf.start()
	else:
		print "Unsupported command"






