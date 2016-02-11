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

MACHINE_ID = '/etc/machine-id'

POWER_OFF = 0
POWER_ON = 1

BOOTED = 100

class ChassisControlObject(Openbmc.DbusProperties,Openbmc.DbusObjectManager):
	def getUuid(self):
		uuid = "";
		try:
			with open(MACHINE_ID) as f:
				data = f.readline().rstrip('\n')
				if (len(data) == 32):
					uuid = data
				else:
					print "ERROR:  UUID is not formatted correctly: "+data
		except:
			print "ERROR: Unable to open uuid file: "+MACHINE_ID
				
		return uuid
 
	def __init__(self,bus,name):
		self.dbus_objects = { }
		Openbmc.DbusProperties.__init__(self)
		Openbmc.DbusObjectManager.__init__(self)
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
				'object_name' : '/org/openbmc/control/led/identify',
				'interface_name' : 'org.openbmc.Led'
			},
			'watchdog' : {				
				'bus_name' : 'org.openbmc.watchdog.Host',
				'object_name' : '/org/openbmc/watchdog/host0',
				'interface_name' : 'org.openbmc.Watchdog'
			},
			'host_services' : {
				'bus_name' : 'org.openbmc.HostServices',
				'object_name' : '/org/openbmc/HostServices',
				'interface_name' : 'org.openbmc.HostServices'
			},
			'settings' : {
				'bus_name' : 'org.openbmc.settings.Host',
				'object_name' : '/org/openbmc/settings/host0',
				'interface_name' : 'org.freedesktop.DBus.Properties'
			},
		}

		#uuid
		self.Set(DBUS_NAME,"uuid",self.getUuid())
		self.Set(DBUS_NAME,"reboot",0)

		bus.add_signal_receiver(self.power_button_signal_handler, 
					dbus_interface = "org.openbmc.Button", signal_name = "Released", 
					path="/org/openbmc/buttons/power0" )
		bus.add_signal_receiver(self.reset_button_signal_handler, 
					dbus_interface = "org.openbmc.Button", signal_name = "PressedLong", 
					path="/org/openbmc/buttons/power0" )
		bus.add_signal_receiver(self.softreset_button_signal_handler, 
					dbus_interface = "org.openbmc.Button", signal_name = "Released", 
					path="/org/openbmc/buttons/reset0" )

    		bus.add_signal_receiver(self.host_watchdog_signal_handler, 
					dbus_interface = "org.openbmc.Watchdog", signal_name = "WatchdogError")

   		bus.add_signal_receiver(self.emergency_shutdown_signal_handler, 
					dbus_interface = "org.openbmc.SensorThresholds", signal_name = "Emergency")

		bus.add_signal_receiver(self.SystemStateHandler,signal_name = "GotoSystemState")
		self.InterfacesAdded(name,self.properties)


	def getInterface(self,name):
		o = self.dbus_objects[name]
		obj = bus.get_object(o['bus_name'],o['object_name'],introspect=False)
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
		intfwatchdog = self.getInterface('watchdog')
		intfwatchdog.stop()
		intf = self.getInterface('power_control')
		intf.setPowerState(POWER_OFF)
		return None

	@dbus.service.method(DBUS_NAME,
		in_signature='', out_signature='')
	def softPowerOff(self):
		print "Soft off power"
		intf = self.getInterface('host_services')
		## host services will call power off when ready
		intf.SoftPowerOff()
		return None

	@dbus.service.method(DBUS_NAME,
		in_signature='', out_signature='')
	def reboot(self):
		print "Rebooting"
		if self.getPowerState() == POWER_OFF:
			self.powerOn();
		else:
			self.Set(DBUS_NAME,"reboot",1)
			self.powerOff()
		return None

	@dbus.service.method(DBUS_NAME,
		in_signature='', out_signature='')
	def softReboot(self):
		print "Soft Rebooting"
		if self.getPowerState() == POWER_OFF:
			self.powerOn();
		else:
			self.Set(DBUS_NAME,"reboot",1)
			self.softPowerOff()
		return None

	@dbus.service.method(DBUS_NAME,
		in_signature='', out_signature='i')
	def getPowerState(self):
		intf = self.getInterface('power_control')
		return intf.getPowerState()

	## Signal handler

	def SystemStateHandler(self,state_name):
		if (state_name == "HOST_POWERED_OFF" or state_name == "HOST_POWERED_ON"):
			intf = self.getInterface('settings')
			intf.Set("org.openbmc.settings.Host","system_state",state_name)

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

	def softreset_button_signal_handler(self):
		self.softReboot();
		
	def host_watchdog_signal_handler(self):
		print "Watchdog Error, Hard Rebooting"
		self.Set(DBUS_NAME,"reboot",1)
		self.powerOff()

	def emergency_shutdown_signal_handler(self):
		print "Emergency Shutdown!"
		self.powerOff()
		
		

if __name__ == '__main__':
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)

    bus = Openbmc.getDBus()
    name = dbus.service.BusName(DBUS_NAME, bus)
    obj = ChassisControlObject(bus, OBJ_NAME)
    mainloop = gobject.MainLoop()
    
    print "Running ChassisControlService"
    mainloop.run()

