#!/usr/bin/env python

import subprocess
import gobject
import dbus
import dbus.service
import dbus.mainloop.glib
import xml.etree.ElementTree as ET
import System

DBUS_NAME = 'org.openbmc.managers.System'
OBJ_NAME = '/org/openbmc/managers/System'

gpio_dev = '/sys/class/gpio'
process_config = System.BarreleyeProcesses()
gpio_config = System.BarreleyeGpios()

def findConfigInstance(bus_name,obj_path):
	line = obj_path.split('/')
	instance_name = line[len(line)-1]
	if (process_config.has_key(bus_name) == False):
		return {}
	for instance in process_config[bus_name]['instances']:
		if (instance['name'] == instance_name):
			return instance

def parseIntrospection(bus_name,obj_name,interfaces):
	obj = bus.get_object(bus_name, obj_name)
	introspect_iface = dbus.Interface(obj,'org.freedesktop.DBus.Introspectable')
	tree = ET.ElementTree(ET.fromstring(introspect_iface.Introspect()))
	root = tree.getroot()
	interfaces[obj_name] = []
	for intf in root.iter('interface'):
		intf_name = intf.attrib['name']
		if (intf_name == 'org.freedesktop.DBus.ObjectManager'):
			manager = dbus.Interface(obj,'org.freedesktop.DBus.ObjectManager')
			for managed_obj in manager.GetManagedObjects():
				parseIntrospection(bus_name,managed_obj,interfaces)
		elif (intf_name.find('org.openbmc') == 0):
			interfaces[obj_name].append(intf_name)


class SystemManager(dbus.service.Object):
	def __init__(self,bus,name):
		dbus.service.Object.__init__(self,bus,name)
		bus.add_signal_receiver(self.request_name,
					dbus_interface = 'org.freedesktop.DBus', 
					signal_name = "NameOwnerChanged")
		# launch dbus object processes
		for bus_name in process_config.keys():
			exe_name = process_config[bus_name]['exe_name']
			cmdline = [ ]
			cmdline.append(exe_name)
			for instance in process_config[bus_name]['instances']:
				cmdline.append(instance['name'])
			subprocess.Popen(cmdline);

		gobject.timeout_add(5000, self.heartbeat_check)

	def heartbeat_check(self):
		print "heartbeat check"
		for bus_name in process_config.keys():
			if (process_config[bus_name]['heartbeat'] == 'yes'):
				if (process_config[bus_name]['heartbeat_count'] == 0):
					print "Heartbeat error: "+bus_name
				else:
					process_config[bus_name]['heartbeat_count'] == 0
					print "Heartbeat ok"

	def heartbeat_update(self,bus_name):
		process_config[bus_name]['heartbeat_count']=1	

	def setup_sensor(self,intf):
		pass 

	def request_name(self, bus_name, a, b):
		if (len(b) > 0 and bus_name.find('org.openbmc') == 0):
			try:
				if (process_config.has_key(bus_name)):
					process_config[bus_name]['heartbeat_count'] = 0
					obj_name = "/"+bus_name.replace('.','/')
					interfaces = {}
					parseIntrospection(bus_name,obj_name,interfaces)
					for obj_path in interfaces.keys():
						instance = findConfigInstance(bus_name,obj_path)
						for intf_name in interfaces[obj_path]:
							obj = bus.get_object(bus_name,obj_path)
							intf = dbus.Interface(obj,intf_name)
							if (intf_name == 'org.openbmc.SensorIntegerThreshold'):
								intf.set(instance['lower_critical'],
								instance['lower_warning'],
								instance['upper_warning'],
								instance['upper_critical'])
									
							if (intf_name == 'org.openbmc.SensorInteger'):
								if (process_config[bus_name]['heartbeat'] == 'yes'):
									bus.add_signal_receiver(self.heartbeat_update,
										dbus_interface = intf_name, 
										signal_name = "Heartbeat")

								if (instance.has_key('parameters')):
									intf.setConfigData(instance['parameters'])

			except Exception as e:
				print e

		if (len(b)==0  and bus_name.find('org.openbmc.sensors') == 0):
			exe_name = process_config[bus_name]['exe_name']
			cmdline = [ ]
			cmdline.append(exe_name)
			for instance in process_config[bus_name]['instances']:
				cmdline.append(instance['name'])
 
			subprocess.Popen(cmdline);

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
		if (gpio_config.has_key(name) == False):
			# TODO: Error handling
			print "ERROR: "+name+" not found in GPIO config table"
			return ['',0,'']
		else:
			gpio_num = gpio_config[name]['gpio_num']

		return [gpio_dev, gpio_num, gpio_config[name]['direction']]


if __name__ == '__main__':
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
    bus = dbus.SessionBus()
    name = dbus.service.BusName(DBUS_NAME,bus)
    obj = SystemManager(bus,OBJ_NAME)
    mainloop = gobject.MainLoop()

    
    print "Running SystemManager"
    mainloop.run()

