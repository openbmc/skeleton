#!/usr/bin/python -u

import os
import sys
import gobject
import dbus
import dbus.service
import dbus.mainloop.glib
import cPickle
import json
import PropertyCacher

if (len(sys.argv) < 2):
	print "Usage:  inventory_items.py [system name]"
	exit(1)
System = __import__(sys.argv[1])
import Openbmc


INTF_NAME = 'org.openbmc.InventoryItem'
DBUS_NAME = 'org.openbmc.managers.Inventory'
ENUM_INTF = 'org.openbmc.Object.Enumerate'

FRUS = System.FRU_INSTANCES

class Inventory(Openbmc.DbusProperties):
	def __init__(self,bus,name):
		dbus.service.Object.__init__(self,bus,name)
		self.objects = [ ]
		self.ObjectAdded(name,ENUM_INTF)		

	def addItem(self,item):
		self.objects.append(item)

	@dbus.service.method(ENUM_INTF,
		in_signature='', out_signature='a{sa{sv}}')
	def enumerate(self):
		tmp_obj = {}
		for item in self.objects:
			tmp_obj[str(item.name)]=item.GetAll(INTF_NAME)
		return tmp_obj
			


class InventoryItem(Openbmc.DbusProperties):
	def __init__(self,bus,name,is_fru,fru_type):		
		Openbmc.DbusProperties.__init__(self)
		dbus.service.Object.__init__(self,bus,name)

		self.name = name
		
		## this will load properties from cache
		PropertyCacher.load(name,INTF_NAME,self.properties)
		data = {'is_fru': is_fru, 'fru_type': fru_type, 'present': 'Inactive', 'fault': 'None', 'version': 'None' }
		self.SetMultiple(INTF_NAME,data)
		self.ObjectAdded(name,INTF_NAME)		
		
		
	@dbus.service.method(INTF_NAME,
		in_signature='a{sv}', out_signature='')
	def update(self,data):
		self.SetMultiple(INTF_NAME,data)
		PropertyCacher.save(self.name,INTF_NAME,self.properties)

	@dbus.service.method(INTF_NAME,
		in_signature='s', out_signature='')
	def setPresent(self,present):
		self.Set(INTF_NAME,'present',present)

	@dbus.service.method(INTF_NAME,
		in_signature='s', out_signature='')
	def setFault(self,fault):
		self.Set(INTF_NAME,'fault',fault)


def getVersion():
	version = "Error"
	with open('/etc/os-release', 'r') as f:
		for line in f:
			p = line.rstrip('\n')
			parts = line.rstrip('\n').split('=')
			if (parts[0] == "BUILD_ID"):
				version = parts[1]
	return version


if __name__ == '__main__':
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
    bus = Openbmc.getDBus()
    name = dbus.service.BusName(DBUS_NAME,bus)
    mainloop = gobject.MainLoop()
    obj_parent = Inventory(bus, '/org/openbmc/inventory')

    for f in FRUS.keys():
	obj_path=f.replace("<inventory_root>",System.INVENTORY_ROOT)
    	obj = InventoryItem(bus,obj_path,FRUS[f]['is_fru'],FRUS[f]['fru_type'])
	obj_parent.addItem(obj)

    	## TODO:  this is a hack to update bmc inventory item with version
    	## should be done by flash object
	if (FRUS[f]['fru_type'] == "BMC"):
		version = getVersion()
		obj.update({'version': version})

    print "Running Inventory Manager"
    mainloop.run()

