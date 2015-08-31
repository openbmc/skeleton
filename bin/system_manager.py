#!/usr/bin/env python

import sys
import subprocess
import gobject
import dbus
import dbus.service
import dbus.mainloop.glib
import xml.etree.ElementTree as ET

if (len(sys.argv) < 2):
	print "Usage:  system_manager.py [system name]"
	exit(1)

System = __import__(sys.argv[1])
import Openbmc

DBUS_NAME = 'org.openbmc.managers.System'
OBJ_NAME = '/org/openbmc/managers/System'
HEARTBEAT_CHECK_INTERVAL = 20000

def findConfigInstance(bus_name,obj_path):
	line = obj_path.split('/')
	instance_name = line[len(line)-1]
	if (System.SYSTEM_CONFIG.has_key(bus_name) == False):
		return {}
	for instance in System.SYSTEM_CONFIG[bus_name]['instances']:
		if (instance['name'] == instance_name):
			return instance

def parseIntrospection(bus_name,obj_name,interfaces,init_interfaces):
	obj = bus.get_object(bus_name, obj_name)
	introspect_iface = dbus.Interface(obj,'org.freedesktop.DBus.Introspectable')
	tree = ET.ElementTree(ET.fromstring(introspect_iface.Introspect()))
	root = tree.getroot()
	interfaces[obj_name] = []
	init_interfaces[obj_name] = {}
	for intf in root.iter('interface'):
		intf_name = intf.attrib['name']
		if (intf_name == 'org.freedesktop.DBus.ObjectManager'):
			manager = dbus.Interface(obj,'org.freedesktop.DBus.ObjectManager')
			for managed_obj in manager.GetManagedObjects():
				parseIntrospection(bus_name,managed_obj,interfaces,init_interfaces)
		elif (intf_name.find(Openbmc.BUS_PREFIX) == 0):
			interfaces[obj_name].append(intf_name)
			for method in intf.iter('method'):
				if (method.attrib['name'] == 'init'):
					#print "Init: "+obj_name+" : "+intf_name
					init_interfaces[obj_name][intf_name]=1
				



class SystemManager(dbus.service.Object):
	def __init__(self,bus,name):
		dbus.service.Object.__init__(self,bus,name)
		bus.add_signal_receiver(self.request_name,
					dbus_interface = 'org.freedesktop.DBus', 
					signal_name = "NameOwnerChanged")
		# launch dbus object processes
		for bus_name in System.SYSTEM_CONFIG.keys():
			self.start_process(bus_name)

    		gobject.timeout_add(HEARTBEAT_CHECK_INTERVAL, self.heartbeat_check)

	def start_process(self,bus_name):
		exe_name = System.SYSTEM_CONFIG[bus_name]['exe_name']
		cmdline = [ ]
		cmdline.append(exe_name)
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
			## even if process doesn't request heartbeat check, 
			##   make sure process is still alive
			p = System.SYSTEM_CONFIG[bus_name]['popen']
			p.poll()
			if (p.returncode != None):
				print "Process for "+bus_name+" appears to be dead"
				self.start_process(bus_name)

			elif (System.SYSTEM_CONFIG[bus_name]['heartbeat'] == 'yes'):
				if (System.SYSTEM_CONFIG[bus_name]['heartbeat_count'] == 0):
					print "Heartbeat error: "+bus_name
					p = System.SYSTEM_CONFIG[bus_name]['popen']
					p.poll()
					if (p.returncode == None):
						print "Process must be hung, so killing"
						p.kill()
						
					self.start_process(bus_name)			
				else:
					System.SYSTEM_CONFIG[bus_name]['heartbeat_count'] = 0
					print "Heartbeat ok: "+bus_name
					
		return True

	def heartbeat_update(self,bus_name):
		System.SYSTEM_CONFIG[bus_name]['heartbeat_count']=1	

	def setup_sensor(self,intf):
		pass 

	def request_name(self, bus_name, a, b):
		if (len(b) > 0 and bus_name.find(Openbmc.BUS_PREFIX) == 0):
			if (System.SYSTEM_CONFIG.has_key(bus_name)):
				System.SYSTEM_CONFIG[bus_name]['heartbeat_count'] = 0
				obj_name = "/"+bus_name.replace('.','/')
				interfaces = {}
				init_interfaces = {}
				# introspect object to get used interfaces
				parseIntrospection(bus_name,obj_name,interfaces,init_interfaces)
				for obj_path in interfaces.keys():
					# find instance in system config
					instance = findConfigInstance(bus_name,obj_path)
					for intf_name in interfaces[obj_path]:
						self.initObject(bus_name,obj_path,intf_name,instance)
					for init_intf in init_interfaces[obj_path].keys():
						obj = bus.get_object(bus_name,obj_path)
						intf = dbus.Interface(obj,init_intf)
						intf.init()


	def initObject(self,bus_name,obj_path,intf_name,instance):
		obj = bus.get_object(bus_name,obj_path)
		intf = dbus.Interface(obj,intf_name)
		if (instance.has_key('properties')):
			properties = dbus.Interface(obj, 'org.freedesktop.DBus.Properties')
			for prop_intf in instance['properties']:
				for prop in instance['properties'][prop_intf]:
					value = instance['properties'][prop_intf][prop]
					properties.Set(prop_intf,prop,value)

		## TODO: fix this explicit check
		if (intf_name == 'org.openbmc.Control' or intf_name == 'org.openbmc.SensorInteger'):
			if (System.SYSTEM_CONFIG[bus_name]['heartbeat'] == 'yes'):
				print "Add heartbeat: "+intf_name;
				bus.add_signal_receiver(self.heartbeat_update,
						dbus_interface = intf_name, 
						signal_name = "Heartbeat")
		
			if (instance.has_key('parameters')):
				intf.setConfigData(instance['parameters'])


	@dbus.service.signal(DBUS_NAME)
	def CriticalThreshold(self, obj):
		print "Critical: "+obj

	@dbus.service.signal(DBUS_NAME)
	def WarningThreshold(self, obj):
		print "Warning: "+obj

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
    mainloop = gobject.MainLoop()

    print "Running SystemManager"
    mainloop.run()

