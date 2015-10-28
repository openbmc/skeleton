#!/usr/bin/python -u

import sys
import subprocess
import gobject
import dbus
import dbus.service
import dbus.mainloop.glib
import os
import time
import json
import xml.etree.ElementTree as ET

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

def get_objects(bus,bus_name,path,objects):
	tmp_path = path
	if (path == ""):
		tmp_path="/"

	obj = bus.get_object(bus_name,tmp_path)
	introspect_iface = dbus.Interface(obj,"org.freedesktop.DBus.Introspectable")
 	tree = ET.ElementTree(ET.fromstring(introspect_iface.Introspect()))
 	root = tree.getroot()
	parent = True
	##print introspect_iface.Introspect()
	for node in root.iter('node'):
		for intf in node.iter('interface'):
			objects[path] = True

		if (node.attrib.has_key('name') == True):
			node_name = node.attrib['name']
			if (parent == False):
				get_objects(bus,bus_name,path+"/"+node_name,objects)
			else:
				if (node_name != "" and node_name != path):
					get_objects(bus,bus_name,node_name,objects)
			
		parent = False




class SystemManager(dbus.service.Object):
	def __init__(self,bus,name):
		dbus.service.Object.__init__(self,bus,name)

		## Signal handlers
		bus.add_signal_receiver(self.NewBusHandler,
					dbus_interface = 'org.freedesktop.DBus', 
					signal_name = "NameOwnerChanged")
		bus.add_signal_receiver(self.SystemStateHandler,signal_name = "GotoSystemState")

		self.current_state = ""
		self.system_states = {}
		self.bus_name_lookup = {}
		self.bin_path = os.path.dirname(os.path.realpath(sys.argv[0]))

		for name in System.APPS.keys():
			sys_state = System.APPS[name]['system_state']
			if (self.system_states.has_key(sys_state) == False):
				self.system_states[sys_state] = []
			self.system_states[sys_state].append(name)
	
		## replace symbolic path in ID_LOOKUP
		for category in System.ID_LOOKUP:
			for key in System.ID_LOOKUP[category]:
				val = System.ID_LOOKUP[category][key]
				new_val = val.replace("<inventory_root>",System.INVENTORY_ROOT)
				System.ID_LOOKUP[category][key] = new_val
	
		self.SystemStateHandler(System.SYSTEM_STATES[0])
		print "SystemManager Init Done"


	def SystemStateHandler(self,state_name):
		## clearing object started flags
		try:
			for obj_path in System.EXIT_STATE_DEPEND[self.current_state]:
				System.EXIT_STATE_DEPEND[self.current_state][obj_path] = 0
		except:
			pass

		print "Running System State: "+state_name
		if (self.system_states.has_key(state_name)):
			for name in self.system_states[state_name]:
				self.start_process(name)
		
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

	def gotoNextState(self):
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
	
	
	@dbus.service.method(DBUS_NAME,
		in_signature='', out_signature='s')
	def getSystemState(self):
		return self.current_state

	def doObjectLookup(self,category,key):
		bus_name = ""
		obj_path = ""
		intf_name = INTF_ITEM
		try:
			obj_path = System.ID_LOOKUP[category][key]
			bus_name = self.bus_name_lookup[obj_path]
			parts = obj_path.split('/')
			if (parts[3] == 'sensor'):
				intf_name = INTF_SENSOR
		except Exception as e:
			print "ERROR SystemManager: "+str(e)+" not found in lookup"

		return [bus_name,obj_path,intf_name]

	@dbus.service.method(DBUS_NAME,
		in_signature='ss', out_signature='(sss)')
	def getObjectFromId(self,category,key):
		return self.doObjectLookup(category,key)

	@dbus.service.method(DBUS_NAME,
		in_signature='sy', out_signature='(sss)')
	def getObjectFromByteId(self,category,key):
		byte = int(key)
		return self.doObjectLookup(category,byte)
	
	def start_process(self,name):
		if (System.APPS[name]['start_process'] == True):
			app = System.APPS[name]
			process_name = self.bin_path+"/"+app['process_name']
			cmdline = [ ]
			cmdline.append(process_name)
			app['popen'] = None
			if (app.has_key('args')):
				for a in app['args']:
					cmdline.append(a)
			try:
				print "Starting process: "+" ".join(cmdline)+": "+name
				app['popen'] = subprocess.Popen(cmdline)
			except Exception as e:
				## TODO: error
				print "ERROR: starting process: "+" ".join(cmdline)

	def heartbeat_check(self):
		for name in System.APPS.keys():
			app = System.APPS[name]
			if (app['start_process'] == True and 
				app.has_key('popen') and
				app['monitor_process'] == True):

				##   make sure process is still alive
				p = app['popen']
				p.poll()
				if (p.returncode != None):
					print "Process for "+name+" appears to be dead"
					self.start_process(name)
	
		return True

	def NewBusHandler(self, bus_name, a, b):
		if (len(b) > 0 and bus_name.find(Openbmc.BUS_PREFIX) == 0):
			objects = {}
			try:
				get_objects(bus,bus_name,"",objects)
				for obj_path in objects.keys():
					self.bus_name_lookup[obj_path] = bus_name
					print "New object: "+obj_path+" ("+bus_name+")"
					if (System.EXIT_STATE_DEPEND[self.current_state].has_key(obj_path) == True):
						System.EXIT_STATE_DEPEND[self.current_state][obj_path] = 1
								
			except Exception as e:
				## object probably disappeared
				#print e
				pass
	
			## check if all required objects are started to move to next state
			try:
				state = 1
				for obj_path in System.EXIT_STATE_DEPEND[self.current_state]:
					if (System.EXIT_STATE_DEPEND[self.current_state][obj_path] == 0):
						state = 0
				## all required objects have started so go to next state
				if (state == 1):
					self.gotoNextState()
			except:
				pass



	@dbus.service.method(DBUS_NAME,
		in_signature='s', out_signature='sis')
	def gpioInit(self,name):
		gpio_path = ''
		gpio_num = -1
		r = ['',gpio_num,'']
		if (System.GPIO_CONFIG.has_key(name) == False):
			# TODO: Error handling
			print "ERROR: "+name+" not found in GPIO config table"
		else:
			
			gpio_num = -1
			gpio = System.GPIO_CONFIG[name]
			if (System.GPIO_CONFIG[name].has_key('gpio_num')):
				gpio_num = gpio['gpio_num']
			else:
				if (System.GPIO_CONFIG[name].has_key('gpio_pin')):
					gpio_num = System.convertGpio(gpio['gpio_pin'])
				else:
					print "ERROR: SystemManager - GPIO lookup failed for "+name
		
			if (gpio_num != -1):
				r = [Openbmc.GPIO_DEV, gpio_num, gpio['direction']]
		return r
		

if __name__ == '__main__':
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
    bus = Openbmc.getDBus()
    name = dbus.service.BusName(DBUS_NAME,bus)
    obj = SystemManager(bus,OBJ_NAME)
    mainloop = gobject.MainLoop()

    print "Running SystemManager"
    mainloop.run()

