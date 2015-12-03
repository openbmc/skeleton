#!/usr/bin/python -u

import sys
import os
import gobject
import dbus
import dbus.service
import dbus.mainloop.glib
import Openbmc
if (len(sys.argv) < 2):
	print "Usage:  sensors_hwmon.py [system name]"
	exit(1)

System = __import__(sys.argv[1])

DBUS_NAME = 'org.openbmc.sensors.hwmon'
OBJ_PATH = '/org/openbmc/sensors'
DIR_POLL_INTERVAL = 10000
HWMON_PATH = '/sys/class/hwmon'

class SensorValue(Openbmc.DbusProperties):
	IFACE_NAME = 'org.openbmc.SensorValue'
	def __init__(self,bus,name):
		Openbmc.DbusProperties.__init__(self)
		self.Set(SensorValue.IFACE_NAME,'units',"")
		dbus.service.Object.__init__(self,bus,name)
		
	@dbus.service.method(IFACE_NAME,
		in_signature='v', out_signature='')
	def setValue(self,value):
		changed = False
		try:
			old_value = self.Get(SensorValue.IFACE_NAME,'value')
			if (value != old_value):
				changed = True
		except:
			changed = True

		if (changed == True):
			self.Set(SensorValue.IFACE_NAME,'value',value)
			self.Changed(self.getValue(),self.getUnits())



	@dbus.service.method(IFACE_NAME,
		in_signature='', out_signature='v')
	def getValue(self):
		return self.Get(SensorValue.IFACE_NAME,'value')

	@dbus.service.method(IFACE_NAME,
		in_signature='', out_signature='s')
	def getUnits(self):
		return self.Get(SensorValue.IFACE_NAME,'units')

	@dbus.service.signal(IFACE_NAME,signature='vs')
	def Changed(self,value,units):
		pass

def readAttribute(filename):
	val = ""
	with open(filename, 'r') as f:
		for line in f:
			val = line.rstrip('\n')
	return val

class HwmonSensor(SensorValue):
	def __init__(self, bus ,name, attribute ,poll_interval ,units, scale):
		SensorValue.__init__(self,bus,name)
		self.attribute = attribute
		self.scale = 1
		if scale > 0:
			self.scale = scale
		
		self.Set(SensorValue.IFACE_NAME,'units',units)
		self.pollSensor()

		if (poll_interval > 0):
			gobject.timeout_add(poll_interval, self.pollSensor)  
		else:
			print "ERROR HWMON: poll_interval must be > 0"
		self.ObjectAdded(name,SensorValue.IFACE_NAME)
			

	def pollSensor(self):
		try:
			with open(self.attribute, 'r') as f:
				for line in f:
					val = int(line.rstrip('\n'))/self.scale
					SensorValue.setValue(self,val)
		except:
			print "Attribute no longer exists: "+self.attribute
			return False

		return True

	@dbus.service.method(SensorValue.IFACE_NAME,
		in_signature='v', out_signature='')
	def setValue(self,value):
		try:
			with open(self.attribute, 'w') as f:
				val = int(value*self.scale)
				f.write(str(val)+'\n')
				SensorValue.setValue(self,value)
		except Exception as e:
			print e
			print "Unable to write: "+self.attribute
		
		SensorValue.setValue(self,value)



	

class Sensors(Openbmc.DbusProperties):
	def __init__(self,bus,name):
		dbus.service.Object.__init__(self,bus,name)
		self.sensors = { }
		self.hwmon_root = { }
		bus.add_signal_receiver(self.OccActiveHandler, 
					dbus_interface = "org.openbmc.SensorValue", signal_name = "Changed", 
					path="/org/openbmc/sensor/virtual/OccStatus" )

		gobject.timeout_add(DIR_POLL_INTERVAL, self.scanDirectory)   
		self.ObjectAdded(name,DBUS_NAME)

	def OccActiveHandler(self,value,units):
		print  "OCC "+value
		if (value == "Enabled"):
			print "Installing OCC device"
			os.system("echo occ-i2c 0x50 >  /sys/bus/i2c/devices/i2c-3/new_device")
			os.system("echo occ-i2c 0x51 >  /sys/bus/i2c/devices/i2c-3/new_device")
		if (value == "Disabled"):
			print "Deleting OCC device"
			os.system("echo 0x50 >  /sys/bus/i2c/devices/i2c-3/delete_device")
			os.system("echo 0x51 >  /sys/bus/i2c/devices/i2c-3/delete_device")

	def addObject(self,dpath,instance_name,attribute):
		hwmon = System.HWMON_CONFIG[instance_name][attribute]
		objsuf = hwmon['object_path']
		if (objsuf.find('<label>') > -1):
			label_file = attribute.replace('_input','_label')
			label = readAttribute(dpath+label_file)
			objsuf = objsuf.replace('<label>',label)

		objpath = OBJ_PATH+'/'+objsuf
		spath = dpath+attribute
		if (self.sensors.has_key(objpath) == False):
			if os.path.isfile(spath):
				print "HWMON add: "+objpath+" : "+spath
				self.sensors[objpath]= HwmonSensor(bus,objpath,spath,
					hwmon['poll_interval'],hwmon['units'],hwmon['scale'])
				self.hwmon_root[dpath].append(objpath)
	

	def scanDirectory(self):
	 	devices = os.listdir(HWMON_PATH)
		found_hwmon = {}
		for d in devices:
			dpath = HWMON_PATH+'/'+d+'/'
			found_hwmon[dpath] = True
			if (self.hwmon_root.has_key(dpath) == False):
				self.hwmon_root[dpath] = []
			## the instance name is a soft link
			instance_name = os.path.realpath(dpath+'device').split('/').pop()
			if (System.HWMON_CONFIG.has_key(instance_name)):
	 			for attribute in System.HWMON_CONFIG[instance_name].keys():
					self.addObject(dpath,instance_name,attribute)
			else:
				print "WARNING: Unhandled hwmon: "+dpath
	

		for k in self.hwmon_root.keys():
			if (found_hwmon.has_key(k) == False):
				## need to remove all objects associated with this path
				print "Removing: "+k
				for objpath in self.hwmon_root[k]:
					if (self.sensors.has_key(objpath) == True):
						print "HWMON remove: "+objpath
						obj = self.sensors.pop(objpath,None)
						obj.remove_from_connection()
				self.hwmon_root.pop(k,None)
				
		return True

			
if __name__ == '__main__':
	
	dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
	bus = Openbmc.getDBus()
	name = dbus.service.BusName(DBUS_NAME,bus)
	root_sensor = Sensors(bus,OBJ_PATH)
	mainloop = gobject.MainLoop()

	print "Starting HWMON sensors"
	mainloop.run()

