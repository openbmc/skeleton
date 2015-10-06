#!/usr/bin/env python

import sys
import gobject
import dbus
import dbus.service
import dbus.mainloop.glib
import tftpy



DBUS_NAME = 'org.openbmc.managers.Flash'
OBJ_NAME = '/org/openbmc/managers/'+sys.argv[1]
TFTP_PORT = 69
DOWNLOAD_DIR = '/tmp'

class FlashManagerObject(dbus.service.Object):
	def __init__(self,bus,name):
		self.dbus_objects = { }

		dbus.service.Object.__init__(self,bus,name)
		## load utilized objects
		self.dbus_objects = {
			'bios' : { 
				'bus_name' : 'org.openbmc.flash.Bios',
				'object_name' : '/org/openbmc/flash/Bios_0',
				'interface_name' : 'org.openbmc.Flash'
			},
			'bmc' : {
				'bus_name' : 'org.openbmc.flash.Bmc',
				'object_name' : '/org/openbmc/flash/Bmc_0',
				'interface_name' : 'org.openbmc.Flash'
			}
		}
		bus = dbus.SessionBus()

	def getInterface(self,name):
		o = self.dbus_objects[name]
		obj = bus.get_object(o['bus_name'],o['object_name'])
		return dbus.Interface(obj,o['interface_name'])

	@dbus.service.method(DBUS_NAME,
		in_signature='sss', out_signature='')
	def updateFromTftp(self,flash,url,filename):
		if (self.dbus_objects.has_key(flash) == False):
			print "Error: Not a valid flash device: "+flash	
			return
		try:
			client = tftpy.TftpClient(url, TFTP_PORT)
			outfile = DOWNLOAD_DIR+"/"+filename
			client.download(filename,outfile)		
			intf = self.getInterface(flash)
			intf.update(outfile)
					
		except Exception as e:
			print "ERROR: "+str(e)
	


if __name__ == '__main__':
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)

    bus = dbus.SessionBus()
    name = dbus.service.BusName(DBUS_NAME, bus)
    obj = FlashManagerObject(bus, OBJ_NAME)
    mainloop = gobject.MainLoop()
    
    print "Running Flash Manager"
    mainloop.run()

