#!/usr/bin/env python

import sys
import gobject
import dbus
import dbus.service
import dbus.mainloop.glib
import System

DBUS_NAME = 'org.openbmc.control.Chassis'
OBJ_NAME = '/org/openbmc/control/Chassis/'+sys.argv[1]

POWER_OFF = 0
POWER_ON = 1

process_config = System.BarreleyeProcesses()



class ChassisControlObject(dbus.service.Object):
	def __init__(self,bus,name):
		self.dbus_objects = { }
		self.dbus_busses = {
				'org.openbmc.control.Power' :        [ { 'name' : 'PowerControl1' ,   'intf' : 'org.openbmc.control.Power' } ],
				'org.openbmc.leds.ChassisIdentify' : [ { 'name' : 'ChassisIdentify1', 'intf' : 'org.openbmc.control.Chassis' } ],
				'org.openbmc.control.Host' :         [ { 'name' : 'HostControl1',     'intf' : 'org.openbmc.control.Host' } ]
		}
		self.power_sequence = 0
		dbus.service.Object.__init__(self,bus,name)
		bus = dbus.SessionBus()
		try: 
			for bus_name in self.dbus_busses.keys():
				self.request_name(bus_name,"",bus_name)
			bus.add_signal_receiver(self.request_name,
					dbus_interface = 'org.freedesktop.DBus', 
					signal_name = "NameOwnerChanged")
		except dbus.exceptions.DBusException, e:
			# TODO: not sure what to do if can't find other services
			print e

		bus.add_signal_receiver(self.power_button_signal_handler, 
					dbus_interface = "org.openbmc.Button", signal_name = "ButtonPressed", 
					path="/org/openbmc/buttons/ButtonPower/PowerButton1" )
    		bus.add_signal_receiver(self.power_good_signal_handler, 
					dbus_interface = "org.openbmc.control.Power", signal_name = "PowerGood",
					path="/org/openbmc/control/Power/PowerControl1")


	
	def request_name(self, bus_name, a, b):
		# bus added
		if (len(b) > 0 ):
			if (self.dbus_busses.has_key(bus_name)):
				obj_path = "/"+bus_name.replace('.','/')
				for objs in self.dbus_busses[bus_name]:
					inst_name = objs['name']
					obj =  bus.get_object(bus_name,obj_path+"/"+inst_name)
					print "Interface:  "+inst_name+","+objs['intf']
					self.dbus_objects[inst_name] = dbus.Interface(obj, objs['intf'])
	

	@dbus.service.method(DBUS_NAME,
		in_signature='', out_signature='s')
	def getID(self):
		return id


	@dbus.service.method(DBUS_NAME,
		in_signature='', out_signature='')
	def setIdentify(self):
		print "Turn on identify"
		self.dbus_objects['ChassisIdentify1'].setOn()
		return None


	@dbus.service.method(DBUS_NAME,
		in_signature='', out_signature='')
	def clearIdentify(self):
		print "Turn off identify"
		r=self.dbus_objects['ChassisIdentify1'].setOff()
		return None

	@dbus.service.method(DBUS_NAME,
		in_signature='', out_signature='')
	def setPowerOn(self):
		print "Turn on power and boot"
		self.power_sequence = 0
		if (self.getPowerState()==0):
			self.dbus_objects['PowerControl1'].setPowerState(POWER_ON)
			self.power_sequence = 1
		return None

	@dbus.service.method(DBUS_NAME,
		in_signature='', out_signature='')
	def setPowerOff(self):
		print "Turn off power"
		self.dbus_objects['PowerControl1'].setPowerState(POWER_OFF);
		return None

	@dbus.service.method(DBUS_NAME,
		in_signature='', out_signature='i')
	def getPowerState(self):
		state = self.dbus_objects['PowerControl1'].getPowerState();
		return state

	@dbus.service.method(DBUS_NAME,
		in_signature='', out_signature='')
	def setDebugMode(self):
		return None

	@dbus.service.method(DBUS_NAME,
		in_signature='i', out_signature='')
	def setPowerPolicy(self,policy):
		return None

	## Signal handler
	def power_button_signal_handler(self):
		# only power on if not currently powered on
		state = self.getPowerState()
		if state == POWER_OFF:
			self.setPowerOn()
		elif state == POWER_ON:
			self.setPowerOff();
		
		# TODO: handle long press and reset

	## Signal handler
	def power_good_signal_handler(self):
		if (self.power_sequence==1):
			self.dbus_objects['HostControl1'].boot()
			self.power_sequence = 2



if __name__ == '__main__':
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)

    bus = dbus.SessionBus()
    name = dbus.service.BusName(DBUS_NAME, bus)
    obj = ChassisControlObject(bus, OBJ_NAME)
    mainloop = gobject.MainLoop()
    
    print "Running ChassisControlService"
    mainloop.run()

