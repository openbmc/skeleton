#!/usr/bin/env python

import sys
import gobject
import dbus
import dbus.service
import dbus.mainloop.glib
import tftpy



DBUS_NAME = 'org.openbmc.managers.Flash'
OBJ_NAME = '/org/openbmc/managers/Flash'
TFTP_PORT = 69
DOWNLOAD_DIR = '/tmp'

class FlashManagerObject(dbus.service.Object):
	def __init__(self,bus,name):
		self.dbus_objects = { }
		self.status = { 'bios' : 'OK', 'bmc' : 'OK' }
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
		bus.add_signal_receiver(self.UpdatedHandler, 
			dbus_interface = "org.openbmc.Flash", signal_name = "Updated", path_keyword='path')


	def UpdatedHandler(self,path = None):
		print "Flash update finish: "+path
		for flash in self.dbus_objects:
			if (path == self.dbus_objects[flash]['object_name']):
				self.status[flash] = 'OK'		

	def getInterface(self,name):
		o = self.dbus_objects[name]
		obj = bus.get_object(o['bus_name'],o['object_name'])
		return dbus.Interface(obj,o['interface_name'])

	@dbus.service.method(DBUS_NAME,
		in_signature='', out_signature='a{ss}')
	def getStatus(self):
		return self.status

	@dbus.service.method(DBUS_NAME,
		in_signature='sss', out_signature='')
	def updateFromTftp(self,flash,url,filename):
		if (self.dbus_objects.has_key(flash) == False):
			print "ERROR FlashManager: Not a valid flash device: "+flash	
			return
		try:
			## need to make download async
			self.status[flash]="DOWNLOADING"
			filename = str(filename)
			client = tftpy.TftpClient(url, TFTP_PORT)
			print "Downloading: "+filename+" from "+url
			outfile = DOWNLOAD_DIR+"/"+filename
			client.download(filename,outfile)
			intf = self.getInterface(flash)
			self.status[flash]="FLASHING"
			intf.update(outfile)
					
		except Exception as e:
			print "ERROR FlashManager: "+str(e)
			self.status="ERROR"
	


if __name__ == '__main__':
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
    bus = Openbmc.getDBus()
    name = dbus.service.BusName(DBUS_NAME, bus)
    obj = FlashManagerObject(bus, OBJ_NAME)
    mainloop = gobject.MainLoop()
    
    print "Running Flash Manager"
    mainloop.run()

