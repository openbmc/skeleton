#!/usr/bin/python -u

import sys
import os
import gobject
import dbus
import dbus.service
import dbus.mainloop.glib
import Openbmc
from Sensors import SensorValue as SensorValue
from Sensors import HwmonSensor as HwmonSensor
from Sensors import SensorThresholds as SensorThresholds
if (len(sys.argv) < 2):
	print "Usage:  sensors_hwmon.py [system name]"
	exit(1)

System = __import__(sys.argv[1])

SENSOR_BUS = 'org.openbmc.Sensors'
SENSOR_PATH = '/org/openbmc/sensors'
DIR_POLL_INTERVAL = 10000
HWMON_PATH = '/sys/class/hwmon'

## static define which interface each property is under
## need a better way that is not slow
IFACE_LOOKUP = {
	'units' : SensorValue.IFACE_NAME,
	'scale' : HwmonSensor.IFACE_NAME,
	'offset' : HwmonSensor.IFACE_NAME,
	'critical_upper' : SensorThresholds.IFACE_NAME,
	'warning_upper' : SensorThresholds.IFACE_NAME,
	'critical_lower' : SensorThresholds.IFACE_NAME,
	'warning_lower' : SensorThresholds.IFACE_NAME,
}

class Hwmons():
	def __init__(self,bus):
		self.sensors = { }
		self.hwmon_root = { }
		self.scanDirectory()
		gobject.timeout_add(DIR_POLL_INTERVAL, self.scanDirectory)   

	def readAttribute(self,filename):
		val = ""
		with open(filename, 'r') as f:
			for line in f:
				val = line.rstrip('\n')
		return val

	def writeAttribute(self,filename,value):
		with open(filename, 'w') as f:
			f.write(str(value)+'\n')


	def poll(self,objpath,attribute):
		try:
			raw_value = int(self.readAttribute(attribute))
			obj = bus.get_object(SENSOR_BUS,objpath)
			intf = dbus.Interface(obj,HwmonSensor.IFACE_NAME)
			rtn = intf.setByPoll(raw_value)
			if (rtn[0] == True):
				self.writeAttribute(attribute,rtn[1])
		except:
			print "HWMON: Attibute no longer exists: "+attribute
			return False


		return True


	def addObject(self,dpath,instance_name,attribute):
		hwmon = System.HWMON_CONFIG[instance_name][attribute]
		objsuf = hwmon['object_path']
		try:
			if (objsuf.find('<label>') > -1):
				label_file = attribute.replace('_input','_label')
				label = self.readAttribute(dpath+label_file)
				objsuf = objsuf.replace('<label>',label)
		except Exception as e:
			print e
			return

		objpath = SENSOR_PATH+'/'+objsuf
		spath = dpath+attribute
		if (self.sensors.has_key(objpath) == False):
			if os.path.isfile(spath):
				print "HWMON add: "+objpath+" : "+spath
				obj = bus.get_object(SENSOR_BUS,SENSOR_PATH)
				intf = dbus.Interface(obj,SENSOR_BUS)
				intf.register("HwmonSensor",objpath)
			
				obj = bus.get_object(SENSOR_BUS,objpath)
				intf = dbus.Interface(obj,dbus.PROPERTIES_IFACE)
				intf.Set(HwmonSensor.IFACE_NAME,'filename',spath)
				
				## check if one of thresholds is defined to know
				## whether to enable thresholds or not
				if (hwmon.has_key('critical_upper')):
					intf.Set(SensorThresholds.IFACE_NAME,'thresholds_enabled',True)

				for prop in hwmon.keys():
					if (IFACE_LOOKUP.has_key(prop)):
						intf.Set(IFACE_LOOKUP[prop],prop,hwmon[prop])
						print "Setting: "+prop+" = "+str(hwmon[prop])

				self.sensors[objpath]=True
				self.hwmon_root[dpath].append(objpath)
				gobject.timeout_add(hwmon['poll_interval'],self.poll,objpath,spath)
	
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
						self.sensors.pop(objpath,None)
						obj = bus.get_object(SENSOR_BUS,SENSOR_PATH)
						intf = dbus.Interface(obj,SENSOR_BUS)
						intf.delete(objpath)

				self.hwmon_root.pop(k,None)
				
		return True

			
if __name__ == '__main__':
	
	dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
	bus = Openbmc.getDBus()
	root_sensor = Hwmons(bus)
	mainloop = gobject.MainLoop()

	print "Starting HWMON sensors"
	mainloop.run()

