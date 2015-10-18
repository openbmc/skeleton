import dbus
import xml.etree.ElementTree as ET

BUS_PREFIX = 'org.openbmc'
OBJ_PREFIX = "/org/openbmc"
GPIO_DEV = '/sys/class/gpio'
BUS = "system"

FRU_TYPES = {
	'SYSTEM' : 0,
	'CPU' : 1,
	'DIMM' : 2,
	'MAIN_PLANAR' : 3,
	'RISER_CARD' : 4,
	'FAN' : 5,
	'BMC' : 6,
	'CORE' : 7,
	'PCIE_CARD' : 8,
}
FRU_STATES = {
	'NORMAL'            : 0,
	'RECOVERABLE_ERROR' : 1,
	'FATAL_ERROR'       : 2,
	'NOT_PRESENT'       : 3,
}


ENUMS = {
	'org.openbmc.SensorIntegerThreshold.state' : 
		['NOT_SET','NORMAL','LOWER_CRITICAL','LOWER_WARNING','UPPER_WARNING','UPPER_CRITICAL'],
}

DBUS_TO_BASE_TYPES = {
	'dbus.Byte' : 'int',
	'dbus.Double' : 'float',
	'dbus.Int32' : 'int',
	'dbus.UInt32' : 'long',
	'dbus.String' : 'str',
	'dbus.UInt64' : 'long',
	'dbus.Boolean' : 'bool',
}

BASE_TO_DBUS_TYPES = {
	'int'   : 'dbus.Int32',
	'float' : 'dbus.Double',
	'str'   : 'dbus.String',
	'long'  : 'dbus.Int64',
	'bool'  : 'dbus.Boolean'
}

def getSystemName():
	#use filename as system name, strip off path and ext
	parts = __file__.replace('.pyc','').replace('.py','').split('/')
	return parts[len(parts)-1]


def getDBus():
	bus = None
	if (BUS == "session"):
		bus = dbus.SessionBus()
	else:
		bus = dbus.SystemBus()
	return bus


def getManagerInterface(bus,manager):
	bus_name = "org.openbmc.managers."+manager
	obj_name = "/org/openbmc/managers/"+manager
	obj = bus.get_object(bus_name,obj_name)
	return dbus.Interface(obj,bus_name)


def get_objs(bus,bus_name,path,objects):
	tmp_path = path
	if (path == ""):
		tmp_path="/"

	obj = bus.get_object(bus_name,tmp_path)
	introspect_iface = dbus.Interface(obj,"org.freedesktop.DBus.Introspectable")
 	tree = ET.ElementTree(ET.fromstring(introspect_iface.Introspect()))
 	root = tree.getroot()
	parent = True
	for node in root.iter('node'):
		for intf in node.iter('interface'):
			intf_name = intf.attrib['name']
			parts=path.split('/')
			instance = parts[len(parts)-1]
			if (objects.has_key(instance) == False):
				objects[instance] = {}
				objects[instance]['PATH'] = path
				objects[instance]['INIT'] = []
			for method in intf.iter('method'):
				if (method.attrib['name'] == "init"):
					objects[instance]['INIT'].append(intf_name)

		if (node.attrib.has_key('name') == True):
			node_name = node.attrib['name']
			if (parent == False):
				get_objs(bus,bus_name,path+"/"+node_name,objects)
			else:
				if (node_name != "" and node_name != path):
					get_objs(bus,bus_name,node_name,objects)
			
		parent = False

class DbusProperties(dbus.service.Object):
	def __init__(self):
		dbus.service.Object.__init__(self)
		self.properties = {}

	@dbus.service.method(dbus.PROPERTIES_IFACE,
		in_signature='ss', out_signature='v')
	def Get(self, interface_name, property_name):
		d = self.GetAll(interface_name)
		try:
			v = d[property_name]
			return v
		except:
 			raise dbus.exceptions.DBusException(
				"org.freedesktop.UnknownPropery: "+property_name)

	@dbus.service.method(dbus.PROPERTIES_IFACE,
		in_signature='s', out_signature='a{sv}')
	def GetAll(self, interface_name):
		try:
			d = self.properties[interface_name]
			return d
 		except:
 			raise dbus.exceptions.DBusException(
				"org.freedesktop.UnknownInterface: "+interface_name)

	@dbus.service.method(dbus.PROPERTIES_IFACE,
		in_signature='ssv')
	def Set(self, interface_name, property_name, new_value):
		if (self.properties.has_key(interface_name) == False):
			self.properties[interface_name] = {}
		try:
			old_value = self.properties[interface_name][property_name] 
			if (old_value != new_value):
				self.properties[interface_name][property_name] = new_value
				self.PropertiesChanged(interface_name,{ property_name: new_value }, [])
				
		except:
        		self.properties[interface_name][property_name] = new_value

	@dbus.service.signal(dbus.PROPERTIES_IFACE,
		signature='sa{sv}as')
	def PropertiesChanged(self, interface_name, changed_properties,
		invalidated_properties):
		pass



class DbusVariable:
	def __init__(self,name,value):
		self.name = str(name)	
		self.dbusType = str(type(value)).split("'")[1]
		self.variant_level = 2
		self.value = None
		if (BASE_TO_DBUS_TYPES.has_key(self.dbusType) == False):
			self.variant_level = value.variant_level
			try: 
				self.value = eval(DBUS_TO_BASE_TYPES[self.dbusType]+"(value)")
			except:
				raise Exception("Unknown dbus type: "+self.dbusType)
		else:
			self.dbusType = BASE_TO_DBUS_TYPES[self.dbusType]
			self.value = value

	def setValue(self,value):
		try: 
			self.value = eval(DBUS_TO_BASE_TYPES[self.dbusType]+"(value)")
		except:
			raise Exception("Unknown dbus type: "+self.dbusType)

	def setVariant(self,variant_level):
		self.variant_level = variant_level

	def getName(self):
		return self.name

	def getValue(self):
		e = self.dbusType+"(self.value, variant_level="+str(self.variant_level)+")"
		return eval(e)

	def getBaseValue(self):
		return self.value

	def __str__(self):
		return self.dbusType+":"+str(self.value)
