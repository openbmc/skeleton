import dbus

BUS_PREFIX = 'org.openbmc'
GPIO_DEV = '/sys/class/gpio'


FRU_TYPES = {
	'CPU' : dbus.Byte(1),
	'DIMM' : dbus.Byte(2),
	'BACKPLANE' : dbus.Byte(3),
	'RISER_CARD' : dbus.Byte(4),
	'FAN' : dbus.Byte(4)
}

ENUMS = {
	'org.openbmc.SensorIntegerThreshold.state' : 
		['NOT_SET','NORMAL','LOWER_CRITICAL','LOWER_WARNING','UPPER_WARNING','UPPER_CRITICAL'],
	'org.openbmc.Fru.type' :
		['NONE','CPU','DIMM','BACKPLANE','RISER_CARD','FAN']
}
	
