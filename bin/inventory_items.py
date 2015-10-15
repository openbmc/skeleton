#!/usr/bin/env python

import os
import sys
import gobject
import dbus
import dbus.service
import dbus.mainloop.glib
import cPickle
import json

if (len(sys.argv) < 2):
	print "Usage:  inventory_items.py [system name]"
	exit(1)
System = __import__(sys.argv[1])
import Openbmc


INTF_NAME = 'org.openbmc.InventoryItem'
DBUS_NAME = 'org.openbmc.managers.Inventory'

FRUS = System.FRU_INSTANCES
FRU_PATH = System.FRU_PATH

class Inventory(dbus.service.Object):
	def __init__(self,bus,name):
		global FRU_PATH
		dbus.service.Object.__init__(self,bus,name)
		if not os.path.exists(FRU_PATH):
   			os.makedirs(FRU_PATH)


		self.objects = [ ]

	def addItem(self,item):
		self.objects.append(item)

	@dbus.service.method("org.openbmc.managers.Inventory",
		in_signature='', out_signature='a{sa{sv}}')
	def getItems(self):
		tmp_obj = {}
		for item in self.objects:
			tmp_obj[str(item.item['name'])]=item.getItemDict()
		return tmp_obj
			


class InventoryItem(dbus.service.Object):
	def __init__(self,bus,name):
		dbus.service.Object.__init__(self,bus,name)
		## store all properties in a dict so can easily
		## send on dbus
		self.item = {
			'name' : name,
			'is_fru' : False,
			'fru_type' : 0,
			'state'  : 0,
			'manufacturer' : "",
		}
		self.cache = True

	def getItemDict(self):
		return self.item

	@dbus.service.method('org.openbmc.InventoryItem',
		in_signature='a{sv}', out_signature='')
	def update(self,data):
		## translate dbus data into basic data types
		for k in data.keys():
			d = Openbmc.DbusProperty(k,data[k])
			self.item[str(k)] = d.getBaseValue()
		self.saveToCache()

	@dbus.service.method("org.openbmc.InventoryItem",
		in_signature='s', out_signature='')
	def setPresent(self,present):
		self.item['present'] = present
		print "Set Present: "+str(present)

	@dbus.service.method("org.openbmc.InventoryItem",
		in_signature='s', out_signature='')
	def setFault(self,fault):
		self.item['fault_state'] = fault
		print "Set Fault: "+str(fault)

	def setField(self,field,value):
		self.item[field] = value

	def isCached(self):
		return self.cache

	def getCacheFilename(self):
		global FRU_PATH
		name = self.item['name'].replace('/','.')
		filename = FRU_PATH+name[1:]+".fru"
		return filename
	
	def saveToCache(self):
		if (self.isCached() == False):
			return
		print "Caching: "+self.item['name']
		try: 
			output = open(self.getCacheFilename(), 'wb')
			## just pickle dict not whole object
			cPickle.dump(self.item,output)
		except Exception as e:
			print "ERROR: "+str(e)
		finally:
			output.close()

	def loadFromCache(self):
		if (self.isCached() == False):
			return;
		## overlay with pickled data
		filename=self.getCacheFilename()
		if (os.path.isfile(filename)):
			print "Loading from cache: "+filename
			try:	
				p = open(filename, 'rb')
				data2 = cPickle.load(p)
				for k in data2.keys():
					self.item[k] = data2[k]
			except Exception as e:
				print "No cache file found: " +str(e)
			finally:
				p.close()


if __name__ == '__main__':
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
    bus = Openbmc.getDBus()
    name = dbus.service.BusName(DBUS_NAME,bus)
    mainloop = gobject.MainLoop()
    obj_parent = Inventory(bus, '/org/openbmc/managers/Inventory')

    for f in FRUS.keys():
	obj_path=f.replace("<inventory_root>",System.INVENTORY_ROOT)
    	obj = InventoryItem(bus,obj_path)
	obj.setField('is_fru',FRUS[f]['is_fru'])
	obj.setField('fru_type',FRUS[f]['fru_type'])
	obj.loadFromCache();
	obj_parent.addItem(obj)
	
    print "Running Inventory Manager"
    mainloop.run()

