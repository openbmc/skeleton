#!/usr/bin/python -u

import sys
import uuid
#from gi.repository import GObject
import gobject
import dbus
import dbus.service
import dbus.mainloop.glib
import Openbmc

DBUS_NAME = 'org.openbmc.control.Chassis'
OBJ_NAME = '/org/openbmc/control/chassis0'
CONTROL_INTF = 'org.openbmc.Control'

POWER_OFF = 0
POWER_ON = 1

BOOTED = 100

class ChassisControlObject(Openbmc.DbusProperties):
	def __init__(self,bus,name):
		self.dbus_objects = { }
		Openbmc.DbusProperties.__init__(self)
		dbus.service.Object.__init__(self,bus,name)
		## load utilized objects
		self.dbus_objects = {
			'power_control' : { 
				'bus_name' : 'org.openbmc.control.Power',
				'object_name' : '/org/openbmc/control/power0',
				'interface_name' : 'org.openbmc.control.Power'
			},
			'identify_led' : {
				'bus_name' : 'org.openbmc.control.led',
				'object_name' : '/org/openbmc/led/IDENTIFY',
				'interface_name' : 'org.openbmc.Led'
			},
			'watchdog' : {				
				'bus_name' : 'org.openbmc.watchdog.Host',
				'object_name' : '/org/openbmc/watchdog/host0',
				'interface_name' : 'org.openbmc.Watchdog'
			}
		}

		#uuid
		self.Set(DBUS_NAME,"uuid",str(uuid.uuid1()))
		self.Set(DBUS_NAME,"reboot",0)
		self.Set(DBUS_NAME,"power_policy",0)	
		self.Set(DBUS_NAME,"last_system_state","")	

		bus.add_signal_receiver(self.power_button_signal_handler, 
					dbus_interface = "org.openbmc.Button", signal_name = "Released", 
					path="/org/openbmc/buttons/power0" )
		bus.add_signal_receiver(self.reset_button_signal_handler, 
					dbus_interface = "org.openbmc.Button", signal_name = "PressedLong", 
					path="/org/openbmc/buttons/power0" )

    		bus.add_signal_receiver(self.host_watchdog_signal_handler, 
					dbus_interface = "org.openbmc.Watchdog", signal_name = "WatchdogError")
		bus.add_signal_receiver(self.SystemStateHandler,signal_name = "GotoSystemState")
		self.ObjectAdded(name,CONTROL_INTF)


	def getInterface(self,name):
		o = self.dbus_objects[name]
		obj = bus.get_object(o['bus_name'],o['object_name'])
		return dbus.Interface(obj,o['interface_name'])


	@dbus.service.method(DBUS_NAME,
		in_signature='', out_signature='')
	def setIdentify(self):
		print "Turn on identify"
		intf = self.getInterface('identify_led')
		intf.setOn()	
		return None

	@dbus.service.method(DBUS_NAME,
		in_signature='', out_signature='')
	def clearIdentify(self):
		print "Turn on identify"
		intf = self.getInterface('identify_led')
		intf.setOff()
		return None

	@dbus.service.method(DBUS_NAME,
		in_signature='', out_signature='')
	def powerOn(self):
		print "Turn on power and boot"
		self.Set(DBUS_NAME,"reboot",0)
		if (self.getPowerState()==0):
			intf = self.getInterface('power_control')
			intf.setPowerState(POWER_ON)
			intfwatchdog = self.getInterface('watchdog')
			#Start watchdog with 30s timeout per the OpenPower Host IPMI Spec
			#Once the host starts booting, it'll reset and refresh the timer
			intfwatchdog.set(30000)
			intfwatchdog.start()
		return None

	@dbus.service.method(DBUS_NAME,
		in_signature='', out_signature='')
	def powerOff(self):
		print "Turn off power"
		intf = self.getInterface('power_control')
		intf.setPowerState(POWER_OFF)
		return None

	@dbus.service.method(DBUS_NAME,
		in_signature='', out_signature='')
	def softPowerOff(self):
		print "Soft off power"
		## TODO: Somehow tell host to shutdown via ipmi
		## for now hard power off
		self.powerOff()	
		return None

	@dbus.service.method(DBUS_NAME,
		in_signature='', out_signature='')
	def reboot(self):
		print "Rebooting"
		if state == POWER_OFF:
			self.powerOn();
		else:
			self.Set(DBUS_NAME,"reboot",1)
			intf.softPowerOff()
		return None

	@dbus.service.method(DBUS_NAME,
		in_signature='', out_signature='i')
	def getPowerState(self):
		intf = self.getInterface('power_control')
		return intf.getPowerState()

	@dbus.service.method(DBUS_NAME,
		in_signature='', out_signature='')
	def setDebugMode(self):
		return None

	@dbus.service.method(DBUS_NAME,
		in_signature='i', out_signature='')
	def setPowerPolicy(self,policy):
		self.Set(DBUS_NAME,"power_policy",policy)	
		return None


	## Signal handler

	def SystemStateHandler(self,state_name):
		self.Set(DBUS_NAME,"last_system_state",state_name)	
		if (state_name == "HOST_POWERED_OFF" and self.Get(DBUS_NAME,"reboot")==1):
			self.powerOn()
				

	def power_button_signal_handler(self):
		# toggle power
		state = self.getPowerState()
		if state == POWER_OFF:
			self.powerOn()
		elif state == POWER_ON:
			self.powerOff();

	def reset_button_signal_handler(self):
		self.reboot();
		
	def host_watchdog_signal_handler(self):
		print "Watchdog Error, Hard Rebooting"
		#self.Set(DBUS_NAME,"reboot",1)
		#self.powerOff()
		

if __name__ == '__main__':
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)

    bus = Openbmc.getDBus()
    name = dbus.service.BusName(DBUS_NAME, bus)
    obj = ChassisControlObject(bus, OBJ_NAME)
    mainloop = gobject.MainLoop()
    
    print "Running ChassisControlService"
    mainloop.run()

