#!/usr/bin/env python

import sys
import subprocess
import dbus
from gi.repository import GObject
import dbus.service
import dbus.mainloop.glib

if (len(sys.argv) < 2):
	print "Usage:  ipmi_bt.py [system name]"
	exit(1)

System = __import__(sys.argv[1])
import Openbmc

DBUS_NAME = 'org.openbmc.control.IpmiBt'
OBJ_NAME = '/org/openbmc/control/IpmiBt'

class IpmiBt(dbus.service.Object):
	def __init__(self,bus,name):
		dbus.service.Object.__init__(self,bus,name)
	
	@dbus.service.signal(DBUS_NAME)
	def UpdateFru(self, iid, message):
 		pass

	@dbus.service.signal(DBUS_NAME)
	def UpdateSensor(self, iid, message):
 		pass

	@dbus.service.method(DBUS_NAME)
	def emitUpdateFru(self,ipmi_id,mfg):
		data = {
			'manufacturer' : mfg
		}
        	self.UpdateFru(ipmi_id,data)
        	return 'Signal emitted'

	@dbus.service.method(DBUS_NAME)
	def emitUpdateSensor(self,ipmi_id,data):
        	self.UpdateSensor(ipmi_id,dbus.Byte(int(data)))
		print "update sensor emitted"
        	return 'Signal emitted'

	def getSensor(self,ipmi_id):
		obj =  bus.get_object('org.openbmc.managers.IpmiTranslator',
				'/org/openbmc/managers/IpmiTranslator/Barreleye')
		intf = dbus.Interface(obj, 'org.openbmc.managers.IpmiTranslator' )
		return intf.getSensor(ipmi_id)

	def pokeHostWatchdog(self):
		obj =  bus.get_object('org.openbmc.managers.IpmiTranslator',
				'/org/openbmc/managers/IpmiTranslator/Barreleye')
		intf = dbus.Interface(obj, 'org.openbmc.managers.IpmiTranslator' )
		intf.pokeHostWatchdog()

	def startHostWatchdog(self):
		obj =  bus.get_object('org.openbmc.managers.IpmiTranslator',
				'/org/openbmc/managers/IpmiTranslator/Barreleye')
		intf = dbus.Interface(obj, 'org.openbmc.managers.IpmiTranslator' )
		intf.startHostWatchdog()





if __name__ == '__main__':
	
	cmd = ""
	data = None
	ipmi_id = 0
	if (len(sys.argv) > 2):
		cmd = sys.argv[2]
	if (len(sys.argv) > 3):
		ipmi_id = int(sys.argv[3])
	if (len(sys.argv)>4):
		data = sys.argv[4]

	dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
	bus = dbus.SessionBus()
	name = dbus.service.BusName(DBUS_NAME,bus)
	obj = IpmiBt(bus,OBJ_NAME)
	mainloop = GObject.MainLoop()

	if (cmd == 'updatefru'):
		obj.emitUpdateFru(ipmi_id,data)
	elif (cmd == 'setsensor'):
		obj.emitUpdateSensor(ipmi_id,data)
	elif (cmd == 'getsensor'):
		print obj.getSensor(ipmi_id)
	elif (cmd == 'pokewatchdog'):
		print obj.pokeHostWatchdog()
	elif (cmd == 'startwatchdog'):
		print obj.startHostWatchdog()
	else:
		print "ERROR: Invalid command"
		print "Valid commands: updatefru, setsensor, getsensor, startwatchdog, pokewatchdog"		

    #mainloop.run()

