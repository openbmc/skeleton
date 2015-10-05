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


class SystemManager(dbus.service.Object):
	def __init__(self,bus,name):
		dbus.service.Object.__init__(self,bus,name)
		self.property_manager = PropertyManager.PropertyManager(bus,System.CACHE_PATH)

		## Signal handlers
		bus.add_signal_receiver(self.NewBusHandler,
					dbus_interface = 'org.freedesktop.DBus', 
					signal_name = "NameOwnerChanged")
		bus.add_signal_receiver(self.HeartbeatHandler, signal_name = "Heartbeat")
		bus.add_signal_receiver(self.SystemStateHandler,signal_name = "GotoSystemState")
		bus.add_signal_receiver(self.event_log_signal_handler, 
					dbus_interface = "org.openbmc.EventLog", 
					signal_name = "EventLog",
					path_keyword='path')

		self.current_state = ""
		self.system_states = {}
		self.bus_name_lookup = {}

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
	
		self.SystemStateHandler("INIT")
		print "SystemManager Init Done"

	@dbus.service.signal('org.openbmc.EventLog')
	def EventLog(self, priority, message, rc):
        	pass

	def SystemStateHandler(self,state_name):
		print "Checking previous state started..."
		i = 0
		started = self.check_state_started()	
		while(i<10 and started == False):
			started = self.check_state_started()	
			i=i+1
			time.sleep(1)	

		if (i == 10):
			self.EventLog(1,"Timeout waiting for state to finish: "+self.current_state,1)	
			return					

		print "Running System State: "+state_name
		if (self.system_states.has_key(state_name)):
			for bus_name in self.system_states[state_name]:
				self.start_process(bus_name)
		
		if (state_name == "INIT"):
			## Add poll for heartbeat
	    		gobject.timeout_add(HEARTBEAT_CHECK_INTERVAL, self.heartbeat_check)
		
		if (System.ENTER_STATE_CALLBACK.has_key(state_name)):
			cb = System.ENTER_STATE_CALLBACK[state_name]
			obj = bus.get_<F11>object(cb['bus_name'],cb['obj_name'])
			method = obj.get_dbus_method(cb['method_name'],cb['interface_name'])
			method()

		self.current_state = state_name
		
	@dbus.service.method(DBUS_NAME,
		in_signature='ss', out_signature='a{ss}')
	def getObjectFromId(self,category,key):
		bus_name = ""
		obj_path = ""

		if (System.ID_LOOKUP.has_key(category)):
			if (System.ID_LOOKUP[category].has_key(key)):
				obj_path = System.ID_LOOKUP[category][key]
		else:
			print "ERROR: key not found: "+category+","+key

		if (self.bus_name_lookup.has_key(obj_path)):
			bus_name = self.bus_name_lookup[obj_path]
		else:
			print "ERROR: bus name not found for: "+obj_path
		r = { 'bus_name' : bus_name, 'obj_path' : obj_path }
		return r
	
	
	def start_process(self,bus_name):
		if (System.SYSTEM_CONFIG[bus_name]['start_process'] == True):
			process_name = System.BIN_PATH+System.SYSTEM_CONFIG[bus_name]['process_name']
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
				print "Error starting process: "+" ".join(cmdline)

	def heartbeat_check(self):
		#print "heartbeat check"
		for bus_name in System.SYSTEM_CONFIG.keys():
			if (System.SYSTEM_CONFIG[bus_name]['start_process'] == True and
				System.SYSTEM_CONFIG[bus_name].has_key('popen') and
				System.SYSTEM_CONFIG[bus_name]['monitor_process'] == True):
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
						#print "Heartbeat ok: "+bus_name
					
		return True

	def HeartbeatHandler(self,bus_name):
		#print "Heartbeat seen: "+bus_name
		System.SYSTEM_CONFIG[bus_name]['heartbeat_count']=1	

	def check_state_started(self):
		r = True
		if (self.current_state == ""):
			return True
		for bus_name in self.system_states[self.current_state]:
			if (System.SYSTEM_CONFIG[bus_name].has_key('popen') == False and
				System.SYSTEM_CONFIG[bus_name]['start_process'] == True):
				print "Not found: "+bus_name
				r = False
				break;	
		return r
	

	def NewBusHandler(self, bus_name, a, b):
		if (len(b) > 0 and bus_name.find(Openbmc.BUS_PREFIX) == 0):
			objects = {}
			Openbmc.get_objs(bus,bus_name,"",objects)
			for instance_name in objects.keys():
				self.bus_name_lookup[objects[instance_name]['PATH']] = bus_name
			
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

	## Signal handler
	def event_log_signal_handler(self,priority,msg,rc,path = None):
		message = {}
		ts = time.time()

		message['priority'] = priority
		message['object_path'] = path
		message['message'] = msg
		message['rc'] = rc

		json_dump = json.dumps(message)
		print "EVENT_LOG: "+json_dump
		#syslog.openlog('OpenBmc',logoption=syslog.LOG_PID)
		#syslog.syslog(priority,json_dump)
		#syslog.closelog()
		

if __name__ == '__main__':
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
    bus = dbus.SessionBus()
    name = dbus.service.BusName(DBUS_NAME,bus)
    obj = SystemManager(bus,OBJ_NAME)
    mainloop = gobject.MainLoop()

    print "Running SystemManager"
    mainloop.run()

