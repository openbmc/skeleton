import dbus

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

class DbusProperty:
	def __init__(self,name,value):
		self.dbusBaseType = {
			'dbus.Byte' : 'int',
			'dbus.Float' : 'float',
			'dbus.Int32' : 'int',
			'dbus.UInt32' : 'long',
			'dbus.String' : 'str',
			'dbus.UInt64' : 'long',
		}
		self.name = str(name)	
		self.dbusType = str(type(value)).split("'")[1]
		self.value = None
		try: 
			self.value = eval(self.dbusBaseType[self.dbusType]+"(value)")
		except:
			raise Exception("Unknown dbus type: "+self.dbusType)

	def changeValue(self,value):
		try: 
			self.value = eval(self.dbusBaseType[self.dbusType]+"(value)")
		except:
			raise Exception("Unknown dbus type: "+self.dbusType)


	def getName(self):
		return self.name
	def getValue(self):
		e = self.dbusType+"(self.value)"
		return eval(e)

	#def __getstate__(self):
	#	odict = self.__dict__.copy() # copy the dict since we change it
 	#	return odict

	##def __setstate__(self, dict):
        #	self.__dict__.update(dict)   # update attributes

	def __str__(self):
		return self.dbusType+":"+str(self.value)
