#!/usr/bin/env python

import sys
import subprocess
import gobject
import dbus
import dbus.service
import dbus.mainloop.glib
import xml.etree.ElementTree as ET
import PropertyManager

if (len(sys.argv) < 2):
	print "Usage:  ipmi_translator.py [system name]"
	exit(1)

System = __import__(sys.argv[1])
import Openbmc

DBUS_NAME = 'org.openbmc.managers.IpmiTranslator'
OBJ_NAME = '/org/openbmc/managers/IpmiTranslator/'+sys.argv[1]
ID_LOOKUP = {
	'BUS_NAME' : {},
	'FRU' : {},
	'SENSOR' : {}, 
}


class IpmiTranslator(dbus.service.Object):
	def __init__(self,bus,name):
		dbus.service.Object.__init__(self,bus,name)
		bus.add_signal_receiver(self.UpdateFruHandler,
					dbus_interface = 'org.openbmc.control.IpmiBt', 
					signal_name = "UpdateFru")
		bus.add_signal_receiver(self.UpdateSensorHandler,
					dbus_interface = 'org.openbmc.control.IpmiBt', 
					signal_name = "UpdateSensor")

		## generate fru and sensor id to dbus object path lookup
		for bus_name in System.SYSTEM_CONFIG.keys():
			obj_name = "/"+bus_name.replace('.','/')
			for instances in System.SYSTEM_CONFIG[bus_name]['instances']:
				obj_path = obj_name+"/"+instances['name']
				if (instances.has_key('sensor_id')):
					iid = instances['sensor_id']
					ID_LOOKUP['BUS_NAME'][iid] = bus_name
					ID_LOOKUP['SENSOR'][iid] = obj_path
				if (instances.has_key('fru_id')):
					iid = instances['fru_id']
					ID_LOOKUP['BUS_NAME'][iid] = bus_name
					ID_LOOKUP['FRU'][iid] = obj_path

				
	## TODO: Should be event driven instead of calling object methods because
	##       object could be hung
	def UpdateFruHandler(self,fru_id,data):
		if (ID_LOOKUP['FRU'].has_key(fru_id)):
			obj_path = ID_LOOKUP['FRU'][fru_id]
			bus_name = ID_LOOKUP['BUS_NAME'][fru_id]
			## save fru object to object and disk
			interface_name = 'org.openbmc.Fru'
			cache = System.CACHED_INTERFACES.has_key(interface_name)
			PropertyManager.saveProperties(bus,bus_name,obj_path,interface_name,cache,data)
		else:
			## TODO: error handling
			pass

	def UpdateSensorHandler(self,sensor_id,value):
		if (ID_LOOKUP['SENSOR'].has_key(sensor_id)):
			obj_path = ID_LOOKUP['SENSOR'][sensor_id]
			bus_name = ID_LOOKUP['BUS_NAME'][sensor_id]
			data = { 'value' : value }
			## save sensor value
			## TODO:  need to accomodate any sensor interface
			interface_name = 'org.openbmc.SensorInteger'
			#cache = System.CACHED_INTERFACES.has_key(interface_name)
			obj = bus.get_object(bus_name,obj_path)
			intf = dbus.Interface(obj, interface_name)
			#intf.setValue(value)
			PropertyManager.saveProperties(bus,bus_name,obj_path,interface_name,cache,data)
		else:
			## TODO: error handling
			pass

	@dbus.service.method(DBUS_NAME,
		in_signature='i', out_signature='i')
	def getSensor(self,sensor_id):
		val = 0
		if (ID_LOOKUP['SENSOR'].has_key(sensor_id)):
			obj_path = ID_LOOKUP['SENSOR'][sensor_id]
			bus_name = ID_LOOKUP['BUS_NAME'][sensor_id]
			print "getSensor: "+obj_path+","+bus_name
			## TODO don't do hardcoding
			obj =  bus.get_object('org.openbmc.managers.Sensors',
					'/org/openbmc/managers/Sensors/Barreleye')
			intf = dbus.Interface(obj, 'org.openbmc.managers.Sensors' )
			val = intf.getSensor(obj_path)
			print "value = "+str(val)

		return val


if __name__ == '__main__':
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
    bus = dbus.SessionBus()
    name = dbus.service.BusName(DBUS_NAME,bus)
    obj = IpmiTranslator(bus,OBJ_NAME)
    mainloop = gobject.MainLoop()

    print "Running IpmiTranslator"
    mainloop.run()

