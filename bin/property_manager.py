#!/usr/bin/python -u

import sys
#from gi.repository import GObject
import gobject
import dbus
import dbus.service
import dbus.mainloop.glib
import cPickle
import glob
import os

if (len(sys.argv) < 2):
	print "Usage:  property_manager.py [system name]"
	exit(1)
System = __import__(sys.argv[1])
import Openbmc

DBUS_NAME = 'org.openbmc.managers.Property'
OBJ_NAME = '/org/openbmc/managers/Property'
INTF_NAME = 'org.openbmc.managers.Property'

class PropertyManager(dbus.service.Object):
	def __init__(self,bus,name):
		dbus.service.Object.__init__(self,bus,name)
		if not os.path.exists(System.CACHE_PATH):
   			os.makedirs(System.CACHE_PATH)

		
		bus.add_signal_receiver(self.PropertyChangedHandler,
			dbus_interface = 'org.freedesktop.DBus.Properties', 
			signal_name = 'PropertiesChanged', sender_keyword='bus_name', path_keyword='path')

		bus.add_signal_receiver(self.RegisterPersistantInterface,
			dbus_interface = 'org.openbmc.PersistantInterface', 
			signal_name = 'Register', sender_keyword='bus_name', path_keyword='path')
		
		self.registered_interfaces = {}	


	def RegisterPersistantInterface(self,interface_name, bus_name = None, path = None):
		interface_name = str(interface_name)
		print "Registering cached object (interface): "+path+" ("+interface_name+")"
		self.registered_interfaces[interface_name] = True
		self.loadFromCache(bus_name,path,interface_name)
		

	def PropertyChangedHandler(self, interface_name, changed_properties, 
		invalidated_properties, bus_name = None, path = None):
		## TODO: just save all properties, probably should journal changes instead
		if (self.registered_interfaces.has_key(interface_name)):
			self.saveToCache(bus_name,path,interface_name)

	def getCacheFilename(self,obj_path,intf_name):
		name = obj_path.replace('/','.')
		filename = System.CACHE_PATH+name[1:]+"@"+intf_name+".props"
		return filename

	def saveToCache(self, bus_name, object_path, interface_name):
		print "Caching: "+object_path
		try:
			obj = bus.get_object(bus_name,object_path)
			intf = dbus.Interface(obj,"org.freedesktop.DBus.Properties")
			props = intf.GetAll(interface_name)	
			output = open(self.getCacheFilename(object_path,interface_name), 'wb')
			## save properties
			dbus_props = {}
			for p in props.keys():
				dbus_prop = Openbmc.DbusVariable(p,props[p])
				dbus_props[str(p)] = dbus_prop.getBaseValue()
			cPickle.dump(dbus_props,output)
		except Exception as e:
			print "ERROR: "+str(e)
		finally:
			output.close()

	def loadFromCache(self,bus_name, object_path, interface_name):
		## overlay with pickled data
		filename=self.getCacheFilename(object_path,interface_name)
		if (os.path.isfile(filename)):
			print "Loading from cache: "+filename
			try:			
				p = open(filename, 'rb')
				data = cPickle.load(p)
				obj = bus.get_object(bus_name,object_path)
				## TODO: don't use exception to determine whether interface is implemented
				try:
					intf = dbus.Interface(obj,"org.openbmc.Object.Properties")
					props = intf.SetMultiple(interface_name,data)
				except TypeError as t:
					print "SetMultiple interface doesn't exist, doing 1 set a time"
					intf = dbus.Interface(obj,dbus.PROPERTIES_IFACE)
					for prop in data:
						intf.Set(interface_name,prop,data[prop])
						
							
			except Exception as e:
				print "ERROR: Loading cache file: " +str(e)
			finally:
				p.close()
	
				
if __name__ == '__main__':
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
    bus = Openbmc.getDBus()
    name = dbus.service.BusName(DBUS_NAME,bus)
    obj = PropertyManager(bus,OBJ_NAME)
    mainloop = gobject.MainLoop()

    print "Running Property Manager"
    mainloop.run()

