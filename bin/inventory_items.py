#!/usr/bin/python -u

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
ENUM_INTF = 'org.openbmc.Object.Enumerate'

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

	@dbus.service.method(ENUM_INTF,
		in_signature='', out_signature='a{sa{sv}}')
	def enumerate(self):
		tmp_obj = {}
		for item in self.objects:
			tmp_obj[str(item.name)]=item.GetAll(INTF_NAME)
		return tmp_obj
			


class InventoryItem(Openbmc.DbusProperties):
	def __init__(self,bus,name):		
		Openbmc.DbusProperties.__init__(self)
		dbus.service.Object.__init__(self,bus,name)
		self.name = name
		self.cache = True
		self.Set(INTF_NAME,'is_fru',False)
		self.Set(INTF_NAME,'fru_type',0)
		self.Set(INTF_NAME,'present',"INACTIVE")
		self.Set(INTF_NAME,'fault',"NONE")
	
		
		
	@dbus.service.method(INTF_NAME,
		in_signature='a{sv}', out_signature='')
	def update(self,data):
		## translate dbus data into basic data types
		self.SetAll(INTF_NAME,data)
		#self.saveToCache()

	@dbus.service.method(INTF_NAME,
		in_signature='s', out_signature='')
	def setPresent(self,present):
		self.setField('present',present)

	@dbus.service.method(INTF_NAME,
		in_signature='s', out_signature='')
	def setFault(self,fault):
		self.setField('fault',fault)

	def setField(self,field,value):
		f = str(field)
		d = Openbmc.DbusVariable(f,value)
		self.Set(INTF_NAME,f,d.getBaseValue())

	def isCached(self):
		return self.cache

	def getCacheFilename(self):
		global FRU_PATH
		name = self.name.replace('/','.')
		filename = FRU_PATH+name[1:]+".fru"
		return filename
	
	def saveToCache(self):
		if (self.isCached() == False):
			return
		print "Caching: "+self.name
		try: 
			output = open(self.getCacheFilename(), 'wb')
			## save properties
			cPickle.dump(self.properties[INTF_NAME],output)
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
					self.setField(k,data2[k])
			except Exception as e:
				print "No cache file found: " +str(e)
			finally:
				p.close()


if __name__ == '__main__':
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
    bus = Openbmc.getDBus()
    name = dbus.service.BusName(DBUS_NAME,bus)
    mainloop = gobject.MainLoop()
    obj_parent = Inventory(bus, '/org/openbmc/inventory')

    for f in FRUS.keys():
	obj_path=f.replace("<inventory_root>",System.INVENTORY_ROOT)
    	obj = InventoryItem(bus,obj_path)
	obj.setField('is_fru',FRUS[f]['is_fru'])
	obj.setField('fru_type',FRUS[f]['fru_type'])
	#obj.loadFromCache();
	obj_parent.addItem(obj)
	
    print "Running Inventory Manager"
    mainloop.run()

