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
	print "Usage:  fru_manager.py [system name]"
	exit(1)
System = __import__(sys.argv[1])
import Openbmc

DBUS_NAME = 'org.openbmc.managers.Frus'
OBJ_NAME = '/org/openbmc/managers/Frus'
FRU_PATH = System.FRU_PATH

class Fru:
	def __init__(self,fru_id,data):
		if (data.has_key('ftype') == False):
			raise Exception("Fru must have ftype")

		self.fru_id = fru_id
		self.data = { 'fru_id' : fru_id }
		self.ftype = data['ftype']
		self.update(data)
	
	def getField(self,field):
		return self.data[field]

	def getId(self):
		return self.fru_id

	def update(self,data):
		for k in data.keys():
			self.data[k] = data[k]

	def isCached(self):
		is_cached = False
		if (self.data.has_key('cache')):
			if (self.data['cache']):
				is_cached = True
		return is_cached
	
	def saveToCache(self):
		if (self.isCached() == False):
			return
		global FRU_PATH
		print "Caching: "+str(self.fru_id)
		filename = FRU_PATH+"fru_"+str(self.fru_id)
		output = open(filename, 'wb')
		## just pickle dict not whole object
		cPickle.dump(self.data,output)
		output.close()		

	def loadFromCache(self):
		if (self.isCached() == False):
			return;
		## overlay with pickled data
		global FRU_PATH
		filename = FRU_PATH+"fru_"+str(self.fru_id)

		if (os.path.isfile(filename)):
			print "Loading from cache: "+filename
			p = open(filename, 'rb')
			data2 = cPickle.load(p)
			for k in data2.keys():
				self.data[k] = data2[k]
	def toJson(self):
		return json.dumps(self.data)

	def __str__(self):	
		r = "Fru: "+str(self.fru_id)+"\n"
		for f in self.data.keys():
			r = r+f+" = "+str(self.data[f])+"\n"
		return r

		

class FruManager(dbus.service.Object):
	def __init__(self,bus,name):
		dbus.service.Object.__init__(self,bus,name)
		
		bus.add_signal_receiver(self.UpdateFruHandler,
					signal_name = 'UpdateFru')

		self.fru_db = {}
		for fid in System.FRUS.keys():
			self.updateFru(fid,System.FRUS[fid])
			
			
	#@dbus.service.signal(DBUS_NAME)
	#def OpenBmcRunning(self):
	#	pass
	def UpdateFruHandler(self,fru_id,data):
		self.updateFru(fru_id,data)		

	@dbus.service.method(DBUS_NAME,
		in_signature='isv', out_signature='')
	def updateFruField(self,fru_id,field,value):
		data = { field : value }
		self.updateFru(fru_id,data)

	@dbus.service.method(DBUS_NAME,
		in_signature='ia{sv}', out_signature='')
	def updateFru(self,fru_id,data):
		## translate dbus data into basic data types
		for k in data.keys():
			d = Openbmc.DbusProperty(k,data[k])
			data[k] = d.getBaseValue()

		if (self.fru_db.has_key(fru_id)):
			## update properties then save to cache
			print "Updating FRU: "+str(fru_id)
			self.fru_db[fru_id].update(data)
			self.fru_db[fru_id].saveToCache()
		else:
			## fru doesn't exist, so add
			## then overlay with data from cache
			print "Adding FRU: "+str(fru_id)
			fru = Fru(fru_id,data)
			self.fru_db[fru_id] = fru
			fru.loadFromCache()


	@dbus.service.method(DBUS_NAME,
		in_signature='', out_signature='s')
	def getFrus(self):
		r = ""
		for f in self.fru_db.keys():
			r=r+"["+self.fru_db[f].toJson()+"],"
		return r


if __name__ == '__main__':
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
    bus = dbus.SessionBus()
    name = dbus.service.BusName(DBUS_NAME,bus)
    obj = FruManager(bus,OBJ_NAME)
    mainloop = gobject.MainLoop()

    print "Running Fru Manager"
    mainloop.run()

