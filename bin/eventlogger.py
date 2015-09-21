#!/usr/bin/env python

import sys
import datetime
import time
import syslog
import json
import gobject
import dbus
import dbus.service
import dbus.mainloop.glib
import Openbmc

DBUS_NAME = 'org.openbmc.loggers.EventLogger'
OBJ_NAME = '/org/openbmc/loggers/EventLogger/'+sys.argv[1]


class EventLogger(dbus.service.Object):
	def __init__(self,bus,name):
		dbus.service.Object.__init__(self,bus,name)
		bus = dbus.SessionBus()
		bus.add_signal_receiver(self.event_log_signal_handler, 
					dbus_interface = "org.openbmc.EventLog", 
					signal_name = "EventLog",
					path_keyword='path')

	## Signal handler
	def event_log_signal_handler(self,priority,msg,rc,path = None):
		message = {}
		ts = time.time()

		message['priority'] = priority
		message['object_path'] = path
		message['message'] = msg
		message['rc'] = rc

		json_dump = json.dumps(message)
		print "EVENT_LOG: "+json_dump
		syslog.openlog('OpenBmc',logoption=syslog.LOG_PID)
		syslog.syslog(priority,json_dump)
		syslog.closelog()
		



if __name__ == '__main__':
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)

    bus = dbus.SessionBus()
    name = dbus.service.BusName(DBUS_NAME, bus)
    obj = EventLogger(bus, OBJ_NAME)
    mainloop = gobject.MainLoop()
    
    print "Running EventLogger"
    mainloop.run()

