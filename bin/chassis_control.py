#!/usr/bin/env python

import gobject
import dbus
import dbus.service
import dbus.mainloop.glib

class ChassisControlObject(dbus.service.Object):
	def __init__(self,bus,name):
		self.power_state=0
		dbus.service.Object.__init__(self,bus,name)
		bus = dbus.SessionBus()
		try: 
			# Get PowerControl object
			power_control_service = bus.get_object('org.openbmc.PowerControl','/org/openbmc/PowerControl/0')
			self.power_control_iface = dbus.Interface(power_control_service, 'org.openbmc.PowerControl')
			# Get ChassisIdentify object
			chassis_identify_service = bus.get_object('org.openbmc.ChassisIdentify','/org/openbmc/ChassisIdentify/0')
			self.identify_led_iface = dbus.Interface(chassis_identify_service, 'org.openbmc.Led');
			# Get HostControl object
			host_control_service = bus.get_object('org.openbmc.HostControl','/org/openbmc/HostControl/0')
			self.host_control_iface = dbus.Interface(host_control_service, 'org.openbmc.HostControl');


		except dbus.exceptions.DBusException, e:
			# TODO: not sure what to do if can't find other services
			print "Unable to find dependent services: ",e


	@dbus.service.method("org.openbmc.ChassisControl",
		in_signature='', out_signature='s')
	def getID(self):
		return id


	@dbus.service.method("org.openbmc.ChassisControl",
		in_signature='', out_signature='')
	def setIdentify(self):
		print "Turn on identify"
		self.identify_led_iface.setOn()
		return None


	@dbus.service.method("org.openbmc.ChassisControl",
		in_signature='', out_signature='')
	def clearIdentify(self):
		print "Turn off identify"
		r=self.identify_led_iface.setOff()
		return None

	@dbus.service.method("org.openbmc.ChassisControl",
		in_signature='', out_signature='')
	def setPowerOn(self):
		print "Turn on power and boot"
		self.power_state=0
		if (self.getPowerState()==0):
			self.power_control_iface.setPowerState(1)
			self.power_state=1
		return None

	@dbus.service.method("org.openbmc.ChassisControl",
		in_signature='', out_signature='')
	def setPowerOff(self):
		print "Turn off power"
		self.power_control_iface.setPowerState(0);
		return None

	@dbus.service.method("org.openbmc.ChassisControl",
		in_signature='', out_signature='i')
	def getPowerState(self):
		state = self.power_control_iface.getPowerState();
		return state

	@dbus.service.method("org.openbmc.ChassisControl",
		in_signature='', out_signature='')
	def setDebugMode(self):
		return None

	@dbus.service.method("org.openbmc.ChassisControl",
		in_signature='i', out_signature='')
	def setPowerPolicy(self,policy):
		return None

	## Signal handler
	def power_button_signal_handler(self):
		# only power on if not currently powered on
		state = self.getPowerState()
		if state == 0:
			self.setPowerOn()
		elif state == 1:
			self.setPowerOff();
		
		# TODO: handle long press and reset

	## Signal handler
	def power_good_signal_handler(self):
		if (self.power_state==1):
			self.host_control_iface.boot()
			self.power_state=2



if __name__ == '__main__':
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)

    bus = dbus.SessionBus()
    name = dbus.service.BusName("org.openbmc.ChassisControl", bus)
    object = ChassisControlObject(bus, '/org/openbmc/ChassisControl')
    mainloop = gobject.MainLoop()
    bus.add_signal_receiver(object.power_button_signal_handler, dbus_interface = "org.openbmc.Button", signal_name = "ButtonPressed")
    bus.add_signal_receiver(object.power_good_signal_handler, dbus_interface = "org.openbmc.PowerControl", signal_name = "PowerGood")

    print "Running ChassisControlService"
    mainloop.run()

