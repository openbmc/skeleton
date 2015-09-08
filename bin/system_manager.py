#!/usr/bin/env python

import sys
import subprocess
from gi.repository import GObject
import dbus
import dbus.service
import dbus.mainloop.glib
import os
import PropertyManager

import Openbmc

if (len(sys.argv) < 2):
	print "Usage:  system_manager.py [system name]"
	exit(1)

System = __import__(sys.argv[1])
import Openbmc

DBUS_NAME = 'org.openbmc.managers.System'
OBJ_NAME = '/org/openbmc/managers/System'
HEARTBEAT_CHECK_INTERVAL = 20000


class SystemManager(dbus.service.Object):
	def __init__(self,bus,name):
		dbus.service.Object.__init__(self,bus,name)
		self.property_manager = PropertyManager.PropertyManager(bus,System.CACHE_PATH)

		## Signal handlers
		bus.add_signal_receiver(self.NewBusHandler,
					dbus_interface = 'org.freedesktop.DBus', 
					signal_name = "NameOwnerChanged")
		bus.add_signal_receiver(self.CacheMeHandler,
					signal_name = 'CacheMe', path_keyword='path',interface_keyword='interface')
		bus.add_signal_receiver(self.HeartbeatHandler, signal_name = "Heartbeat")

		try:
			# launch dbus object processes
			for bus_name in System.SYSTEM_CONFIG.keys():
				self.start_process(bus_name)
		except Exception as e:
			## TODO: error handling
			pass
		
		## Add poll for heartbeat
    		GObject.timeout_add(HEARTBEAT_CHECK_INTERVAL, self.heartbeat_check)
		

	def CacheMeHandler(self,busname,path=None,interface=None):
		#interface_name = 'org.openbmc.Fru'
		print "CacheME: "+busname+","+path+","+interface
		data = {}
		cache = System.CACHED_INTERFACES.has_key(interface)
		self.property_manager.saveProperties(busname,path,interface,cache,data)


	def start_process(self,bus_name):
		if (System.SYSTEM_CONFIG[bus_name]['start_process'] == True):
			process_name = System.BIN_PATH+System.SYSTEM_CONFIG[bus_name]['process_name']
			cmdline = [ ]
			cmdline.append(process_name)
			for instance in System.SYSTEM_CONFIG[bus_name]['instances']:
				cmdline.append(instance['name'])
			try:
				print "Starting process: "+" ".join(cmdline)
				System.SYSTEM_CONFIG[bus_name]['popen'] = subprocess.Popen(cmdline);
			except Exception as e:
				## TODO: error
				print "Error starting process: "+" ".join(cmdline)

	def heartbeat_check(self):
		print "heartbeat check"
		for bus_name in System.SYSTEM_CONFIG.keys():
			if (System.SYSTEM_CONFIG[bus_name]['start_process'] == True):
				## even if process doesn't request heartbeat check, 
				##   make sure process is still alive
				p = System.SYSTEM_CONFIG[bus_name]['popen']
				p.poll()
				if (p.returncode != None):
					print "Process for "+bus_name+" appears to be dead"
					self.start_process(bus_name)
	
				## process is alive, now check if heartbeat received
				## during previous interval
				elif (System.SYSTEM_CONFIG[bus_name]['heartbeat'] == 'yes'):
					if (System.SYSTEM_CONFIG[bus_name]['heartbeat_count'] == 0):
						print "Heartbeat error: "+bus_name
						p = System.SYSTEM_CONFIG[bus_name]['popen']
						## TODO: error checking
						p.poll()
						if (p.returncode == None):
							print "Process must be hung, so killing"
							p.kill()
						
						self.start_process(bus_name)			
					else:
						System.SYSTEM_CONFIG[bus_name]['heartbeat_count'] = 0
						print "Heartbeat ok: "+bus_name
					
		return True

	def HeartbeatHandler(self,bus_name):
		System.SYSTEM_CONFIG[bus_name]['heartbeat_count']=1	

	def NewBusHandler(self, bus_name, a, b):
		if (len(b) > 0 and bus_name.find(Openbmc.BUS_PREFIX) == 0):
			if (System.SYSTEM_CONFIG.has_key(bus_name)):
				System.SYSTEM_CONFIG[bus_name]['heartbeat_count'] = 0
				obj_root = "/"+bus_name.replace('.','/')
				obj_paths = []

				## Loads object properties from system config file
				##  then overlays saved properties from file
				for instance in System.SYSTEM_CONFIG[bus_name]['instances']:
					obj_path = obj_root+'/'+instance['name']
					obj_paths.append(obj_path)
					if (instance.has_key('properties')):
						print "load props: "+obj_path
						self.property_manager.loadProperties(bus_name,obj_path,												instance['properties'])

				## scan all used interfaces and get interfaces with init method
				

				## After object properties are setup, call init method if requested
				#if (System.SYSTEM_CONFIG[bus_name].has_key('init_methods')):
				for obj_path in obj_paths:
					obj = bus.get_object(bus_name,obj_path)
					methods = Openbmc.get_methods(obj)
					for intf_name in methods.keys():
						if (methods[intf_name].has_key('init')):
							intf = dbus.Interface(obj,intf_name)
							print "Calling init: " +intf_name
							intf.init()



	@dbus.service.method(DBUS_NAME,
		in_signature='s', out_signature='sis')
	def gpioInit(self,name):
		gpio_path = ''
		gpio_num = 0
		if (System.GPIO_CONFIG.has_key(name) == False):
			# TODO: Error handling
			print "ERROR: "+name+" not found in GPIO config table"
			return ['',0,'']
		else:
			gpio_num = System.GPIO_CONFIG[name]['gpio_num']

		return [Openbmc.GPIO_DEV, gpio_num, System.GPIO_CONFIG[name]['direction']]


if __name__ == '__main__':
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
    bus = dbus.SessionBus()
    name = dbus.service.BusName(DBUS_NAME,bus)
    obj = SystemManager(bus,OBJ_NAME)
    mainloop = GObject.MainLoop()

    print "Running SystemManager"
    mainloop.run()

