import dbus
import xml.etree.ElementTree as ET

BUS_PREFIX = 'org.openbmc'
OBJ_PREFIX = "/org/openbmc"
GPIO_DEV = '/sys/class/gpio'


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


def getManagerInterface(bus,manager):
	bus_name = "org.openbmc.managers."+manager
	obj_name = "/org/openbmc/managers/"+manager
	obj = bus.get_object(bus_name,obj_name)
	return dbus.Interface(obj,bus_name)


def get_objs(bus,bus_name,path,objects):
	#print ">>>>>>>>>>>>>>>>>>>>>>>>>>>> "+bus_name+"; "+path
	tmp_path = path
	if (path == ""):
		tmp_path="/"
	obj = bus.get_object(bus_name,tmp_path)
	introspect_iface = dbus.Interface(obj,"org.freedesktop.DBus.Introspectable")
	#print introspect_iface.Introspect()
 	tree = ET.ElementTree(ET.fromstring(introspect_iface.Introspect()))
 	root = tree.getroot()
	parent = True
	for node in root.iter('node'):
		for intf in node.iter('interface'):
			intf_name = intf.attrib['name']
			#if (intf_name.find(BUS_PREFIX)==0):
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


class DbusProperty:
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
