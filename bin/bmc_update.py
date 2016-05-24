#!/usr/bin/python -u

import gobject
import dbus
import dbus.service
import dbus.mainloop.glib
import Openbmc
import shutil
import tarfile
import os

DBUS_NAME = 'org.openbmc.control.BmcFlash'
OBJ_NAME = '/org/openbmc/control/flash/bmc'
FLASH_INTF = 'org.openbmc.Flash'
DOWNLOAD_INTF = 'org.openbmc.managers.Download'

UPDATE_PATH = '/run/initramfs'


def doExtract(members,files):
    for tarinfo in members:
        if files.has_key(tarinfo.name) == True:
            yield tarinfo


class BmcFlashControl(Openbmc.DbusProperties,Openbmc.DbusObjectManager):
	def __init__(self,bus,name):
		self.dbus_objects = { }
		Openbmc.DbusProperties.__init__(self)
		Openbmc.DbusObjectManager.__init__(self)
		dbus.service.Object.__init__(self,bus,name)
		
		self.Set(DBUS_NAME,"status","Idle")
		self.Set(DBUS_NAME,"filename","")
		self.Set(DBUS_NAME,"preserve_network_settings",False)
		self.Set(DBUS_NAME,"restore_application_defaults",False)
		self.Set(DBUS_NAME,"update_kernel_and_apps",False)
		self.Set(DBUS_NAME,"clear_persistent_files",False)
	
		bus.add_signal_receiver(self.download_error_handler,signal_name = "DownloadError")
		bus.add_signal_receiver(self.download_complete_handler,signal_name = "DownloadComplete")

		self.InterfacesAdded(name,self.properties)


	@dbus.service.method(DBUS_NAME,
		in_signature='ss', out_signature='')
	def updateViaTftp(self,ip,filename):
		self.TftpDownload(ip,filename)
		self.Set(DBUS_NAME,"status","Downloading")
		
	@dbus.service.method(DBUS_NAME,
		in_signature='s', out_signature='')
	def update(self,filename):
		self.Set(DBUS_NAME,"filename",filename)
		self.download_complete_handler(filename, filename)

	@dbus.service.signal(DOWNLOAD_INTF,signature='ss')
	def TftpDownload(self,ip,filename):
		self.Set(DBUS_NAME,"filename",filename)
		pass		


	## Signal handler
	def download_error_handler(self,filename):
		if (filename == self.Get(DBUS_NAME,"filename")):
			self.Set(DBUS_NAME,"status","Download Error")

	def download_complete_handler(self,outfile,filename):
		## do update
		if (filename != self.Get(DBUS_NAME,"filename")):
			return
	
		print "Download complete. Updating..."
	
		self.Set(DBUS_NAME,"status","Download Complete")
		copy_files = {}
		
		## determine needed files
		if (self.Get(DBUS_NAME,"update_kernel_and_apps") == False):
			copy_files["image-bmc"] = True
		else:
			copy_files["image-kernel"] = True
			copy_files["image-initramfs"] = True
			copy_files["image-rofs"] = True

		if (self.Get(DBUS_NAME,"restore_application_defaults") == True):
			copy_files["image-rwfs"] = True
			
		
		## make sure files exist in archive
		try:
			tar = tarfile.open(outfile,"r")
			files = {}
			for f in tar.getnames():
				files[f] = True
			tar.close()
			for f in copy_files.keys():
				if (files.has_key(f) == False):
					raise Exception("ERROR: File not found in update archive: "+f)							

		except Exception as e:
			print e
			self.Set(DBUS_NAME,"status","Update Error")
			return

		try:
			tar = tarfile.open(outfile,"r")
			tar.extractall(UPDATE_PATH,members=doExtract(tar,copy_files))
			tar.close()

			if (self.Get(DBUS_NAME,"clear_persistent_files") == True):
				print "Removing persistent files"
				os.unlink(UPDATE_PATH+"/whitelist")
			if (self.Get(DBUS_NAME,"preserve_network_settings") == True):
				print "Preserving network settings"
				shutil.copy2("/dev/mtd2",UPDATE_PATH+"/image-u-boot-env")
				
		except Exception as e:
			print e
			self.Set(DBUS_NAME,"status","Update Error")
				
		

		self.Set(DBUS_NAME,"status","Update Success.  Please reboot.")
		

if __name__ == '__main__':
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)

    bus = Openbmc.getDBus()
    name = dbus.service.BusName(DBUS_NAME, bus)
    obj = BmcFlashControl(bus, OBJ_NAME)
    mainloop = gobject.MainLoop()
    
    print "Running Bmc Flash Control"
    mainloop.run()

