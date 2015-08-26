#!/usr/bin/env python

import gobject
import dbus
import dbus.service
import dbus.mainloop.glib
import System

DBUS_NAME = 'org.openbmc.managers.Gpios'
OBJ_NAME = '/org/openbmc/managers/Gpios'

gpio_config = System.BarreleyeGpios()
gpio_dev = '/sys/class/gpio'

class GpioManager(dbus.service.Object):
	def __init__(self,bus,name):
		dbus.service.Object.__init__(self,bus,name)
		bus = dbus.SessionBus()
		self.gpio_locks = {}
		

	@dbus.service.method(DBUS_NAME,
		in_signature='s', out_signature='sis')
	def init(self,name):
		gpio_path = ''
		if (gpio_config.has_key(name) == False):
			# TODO: Error handling
			print "ERROR: "+name+" not found in GPIO config table"
		else:
			gpio_num = gpio_config[name]['gpio_num']
			print "GPIO Lookup: "+name+" = "+str(gpio_num)

		return [gpio_dev, gpio_num, gpio_config[name]['direction']]

	@dbus.service.method(DBUS_NAME,
		in_signature='s', out_signature='')
	def open(self,name):
		gpio_num = gpio_config[name]['gpio_num']
		self.gpio_locks[gpio_num] = 1

	@dbus.service.method(DBUS_NAME,
		in_signature='s', out_signature='')
	def close(self,name):
		# unexport?
		gpio_num = gpio_config[name]['gpio_num']
		self.gpio_locks[gpio_num] = 0


	## Signal handler


if __name__ == '__main__':
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)

    bus = dbus.SessionBus()
    name = dbus.service.BusName(DBUS_NAME, bus)
    obj = GpioManager(bus, OBJ_NAME)
    mainloop = gobject.MainLoop()
    
    print "Running GpioManager"
    mainloop.run()

