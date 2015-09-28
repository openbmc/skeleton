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
	print "Usage:  inventory_manager.py [system name]"
	exit(1)
System = __import__(sys.argv[1])
import Openbmc

DBUS_NAME = 'org.openbmc.managers.Inventory'
OBJ_NAME = '/org/openbmc/managers/Inventory'
FRUS = System.FRU_INSTANCES
FRU_PATH = System.FRU_PATH

## accessor class to FRU data structure
class Fru:
	def __init__(self,fru):
		## validation
		if (FRUS.has_key(fru) == False):
			# TODO: event log
			raise Exception("Invalid FRU path: "+fru)
		
		self.fru = fru
	
	def getField(self,field):
		if (FRUS[self.fru].has_key(field) == False):
			# TODO: event log
			raise Exception("Invalid field: "+field)
			
		return FRUS[self.fru][field]

	def isFru(self):
		return FRUS[self.fru]['fru']

	def update(self,data):
		for k in data.keys():
			FRUS[self.fru][k] = data[k]

	def isCached(self):
		return True

	def getCacheFilename(self):
		global FRU_PATH
		filename = FRU_PATH+self.fru.replace('/','.')
		return filename
	
	def saveToCache(self):
		if (self.isCached() == False):
			return
		print "Caching: "+self.fru
		output = open(self.getCacheFilename(), 'wb')
		## just pickle dict not whole object
		print FRUS[self.fru]
		cPickle.dump(FRUS[self.fru],output)
		output.close()

	def loadFromCache(self):
		if (self.isCached() == False):
			return;
		## overlay with pickled data
		filename=self.getCacheFilename()
		if (os.path.isfile(filename)):
			print "Loading from cache: "+filename
			p = open(filename, 'rb')
			data2 = cPickle.load(p)
			for k in data2.keys():
				FRUS[self.fru][k] = data2[k]

	def __str__(self):	
		r = "Fru: "+str(self.fru_id)+"\n"
		for f in self.data.keys():
			r = r+f+" = "+str(self.data[f])+"\n"
		return r

		

class InventoryManager(dbus.service.Object):
	def __init__(self,bus,name):
		dbus.service.Object.__init__(self,bus,name)
		
		bus.add_signal_receiver(self.UpdateFruHandler,
					dbus_interface = "org.openbmc.sensors.IpmiBt", 
					signal_name = 'UpdateFru')

		bus.add_signal_receiver(self.SetSensorHandler, 
					dbus_interface = "org.openbmc.sensors.IpmiBt", 
					signal_name = "SetSensor")

		self.fru_db = {}
		self.fru_id_lookup = {}
		self.sensor_id_lookup = {}

		for fru_path in FRUS.keys():
			self.addFru(fru_path)
			f = FRUS[fru_path]
			if (f.has_key('fru_id')):
				self.fru_id_lookup[f['fru_id']] = fru_path
			if (f.has_key('sensor_id')):
				self.sensor_id_lookup[f['sensor_id']] = fru_path

			
	def UpdateFruHandler(self,fru_id,data):
		self.updateFruFromId(fru_id,data)		

	def SetSensorHandler(self,sensor_id,data):
		fru_path = self.getFruSensor(sensor_id)
		if (fru_path != ""):
			state = { 'state' : data }
			self.updateFru(fru_path,state)
			
		
	@dbus.service.method(DBUS_NAME,
		in_signature='y', out_signature='s')	
	def getFruSensor(self,sensor_id):
		if (self.sensor_id_lookup.has_key(sensor_id) == False):
			return ""
		return self.sensor_id_lookup[sensor_id]
		
	def addFru(self,fru_path):
		new_fru = Fru(fru_path)
		new_fru.loadFromCache()
		self.fru_db[fru_path] = new_fru
				
	@dbus.service.method(DBUS_NAME,
		in_signature='ia{sv}', out_signature='')
	def updateFruFromId(self,fru_id,data):
		iid = int(fru_id)
		if (self.fru_id_lookup.has_key(iid) == False):
			# TODO: event log
			print "fru id "+str(iid)+" not found"
		else:
			self.updateFru(self.fru_id_lookup[iid],data)
		

	@dbus.service.method(DBUS_NAME,
		in_signature='sa{sv}', out_signature='')
	def updateFru(self,fru_path,data):
		## translate dbus data into basic data types
		clean_data = {}
		for k in data.keys():
			d = Openbmc.DbusProperty(k,data[k])
			clean_data[str(k)] = d.getBaseValue()

		if (self.fru_db.has_key(fru_path)):
			## update properties then save to cache
			print "Updating FRU: "+fru_path
			self.fru_db[fru_path].update(clean_data)
			self.fru_db[fru_path].saveToCache()


	@dbus.service.method(DBUS_NAME,
		in_signature='', out_signature='a{sa{sv}}')
	def getFrus(self):
		return FRUS


if __name__ == '__main__':
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
    bus = dbus.SessionBus()
    name = dbus.service.BusName(DBUS_NAME,bus)
    obj = InventoryManager(bus,OBJ_NAME)
    mainloop = gobject.MainLoop()

    print "Running Inventory Manager"
    mainloop.run()

