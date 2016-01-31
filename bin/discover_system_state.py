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
	'occstatus0' : { 
		'bus_name' : 'org.openbmc.Sensors',
		'object_name' : '/org/openbmc/sensors/host/cpu0/OccStatus',
		'interface_name' : 'org.openbmc.SensorValue'
	},
	'occstatus1' : { 
		'bus_name' : 'org.openbmc.Sensors',
		'object_name' : '/org/openbmc/sensors/host/cpu1/OccStatus',
		'interface_name' : 'org.openbmc.SensorValue'
	},
	'bootprogress' : { 
		'bus_name' : 'org.openbmc.Sensors',
		'object_name' : '/org/openbmc/sensors/host/BootProgress',
		'interface_name' : 'org.openbmc.SensorValue'
	},
	'chassis' : { 
		'bus_name' : 'org.openbmc.control.Chassis',
		'object_name' : '/org/openbmc/control/chassis0',
		'interface_name' : 'org.openbmc.control.Chassis'
	},
	'settings' : {
		'bus_name' : 'org.openbmc.settings.Host',
		'object_name' : '/org/openbmc/settings/host0',
		'interface_name' : 'org.freedesktop.DBus.Properties'
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
	intf = getInterface(bus,dbus_objects,'occstatus0')
	intf.setValue("Enabled")
	intf = getInterface(bus,dbus_objects,'occstatus1')
	intf.setValue("Enabled")
else:
	## Power is off, so check power policy
	settings_intf = getInterface(bus,dbus_objects,'settings')
	system_state = settings_intf.Get("org.openbmc.settings.Host","system_state")
	power_policy = settings_intf.Get("org.openbmc.settings.Host","power_policy")

	print "Last System State: "+system_state+";  Power Policy: "+power_policy
	chassis_intf = getInterface(bus,dbus_objects,'chassis')
	if (power_policy == "ALWAYS_POWER_ON" or
	   (power_policy == "RESTORE_LAST_STATE" and 
	    system_state =="HOST_POWERED_ON")):
		chassis_intf.powerOn()


