#!/usr/bin/env python

import sys
import subprocess
import dbus
from gi.repository import GObject
import dbus.service
import dbus.mainloop.glib
import Openbmc


if __name__ == '__main__':
	cmd = sys.argv[1]
	data = None
	ipmi_id = dbus.Byte(0)
	if (len(sys.argv) > 2):
		ipmi_id = dbus.Byte(int(sys.argv[2]))
	if (len(sys.argv)>3):
		data = sys.argv[3]

	bus = dbus.SessionBus()
	intf = Openbmc.getManagerInterface(bus,"Ipmi")

	if (cmd == 'updatefru'):
		d = { 'manufacturer' : data }
		intf.updateFru(ipmi_id,d)
	elif (cmd == 'getfrus'):
		print intf.getFrus()
	elif (cmd == 'setsensor'):
		data_b = dbus.Byte(int(data))
		intf.setSensor(ipmi_id,data_b)
	elif (cmd == 'getsensor'):
		print intf.getSensor(ipmi_id)
	elif (cmd == 'pokewatchdog'):
		print intf.pokeHostWatchdog()
	elif (cmd == 'startwatchdog'):
		print intf.startHostWatchdog()
	elif (cmd == 'poweron'):
		print intf.powerOn()
	elif (cmd == 'poweroff'):
		print intf.powerOff()
	else:
		print "ERROR: Invalid command"
		print "Valid commands: updatefru, setsensor, getsensor, startwatchdog, pokewatchdog"		

    #mainloop.run()

