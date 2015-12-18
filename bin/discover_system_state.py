#!/usr/bin/python

import sys
import gobject
import dbus
import dbus.service
import dbus.mainloop.glib


dbus_objects = {
	'power' : { 
		'bus_name' : 'org.openbmc.control.Power',
		'object_name' : '/org/openbmc/control/power0',
		'interface_name' : 'org.openbmc.control.Power'
	},
	'occstatus' : { 
		'bus_name' : 'org.openbmc.Sensors',
		'object_name' : '/org/openbmc/sensors/host/OccStatus',
		'interface_name' : 'org.openbmc.SensorValue'
	},
	'bootprogress' : { 
		'bus_name' : 'org.openbmc.Sensors',
		'object_name' : '/org/openbmc/sensors/host/BootProgress',
		'interface_name' : 'org.openbmc.SensorValue'
	},
}

def getInterface(bus,objs,key):
	obj = bus.get_object(objs[key]['bus_name'],objs[key]['object_name'],introspect=False)
	return dbus.Interface(obj,objs[key]['interface_name'])

def getProperty(bus,objs,key,prop):
	obj = bus.get_object(objs[key]['bus_name'],objs[key]['object_name'],introspect=False)
	intf = dbus.Interface(obj,dbus.PROPERTIES_IFACE)
	return intf.Get(objs[key]['interface_name'],prop)


bus = dbus.SystemBus()
pgood = getProperty(bus,dbus_objects,'power','pgood')

if (pgood == 1):
	intf = getInterface(bus,dbus_objects,'bootprogress')
	intf.setValue("FW Progress, Starting OS")
	intf = getInterface(bus,dbus_objects,'occstatus')
	intf.setValue("Enabled")
	

		

