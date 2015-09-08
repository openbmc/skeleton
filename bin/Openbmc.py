import dbus
import xml.etree.ElementTree as ET

BUS_PREFIX = 'org.openbmc'
GPIO_DEV = '/sys/class/gpio'


FRU_TYPES = {
	'CPU' : 1,
	'DIMM' : 2,
	'BACKPLANE' : 3,
	'RISER_CARD' : 4,
	'FAN' : 5
}
ENUMS = {
	'org.openbmc.SensorIntegerThreshold.state' : 
		['NOT_SET','NORMAL','LOWER_CRITICAL','LOWER_WARNING','UPPER_WARNING','UPPER_CRITICAL'],
	'org.openbmc.Fru.type' :
		['NONE','CPU','DIMM','BACKPLANE','RISER_CARD','FAN']
}


def object_to_bus_name(obj):
	parts = obj.split('/')
	parts.pop(0)
	parts.pop()
	return ".".join(parts)	

def bus_to_object_name(bus_name):
	return "/"+bus_name.replace('.','/')

def get_methods(obj):
	methods = {}
	introspect_iface = dbus.Interface(obj,"org.freedesktop.DBus.Introspectable")
 	tree = ET.ElementTree(ET.fromstring(introspect_iface.Introspect()))
 	root = tree.getroot()
	for intf in root.iter('interface'):
 		intf_name = intf.attrib['name']
		if (intf_name.find(BUS_PREFIX)==0):
			methods[intf_name] = {}
			for method in intf.iter('method'):
				methods[intf_name][method.attrib['name']] = True
		
	return methods

class DbusProperty:
	def __init__(self,name,value):
		self.dbusBaseType = {
			'dbus.Byte' : 'int',
			'dbus.Double' : 'float',
			'dbus.Int32' : 'int',
			'dbus.UInt32' : 'long',
			'dbus.String' : 'str',
			'dbus.UInt64' : 'long',
		}
		self.name = str(name)	
		self.dbusType = str(type(value)).split("'")[1]
		self.variant_level = value.variant_level
		self.value = None

		try: 
			self.value = eval(self.dbusBaseType[self.dbusType]+"(value)")
		except:
			raise Exception("Unknown dbus type: "+self.dbusType)

	def setValue(self,value):
		try: 
			self.value = eval(self.dbusBaseType[self.dbusType]+"(value)")
		except:
			raise Exception("Unknown dbus type: "+self.dbusType)

	def setVariant(self,variant_level):
		self.variant_level = variant_level

	def getName(self):
		return self.name

	def getValue(self):
		e = self.dbusType+"(self.value, variant_level="+str(self.variant_level)+")"
		return eval(e)

	#def __getstate__(self):
	#	odict = self.__dict__.copy() # copy the dict since we change it
 	#	return odict

	##def __setstate__(self, dict):
        #	self.__dict__.update(dict)   # update attributes

	def __str__(self):
		return self.dbusType+":"+str(self.value)
