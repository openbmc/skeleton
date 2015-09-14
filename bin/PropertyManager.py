#!/usr/bin/env python

import sys
import cPickle
import os
import Openbmc
import dbus
import dbus.service
import dbus.mainloop.glib
from gi.repository import Gio, GLib, GObject


class PropertyManager():
	def __init__(self,bus,save_path):
		self.bus = bus
		self.save_path = save_path
	
	def loadProperties(self,bus_name,obj_path,properties):
		## Load properties from system config
		obj = self.bus.get_object(bus_name,obj_path)
		dbus_properties = dbus.Interface(obj, 'org.freedesktop.DBus.Properties')
		for prop_interface in properties.keys():
			for prop in properties[prop_interface]:
				tmp_val = dbus_properties.Get(prop_interface,prop)
				dbus_prop = Openbmc.DbusProperty(prop,tmp_val)
				value = properties[prop_interface][prop]
				dbus_prop.setValue(value)
				dbus_properties.Set(prop_interface,prop,dbus_prop.getValue())
			
			## if save file exists, overlay properties from file
			directory = obj_path.replace('/','.')
			directory = self.save_path+directory.lstrip('.')
			filename = directory+"/"+prop_interface
			if (os.path.isfile(filename) == False):
				pass
				## not an error	
				#print "No cache available for: "+filename
			else:
				try:
					print "Loading from disk: "+obj_path
					output = open(filename, 'rb')
					dbus_props = cPickle.load(output)
					output.close()
					save_properties = dbus.Interface(obj, 'org.freedesktop.DBus.Properties')
					for dbus_prop in dbus_props:
						save_properties.Set(prop_interface,dbus_prop.getName(),dbus_prop.getValue())
		
				except Exception as e:
					## TODO: Error handling
					print "Error loadFru: "+str(e)

		return None

	def saveProperties(self,bus_name,obj_path,interface_name,cache,properties):
		obj = self.bus.get_object(bus_name,obj_path)
		prop_intf = dbus.Interface(obj, 'org.freedesktop.DBus.Properties')

		for prop in properties.keys():
			print "Saving properties: "+prop
			## convert property to correct dbus type
			prop_intf.Set(interface_name,prop,properties[prop])

		dbus_props = []
		if (cache):
			print "Caching: "+obj_path
			all_properties = prop_intf.GetAll(interface_name)
			for prop in all_properties.keys():
				dbus_prop = Openbmc.DbusProperty(prop,all_properties[prop])
				dbus_props.append(dbus_prop)
		
			try:
				directory = obj_path.replace('/','.')
				directory = self.save_path+directory.lstrip('.')
				filename = directory+"/"+interface_name	
				if not os.path.exists(directory):
   					os.makedirs(directory)

				output = open(filename, 'wb')
				cPickle.dump(dbus_props,output)
				output.close()	
			except Exception as e:
				## TODO: error handling
				print str(e)
		
		return None


