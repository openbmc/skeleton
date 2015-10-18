#!/usr/bin/env python

import sys
import subprocess
import gobject
import dbus
import dbus.service
import dbus.mainloop.glib
import os
import PropertyManager
import time
import json
import Openbmc

if (len(sys.argv) < 2):
	print "Usage:  system_manager.py [system name]"
	exit(1)

System = __import__(sys.argv[1])
import Openbmc

DBUS_NAME = 'org.openbmc.managers.System'
OBJ_NAME = '/org/openbmc/managers/System'
HEARTBEAT_CHECK_INTERVAL = 20000
STATE_START_TIMEOUT = 10
INTF_SENSOR = 'org.openbmc.SensorValue'
INTF_ITEM = 'org.openbmc.InventoryItem'
INTF_CONTROL = 'org.openbmc.Control'

class SystemManager(dbus.service.Object):
	def __init__(self,bus,name):
		dbus.service.Object.__init__(self,bus,name)
		self.property_manager = PropertyManager.PropertyManager(bus,System.CACHE_PATH)

		## Signal handlers
		bus.add_signal_receiver(self.NewBusHandler,
					dbus_interface = 'org.freedesktop.DBus', 
					signal_name = "NameOwnerChanged")
		bus.add_signal_receiver(self.SystemStateHandler,signal_name = "GotoSystemState")

		self.current_state = ""
		self.system_states = {}
		self.bus_name_lookup = {}
		self.bin_path = os.path.dirname(os.path.realpath(sys.argv[0]))

		for bus_name in System.SYSTEM_CONFIG.keys():
			sys_state = System.SYSTEM_CONFIG[bus_name]['system_state']
			if (self.system_states.has_key(sys_state) == False):
				self.system_states[sys_state] = []
			self.system_states[sys_state].append(bus_name)
	
		## replace symbolic path in ID_LOOKUP
		for category in System.ID_LOOKUP:
			for key in System.ID_LOOKUP[category]:
				val = System.ID_LOOKUP[category][key]
				new_val = val.replace("<inventory_root>",System.INVENTORY_ROOT)
				System.ID_LOOKUP[category][key] = new_val
	
		self.SystemStateHandler("BMC_INIT")
		print "SystemManager Init Done"

	def SystemStateHandler(self,state_name):
		print "Checking previous state started..."
		i = 0
		started = self.check_state_started()	
		while(i<10 and started == False):
			started = self.check_state_started()	
			i=i+1
			time.sleep(1)	

		if (i == STATE_START_TIMEOUT):
			print "ERROR: Timeout waiting for state to finish: "+self.current_state
			return					
		
		## clearing object started flags
		try:
			for obj_path in System.EXIT_STATE_DEPEND[self.current_state]:
				System.EXIT_STATE_DEPEND[self.current_state][obj_path] = 0
		except:
			pass

		print "Running System State: "+state_name
		if (self.system_states.has_key(state_name)):
			for bus_name in self.system_states[state_name]:
				self.start_process(bus_name)
		
		if (state_name == "BMC_INIT"):
			## Add poll for heartbeat
	    		gobject.timeout_add(HEARTBEAT_CHECK_INTERVAL, self.heartbeat_check)
		
		try:	
			cb = System.ENTER_STATE_CALLBACK[state_name]
			obj = bus.get_object(cb['bus_name'],cb['obj_name'])
			method = obj.get_dbus_method(cb['method_name'],cb['interface_name'])
			method()
		except:
			pass

		self.current_state = state_name
		
	@dbus.service.method(DBUS_NAME,
		in_signature='ss', out_signature='(sss)')
	def getObjectFromId(self,category,key):
		bus_name = ""
		obj_path = ""
		intf_name = INTF_ITEM
		try:
			obj_path = System.ID_LOOKUP[category][key]
			bus_name = self.bus_name_lookup[obj_path]
			parts = obj_path.split('/')
			if (parts[2] == 'sensor'):
				intf_name = INTF_SENSOR
		except Exception as e:
			print "ERROR SystemManager: "+str(e)+" not found in lookup"

		return [bus_name,obj_path,intf_name]


	@dbus.service.method(DBUS_NAME,
		in_signature='sy', out_signature='(sss)')
	def getObjectFromByteId(self,category,key):
		bus_name = ""
		obj_path = ""
		intf_name = INTF_ITEM
		try:
			byte = int(key)
			obj_path = System.ID_LOOKUP[category][byte]
			bus_name = self.bus_name_lookup[obj_path]
			parts = obj_path.split('/')
			if (parts[3] == 'sensor'):
				intf_name = INTF_SENSOR
		except Exception as e:
			print "ERROR SystemManager: "+str(e)+" not found in lookup"

		return [bus_name,obj_path,intf_name]

	
	def start_process(self,bus_name):
		if (System.SYSTEM_CONFIG[bus_name]['start_process'] == True):
			process_name = self.bin_path+"/"+System.SYSTEM_CONFIG[bus_name]['process_name']
			cmdline = [ ]
			cmdline.append(process_name)
			System.SYSTEM_CONFIG[bus_name]['popen'] = None
			for instance in System.SYSTEM_CONFIG[bus_name]['instances']:
				cmdline.append(instance['name'])
			try:
				print "Starting process: "+" ".join(cmdline)+": "+bus_name
				System.SYSTEM_CONFIG[bus_name]['popen'] = subprocess.Popen(cmdline)
			except Exception as e:
				## TODO: error
				print "ERROR: starting process: "+" ".join(cmdline)

	def heartbeat_check(self):
		for bus_name in System.SYSTEM_CONFIG.keys():
			if (System.SYSTEM_CONFIG[bus_name]['start_process'] == True and
				System.SYSTEM_CONFIG[bus_name].has_key('popen') and
				System.SYSTEM_CONFIG[bus_name]['monitor_process'] == True):
				##   make sure process is still alive
				p = System.SYSTEM_CONFIG[bus_name]['popen']
				p.poll()
				if (p.returncode != None):
					print "Process for "+bus_name+" appears to be dead"
					self.start_process(bus_name)
	
		return True

	def check_state_started(self):
		r = True
		if (self.current_state == ""):
			return True
		if (self.system_states.has_key(self.current_state)):
			for bus_name in self.system_states[self.current_state]:
				if (System.SYSTEM_CONFIG[bus_name].has_key('popen') == False and
					System.SYSTEM_CONFIG[bus_name]['start_process'] == True):
					r = False
					break;	
		return r
	

	def NewBusHandler(self, bus_name, a, b):
		if (len(b) > 0 and bus_name.find(Openbmc.BUS_PREFIX) == 0):
			objects = {}
			try:
				Openbmc.get_objs(bus,bus_name,"",objects)
				for instance_name in objects.keys():
					obj_path = objects[instance_name]['PATH']
					self.bus_name_lookup[obj_path] = bus_name
					if (System.EXIT_STATE_DEPEND[self.current_state].has_key(obj_path) == True):
						System.EXIT_STATE_DEPEND[self.current_state][obj_path] = 1
								
			except:
				pass
	
			if (System.SYSTEM_CONFIG.has_key(bus_name)):
				System.SYSTEM_CONFIG[bus_name]['heartbeat_count'] = 0
				for instance_name in objects.keys(): 
					obj_path = objects[instance_name]['PATH']
					for instance in System.SYSTEM_CONFIG[bus_name]['instances']:
						if (instance.has_key('properties') and instance['name'] == instance_name):
							props = instance['properties']
							print "Load Properties: "+obj_path
							self.property_manager.loadProperties(bus_name,obj_path,props)
					## If object has an init method, call it
					for init_intf in objects[instance_name]['INIT']:
						obj = bus.get_object(bus_name,obj_path)
						intf = dbus.Interface(obj,init_intf)
						print "Calling init method: " +obj_path+" : "+init_intf
						intf.init()

			## check if all objects are started to move to next state
			try:
				state = 1
				for obj_path in System.EXIT_STATE_DEPEND[self.current_state]:
					if (System.EXIT_STATE_DEPEND[self.current_state][obj_path] == 0):
						state = 0
				if (state == 1):
					s = 0
					for i in range(len(System.SYSTEM_STATES)):
						if (System.SYSTEM_STATES[i] == self.current_state):
							s = i+1

					if (s == len(System.SYSTEM_STATES)):
						print "ERROR SystemManager: No more system states"
					else:
						new_state_name = System.SYSTEM_STATES[s]
						print "SystemManager Goto System State: "+new_state_name
						self.SystemStateHandler(new_state_name)
			except:
				pass



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
    bus = Openbmc.getDBus()
    name = dbus.service.BusName(DBUS_NAME,bus)
    obj = SystemManager(bus,OBJ_NAME)
    mainloop = gobject.MainLoop()

    print "Running SystemManager"
    mainloop.run()

